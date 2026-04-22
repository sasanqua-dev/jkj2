// Arduino Uno のピン配置
// モータ
const int motor_r1 = 2; // Arduinoの2番ピンに対応
const int motor_r2 = 3;
const int pwm_motor_r = 10; // PWM信号生成可能なピンを選ぶ
const int motor_l1 = 4;
const int motor_l2 = 5;
const int pwm_motor_l = 11;

int last_turn_flag = 0; // 1 = 右 2 = 左
// フォトリフレクタ（2センサ構成：左・右）
const int sensor_l = A3; // 左センサ
const int sensor_r = A4; // 右センサ
// センサから読み取った値（analog）
int val_l = 0;
int val_r = 0;

// ライントレース用のパラメータ
// 黒い線の上では反射が少なく analogRead の値が大きくなる想定
// 実機に合わせて threshold を調整する
const int threshold = 500;    // 白／黒を判定するしきい値（復帰判定用）
const int baseSpeed = 200;    // ライントレース中の基本速度（0-255）

// PID ゲイン（実機に合わせて調整する）
// error = val_l - val_r （-1023 ～ +1023 程度のレンジ）
//   error > 0 … 左センサが黒寄り＝右にずれている → 左へ旋回
//   error < 0 … 右センサが黒寄り＝左にずれている → 右へ旋回
const float Kp = 0.15;        // 比例ゲイン
const float Kd = 0.80;        // 微分ゲイン
int prev_error = 0;

// 線を見失ったと判断するまでの時間（ms）。両方白がこの時間継続したら復帰動作に入る
const unsigned long lostLineTimeout = 500;
unsigned long both_white_since = 0; // 両方白になった時刻（0 = 両方白ではない）

void setup() { // 実行時に1回だけ実行
  pinMode(motor_r1, OUTPUT); // motor_r1 に対応するピン（2番）を出力ポートに設定
  pinMode(motor_r2, OUTPUT);
  pinMode(motor_l1, OUTPUT);
  pinMode(motor_l2, OUTPUT);
  pinMode(pwm_motor_r, OUTPUT);
  pinMode(pwm_motor_l, OUTPUT);
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

// 左右のPWM値を指定してモータを駆動する（負の値で逆回転）
void drive(int l_pwm, int r_pwm) {
  // 左モータ
  if (l_pwm >= 0) {
    digitalWrite(motor_l1, HIGH);
    digitalWrite(motor_l2, LOW);
  } else {
    digitalWrite(motor_l1, LOW);
    digitalWrite(motor_l2, HIGH);
    l_pwm = -l_pwm;
  }
  if (l_pwm > 255) l_pwm = 255;
  analogWrite(pwm_motor_l, l_pwm);
  // 右モータ
  if (r_pwm >= 0) {
    digitalWrite(motor_r1, HIGH);
    digitalWrite(motor_r2, LOW);
  } else {
    digitalWrite(motor_r1, LOW);
    digitalWrite(motor_r2, HIGH);
    r_pwm = -r_pwm;
  }
  if (r_pwm > 255) r_pwm = 255;
  analogWrite(pwm_motor_r, r_pwm);
}

void forward(int speedBase = 200) { // 前進させる関数
  drive(speedBase, speedBase);
}

void backward(int speedBase = 200) { // 後退させる関数
  drive(-speedBase, -speedBase);
}

void turnLeft(int speedBase = 150) { // 左に曲がる関数（復帰用：その場旋回）
  drive(-(speedBase + 25), speedBase);
}

void turnRight(int speedBase = 150) { // 右に曲がる関数（復帰用：その場旋回）
  drive(speedBase, -(speedBase + 25));
}

void loop() { // 制御プログラム（2センサ＋PID によるライントレース）
  // 2つのセンサをアナログ値で読み取る
  val_l = analogRead(sensor_l);
  val_r = analogRead(sensor_r);

  // 両方白（＝線がセンサ間もしくは線を見失った状態）の判定
  bool on_l = (val_l >= threshold);
  bool on_r = (val_r >= threshold);
  bool both_white = (!on_l && !on_r);

  // 両方白の連続時間を計測
  if (both_white) {
    if (both_white_since == 0) both_white_since = millis();
  } else {
    both_white_since = 0;
  }

  // 一定時間「両方白」が続いたら線を見失ったと判断して復帰動作に入る
  if (both_white && both_white_since != 0 &&
      millis() - both_white_since >= lostLineTimeout) {
    if (last_turn_flag == 1) {
      turnRight(baseSpeed);
    } else if (last_turn_flag == 2) {
      turnLeft(baseSpeed);
    } else {
      stopMotor();
    }
    // シリアル出力（error は使わないので 0）
    Serial.print(val_l); Serial.print('\t');
    Serial.print(val_r); Serial.print('\t');
    Serial.println(0);
    delay(20);
    return;
  }

  // ===== PID 制御 =====
  // 誤差：左右センサ値の差。正なら右にずれている、負なら左にずれている
  int error = val_l - val_r;
  int d_error = error - prev_error;
  float correction = Kp * (float)error + Kd * (float)d_error;

  // 左右PWMを計算：正の correction で左を遅く、右を速くして左旋回
  int l_pwm = baseSpeed - (int)correction;
  int r_pwm = baseSpeed + (int)correction;

  // クランプ（drive() 側でも上限はかかるが、下限側もここで丸める）
  if (l_pwm > 255) l_pwm = 255;
  if (r_pwm > 255) r_pwm = 255;
  if (l_pwm < -255) l_pwm = -255;
  if (r_pwm < -255) r_pwm = -255;

  drive(l_pwm, r_pwm);

  // 復帰用に直前の旋回方向を保持（誤差の符号で判定）
  if (error > 50) {
    last_turn_flag = 2; // 左
  } else if (error < -50) {
    last_turn_flag = 1; // 右
  }

  prev_error = error;

  // シリアル出力（threshold / ゲイン調整に利用）：左 / 右 / error
  Serial.print(val_l); Serial.print('\t');
  Serial.print(val_r); Serial.print('\t');
  Serial.println(error);

  delay(20); // 0.02 秒待つ（動作を続ける）
}
