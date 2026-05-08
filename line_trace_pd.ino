// Arduino Uno のピン配置
const int motor_r1 = 2;
const int motor_r2 = 3;
const int pwm_motor_r = 10;
const int motor_l1 = 4;
const int motor_l2 = 5;
const int pwm_motor_l = 11;

// フォトリフレクタ（左・右）
const int sensor_l = A3;
const int sensor_r = A4;

// ---- チューニングパラメータ ----
// センサ生値のキャリブレーション
// シリアルプロッタで val_l, val_r を見て、白床と黒線上の値を実測して入れる
const int WHITE_VAL = 600;   // 白床上での analogRead 値（おおよそ）
const int BLACK_VAL = 900;   // 黒線上での analogRead 値（おおよそ）

// 速度（0-255）
const int BASE_SPEED   = 100;  // 直線時のベース速度
const int MIN_SPEED    = 80;   // カーブ時の最低ベース速度（ここまで落ちる）
const int MAX_OUTPUT   = 120;  // モータへ出すPWMの上限（飽和防止）

// PDゲイン（実機で調整。まずはKpから）
const float Kp = 0.45;   // 比例：ズレに対する反応の強さ
const float Kd = 3.5;    // 微分：ズレの変化に対する反応（揺り戻しを抑える）

// カーブ時減速の感度。errorの絶対値がこの値で MIN_SPEED まで落ちる
const int CURVE_SLOWDOWN_AT = 70;

// 完全ロスト時の探索旋回速度
const int LOST_OUTER = 110;
const int LOST_INNER = -40;   // 負値=逆転（超信地旋回でその場で線を探す）

// ---- 状態変数 ----
float last_error = 0;
int last_turn = 0;            // 1=右寄り, 2=左寄り（ロスト時の探索方向）
const int LOST_THRESHOLD = WHITE_VAL + 80;  // 両センサがこれ未満ならロスト判定

void setup() {
  pinMode(motor_r1, OUTPUT);
  pinMode(motor_r2, OUTPUT);
  pinMode(motor_l1, OUTPUT);
  pinMode(motor_l2, OUTPUT);
  pinMode(pwm_motor_r, OUTPUT);
  pinMode(pwm_motor_l, OUTPUT);
  Serial.begin(9600);
}

void drive(int leftSpeed, int rightSpeed) {
  // 左
  if (leftSpeed >= 0) {
    digitalWrite(motor_l1, HIGH);
    digitalWrite(motor_l2, LOW);
  } else {
    digitalWrite(motor_l1, LOW);
    digitalWrite(motor_l2, HIGH);
    leftSpeed = -leftSpeed;
  }
  if (leftSpeed > 255) leftSpeed = 255;
  analogWrite(pwm_motor_l, leftSpeed);

  // 右
  if (rightSpeed >= 0) {
    digitalWrite(motor_r1, HIGH);
    digitalWrite(motor_r2, LOW);
  } else {
    digitalWrite(motor_r1, LOW);
    digitalWrite(motor_r2, HIGH);
    rightSpeed = -rightSpeed;
  }
  if (rightSpeed > 255) rightSpeed = 255;
  analogWrite(pwm_motor_r, rightSpeed);
}

void loop() {
  int val_l = analogRead(sensor_l);
  int val_r = analogRead(sensor_r);

  // ---- ロスト判定（両センサとも白床レベル）----
  // 両方白なら、直前の旋回方向で探しに行く
  if (val_l < LOST_THRESHOLD && val_r < LOST_THRESHOLD) {
    if (last_turn == 1) {
      drive(LOST_OUTER, LOST_INNER);   // 右で探す
    } else if (last_turn == 2) {
      drive(LOST_INNER, LOST_OUTER);   // 左で探す
    } else {
      drive(BASE_SPEED, BASE_SPEED);   // 履歴なし→直進
    }
    Serial.print(val_l); Serial.print('\t');
    Serial.print(val_r); Serial.print('\t');
    Serial.println("LOST");
    return;
  }

  // ---- 比例制御用の error 計算 ----
  // 左右のアナログ値の差をそのまま誤差として使う
  // 線が右寄り → 右センサが黒く近づく → val_r 大 → error 正
  // 線が左寄り → val_l 大 → error 負
  float error = (float)(val_r - val_l);

  // ---- PD 制御 ----
  float derivative = error - last_error;
  float correction = Kp * error + Kd * derivative;
  last_error = error;

  // ---- カーブ時減速 ----
  // |error| が大きいほどベース速度を落とす
  float err_abs = fabs(error);
  int base = BASE_SPEED;
  if (err_abs > 0) {
    int reduce = (int)((BASE_SPEED - MIN_SPEED) * (err_abs / (CURVE_SLOWDOWN_AT * 10.0)));
    if (reduce > BASE_SPEED - MIN_SPEED) reduce = BASE_SPEED - MIN_SPEED;
    base = BASE_SPEED - reduce;
  }

  // ---- 左右モータ速度の決定 ----
  // error 正（右寄り）→ 右に曲がりたい → 左輪速く、右輪遅く
  int left  = base + (int)correction;
  int right = base - (int)correction;

  // 出力上限。下限は -MAX_OUTPUT（逆転を許可してピボット可能）
  if (left  >  MAX_OUTPUT) left  =  MAX_OUTPUT;
  if (left  < -MAX_OUTPUT) left  = -MAX_OUTPUT;
  if (right >  MAX_OUTPUT) right =  MAX_OUTPUT;
  if (right < -MAX_OUTPUT) right = -MAX_OUTPUT;

  drive(left, right);

  // 旋回方向の記録（ロスト時の探索に使う）
  if (error > 100) last_turn = 1;       // 右寄り
  else if (error < -100) last_turn = 2; // 左寄り

  // デバッグ出力
  Serial.print(val_l);  Serial.print('\t');
  Serial.print(val_r);  Serial.print('\t');
  Serial.print((int)error); Serial.print('\t');
  Serial.print((int)correction); Serial.print('\t');
  Serial.print(left);   Serial.print('\t');
  Serial.println(right);
}
