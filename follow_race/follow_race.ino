//ライントレース一周 time34.60
// Arduino Uno のピン配置
// モータ
const int motor_r1 = 2; // Arduinoの2番ピンに対応
const int motor_r2 = 3;
const int pwm_motor_r = 10; // PWM信号生成可能なピンを選ぶ
const int motor_l1 = 4;
const int motor_l2 = 5;
const int pwm_motor_l = 11;
const int traceSpeed = 105;   // 基本速度（0-255）

// 追従用のパラメータ
int left_foward_trig_pin = 6;    // Trigger
int left_foward_echo_pin = 7;    // Echo
int right_foward_trig_pin = 9;    // Trigger
int right_foward_echo_pin = 13;    // Echo
const int obstacle_pin = 8;  // 障害物センサ OUT

long left_duration, right_duration;
float left_cm, right_cm;
int diffLimit = 5;

// ----------------------------------------------------
// カーブ中の一時的なセンサロストで直進に戻ってしまい
// 曲がりきれなくなるのを防ぐためのヒステリシス
// ----------------------------------------------------
enum DriveMode { STRAIGHT, TURN_LEFT, TURN_RIGHT };
DriveMode currentMode = STRAIGHT;
int loseCount = 0;
const int LOSE_CONFIRM = 3; // 「両方見失った」が何回連続したら直進に切り替えるか

// ----------------------------------------------------
// 外れ値フィルタ：直近10回の測定値から平均・標準偏差を求め、
// 大きく外れた値（ノイズ由来と思われる値）は採用せず直前の採用値を使う
// ----------------------------------------------------
const int HIST_SIZE = 10;
const float OUTLIER_K = 2.0;     // 何σ以上ずれたら外れ値とみなすか
const float MIN_STDDEV = 2.0;    // 測定値が安定している時に誤検知しないための下限値(cm)
const int REJECT_RESET = 3;      // 何回連続で外れ値判定されたらフィルタをリセットするか

struct OutlierFilter {
  float history[HIST_SIZE];
  int count = 0;
  int idx = 0;
  float lastAccepted = 20;
  int rejectStreak = 0;

  float filter(float newValue) {
    if (newValue <= 0) {
      // 超音波センサのタイムアウト（未検出）は無効値として履歴に入れない
      return lastAccepted;
    }

    if (count < HIST_SIZE) {
      // 履歴が十分に溜まるまではそのまま採用
      history[idx] = newValue;
      idx = (idx + 1) % HIST_SIZE;
      count++;
      lastAccepted = newValue;
      return newValue;
    }

    float sum = 0;
    for (int i = 0; i < HIST_SIZE; i++) sum += history[i];
    float mean = sum / HIST_SIZE;

    float varSum = 0;
    for (int i = 0; i < HIST_SIZE; i++) {
      float d = history[i] - mean;
      varSum += d * d;
    }
    float stddev = sqrt(varSum / HIST_SIZE);
    if (stddev < MIN_STDDEV) stddev = MIN_STDDEV;

    if (fabs(newValue - mean) > OUTLIER_K * stddev) {
      // 外れ値とみなして採用せず、直前の採用値を維持。
      // ただし連続で外れ値判定される場合はノイズではなく実際の変化とみなし、
      // 履歴を新しい値でリセットして復帰できるようにする
      rejectStreak++;
      if (rejectStreak >= REJECT_RESET) {
        for (int i = 0; i < HIST_SIZE; i++) history[i] = newValue;
        idx = 0;
        rejectStreak = 0;
        lastAccepted = newValue;
        return newValue;
      }
      return lastAccepted;
    }

    rejectStreak = 0;
    history[idx] = newValue;
    idx = (idx + 1) % HIST_SIZE;
    lastAccepted = newValue;
    return newValue;
  }
};

OutlierFilter leftFilter;
OutlierFilter rightFilter;

// ----------------------------------------------------
// 追従距離を一定に保つための速度自動調整
// 目標距離より遠ければ加速、近ければ減速する比例制御
// ----------------------------------------------------
const float targetDistance = 7.0;      // 保ちたい追従距離(cm)
const float speedKp = 2.0;             // 距離誤差(cm)あたりの速度補正量
const int minTraceSpeed = 90;          // これ以上は遅くしない
const int maxTraceSpeed = 200;         // これ以上は速くしない
const float maxSensorDistance = 100.0; // センサが未検出(0)の時に「遠い」とみなす距離

int computeTraceSpeed(float distance) {
  if (distance <= 0) distance = maxSensorDistance; // 未検出は遠いとみなして加速させる
  float speed = traceSpeed + speedKp * (distance - targetDistance);
  if (speed < minTraceSpeed) speed = minTraceSpeed;
  if (speed > maxTraceSpeed) speed = maxTraceSpeed;
  return (int)speed;
}



void setup() { // 実行時に1回だけ実行
  pinMode(motor_r1, OUTPUT); // motor_r1 に対応するピン（2番）を出力ポートに設定
  pinMode(motor_r2, OUTPUT);
  pinMode(motor_l1, OUTPUT);
  pinMode(motor_l2, OUTPUT);
  pinMode(pwm_motor_r, OUTPUT);
  pinMode(pwm_motor_l, OUTPUT);
  pinMode(right_foward_trig_pin, OUTPUT);
  pinMode(right_foward_echo_pin, INPUT);
  pinMode(left_foward_trig_pin, OUTPUT);
  pinMode(left_foward_echo_pin, INPUT);
  pinMode(obstacle_pin, INPUT);
  Serial.begin(9600);
}

void stopMotor() { // モータを停止させる関数
  digitalWrite(motor_l1, LOW);
  digitalWrite(motor_l2, LOW);
  analogWrite(pwm_motor_l, 0);
  digitalWrite(motor_r1, LOW);
  digitalWrite(motor_r2, LOW);
  analogWrite(pwm_motor_r, 0);
}

void forward(int speedBase = 200) { // 前進させる関数
  digitalWrite(motor_l1, HIGH);
  digitalWrite(motor_l2, LOW);
  analogWrite(pwm_motor_l, speedBase + 10);
  digitalWrite(motor_r1, LOW);
  digitalWrite(motor_r2, HIGH);
  analogWrite(pwm_motor_r, speedBase);
  delay(5);
}

void backward(int speedBase = 200) { // 後退させる関数
  digitalWrite(motor_l1, LOW);
  digitalWrite(motor_l2, HIGH);
  analogWrite(pwm_motor_r, speedBase);
  delay(20);
}

void turnLeft(int speedBase = 150) { // 左に曲がる関数
  digitalWrite(motor_l1, LOW);
  digitalWrite(motor_l2, HIGH);
  analogWrite(pwm_motor_l, speedBase + 50);
  digitalWrite(motor_r1, LOW);
  digitalWrite(motor_r2, HIGH);
  analogWrite(pwm_motor_r, speedBase - 10);
  
}

void turnRight(int speedBase = 150) { // 右に曲がる関数
  digitalWrite(motor_l1, HIGH);
  digitalWrite(motor_l2, LOW);
  analogWrite(pwm_motor_l, speedBase - 10);
  digitalWrite(motor_r1, HIGH);
  digitalWrite(motor_r2, LOW);
  analogWrite(pwm_motor_r, speedBase + 50);
}

// ライントレース用：両輪とも前進させつつ左右の速度差で緩やかに曲がる
// diff が大きいほど曲がりが鋭くなる
void curveLeft(int speedBase = 180, int diff = 0) { // 左方向へ緩やかに曲がる
  int l = 0;
  int r = speedBase + diff;
  if (l < 0) l = 0;
  if (r > 255) r = 255;
  digitalWrite(motor_l1, HIGH);
  digitalWrite(motor_l2, LOW);
  analogWrite(pwm_motor_l, l);
  digitalWrite(motor_r1, LOW);
  digitalWrite(motor_r2, HIGH);
  analogWrite(pwm_motor_r, r);
  delay(1);
}

void curveRight(int speedBase = 180, int diff = 0) { // 右方向へ緩やかに曲がる
  int l = speedBase + diff;
  int r = 0;
  if (l > 255) l = 255;
  if (r < 0) r = 0;
  digitalWrite(motor_l1, HIGH);
  digitalWrite(motor_l2, LOW);
  analogWrite(pwm_motor_l, l);
  digitalWrite(motor_r1, LOW);
  digitalWrite(motor_r2, HIGH);
  analogWrite(pwm_motor_r, r);
  delay(1);
}

void loop() {
  bool obstacle = (digitalRead(obstacle_pin) == LOW); 
// センサによって LOWで検知 / HIGHで検知 が逆の場合あり

if (obstacle) {
  stopMotor();
  Serial.println("Obstacle detected");
  delay(100);
  return;
}
  // ----------------------------------------------------
  // 1. 左側のセンサー測定
  // ----------------------------------------------------
  digitalWrite(left_foward_trig_pin, LOW);
  delayMicroseconds(2);
  digitalWrite(left_foward_trig_pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(left_foward_trig_pin, LOW);
  
  left_duration = pulseIn(left_foward_echo_pin, HIGH, 20000); // タイムアウトを20msに設定（約3.4m分）
  left_cm = (left_duration / 2.0) / 29.1;

  // センサー同士の干渉を防ぐため、少しだけ間隔をあける
  delay(30);

  // ----------------------------------------------------
  // 2. 右側のセンサー測定
  // ----------------------------------------------------
  digitalWrite(right_foward_trig_pin, LOW);
  delayMicroseconds(2);
  digitalWrite(right_foward_trig_pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(right_foward_trig_pin, LOW);
  
  right_duration = pulseIn(right_foward_echo_pin, HIGH, 20000); // タイムアウトを20msに設定
  right_cm = (right_duration / 2.0) / 29.1;

  // 過去10回分の測定履歴から外れ値を除外
  

  Serial.print("Left: ");
  Serial.print(left_cm);
  Serial.print("cm, Right: ");
  Serial.print(right_cm);
  Serial.print("cm");

  left_cm = leftFilter.filter(left_cm);
  right_cm = rightFilter.filter(right_cm);
  
  delay(10);

  // 左右センサの平均距離をもとに、目標距離を保つ速度を計算
  float frontDistance = (left_cm + right_cm) / 2.0;
  int adaptiveSpeed = computeTraceSpeed(frontDistance);
  Serial.print(", Speed: ");
  Serial.print(adaptiveSpeed);
  Serial.println();

  if (left_cm == 0 && right_cm == 0){
    // センサ未検出（初期状態など）→ 何もしない

  } else {
    // 片側だけが遠い（左右差がdiffLimit以上）→ カーブ方向を最優先・即座に判定
    DriveMode desiredMode;
    if (left_cm + diffLimit < right_cm) {
      desiredMode = TURN_LEFT;   // 右が遠い→左へカーブ
    } else if (right_cm + diffLimit < left_cm) {
      desiredMode = TURN_RIGHT;  // 左が遠い→右へカーブ
    } else {
      desiredMode = STRAIGHT;    // 左右差なし（両方遠い or 両方近い）
    }

    if (desiredMode != STRAIGHT || currentMode == STRAIGHT) {
      // 明確なカーブ判定、または元々直進中 → そのまま反映
      currentMode = desiredMode;
      loseCount = 0;
    } else {
      // 旋回中に左右差が一瞬消えても、すぐには直進に戻さず
      // LOSE_CONFIRM回連続で確認できてから直進に切り替える（曲がりきる前に離脱するのを防ぐ）
      loseCount++;
      if (loseCount >= LOSE_CONFIRM) {
        currentMode = STRAIGHT;
        loseCount = 0;
      }
    }

    if (currentMode == TURN_LEFT) {
      curveLeft(adaptiveSpeed - 10);
    } else if (currentMode == TURN_RIGHT) {
      curveRight(adaptiveSpeed - 10);
    } else {
      forward(adaptiveSpeed);
    }
  }
}