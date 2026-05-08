// Arduino Uno のピン配置
const int motor_r1 = 2;
const int motor_r2 = 3;
const int pwm_motor_r = 10;
const int motor_l1 = 4;
const int motor_l2 = 5;
const int pwm_motor_l = 11;

// フォトリフレクタ（左・右）── 黒線を左右から挟む配置
const int sensor_l = A3;
const int sensor_r = A4;

// ---- センサ判定 ----
// 黒線上では analogRead 値が大きくなる想定
const int threshold = 750;

// ---- 速度設定（動作範囲 70〜130）----
const int MIN_DRIVE    = 0;   // 動作下限
const int MAX_DRIVE    = 130;  // 動作上限
const int BASE_SPEED   = 100;  // 直進時の基本速度（範囲のほぼ中央）
const int OUTER_GENTLE = 90;  // 緩いカーブ時の外輪
const int INNER_GENTLE = 40;   // 緩いカーブ時の内輪
const int OUTER_SHARP  = 90;  // 急カーブ（両方黒）時の外輪
const int INNER_SHARP  = 0;   // 急カーブ時の内輪（=MIN_DRIVE、下限）

// 直近の旋回方向: 0=なし, 1=右, 2=左
int last_turn = 0;

void setup() {
  pinMode(motor_r1, OUTPUT);
  pinMode(motor_r2, OUTPUT);
  pinMode(motor_l1, OUTPUT);
  pinMode(motor_l2, OUTPUT);
  pinMode(pwm_motor_r, OUTPUT);
  pinMode(pwm_motor_l, OUTPUT);
  Serial.begin(9600);
}

// 左右の前進速度を直接指定。範囲外は丸める
void drive(int leftSpeed, int rightSpeed) {
  if (leftSpeed  < MIN_DRIVE) leftSpeed  = MIN_DRIVE;
  if (rightSpeed < MIN_DRIVE) rightSpeed = MIN_DRIVE;
  if (leftSpeed  > MAX_DRIVE) leftSpeed  = MAX_DRIVE;
  if (rightSpeed > MAX_DRIVE) rightSpeed = MAX_DRIVE;

  digitalWrite(motor_l1, HIGH);
  digitalWrite(motor_l2, LOW);
  analogWrite(pwm_motor_l, leftSpeed);

  digitalWrite(motor_r1, HIGH);
  digitalWrite(motor_r2, LOW);
  analogWrite(pwm_motor_r, rightSpeed);
}

void stopMotor() {
  digitalWrite(motor_l1, LOW);
  digitalWrite(motor_l2, LOW);
  analogWrite(pwm_motor_l, 0);
  digitalWrite(motor_r1, LOW);
  digitalWrite(motor_r2, LOW);
  analogWrite(pwm_motor_r, 0);
}

void loop() {
  int val_l = analogRead(sensor_l);
  int val_r = analogRead(sensor_r);

  bool on_l = (val_l >= threshold);
  bool on_r = (val_r >= threshold);

  // ---- センサパターンによる分岐（挟む配置）----
  //  LR
  //  00 … 両方白：線は中央（センサ間）→ 直進
  //  10 … 左センサが黒：機体が右にズレた → 左へ戻す
  //  01 … 右センサが黒：機体が左にズレた → 右へ戻す
  //  11 … 両方黒：交差点 or 急カーブで線をまたいだ → 直近の旋回を継続

  if (!on_l && !on_r) {
    // 両方白 → 線が中央。直進
    drive(BASE_SPEED, BASE_SPEED);
  }
  else if (on_l && !on_r) {
    // 左が黒 = 右にズレている → 左へ戻す
    // 左輪を遅く、右輪を速く
    drive(INNER_GENTLE, OUTER_GENTLE);
    last_turn = 2;  // 左に切った
  }
  else if (!on_l && on_r) {
    // 右が黒 = 左にズレている → 右へ戻す
    drive(OUTER_GENTLE, INNER_GENTLE);
    last_turn = 1;  // 右に切った
  }
  else {
    // 両方黒 → 急カーブ進入。直近の旋回方向で鋭く曲がる
    if (last_turn == 1) {
      drive(OUTER_SHARP, INNER_SHARP);   // 右へ鋭く
    } else if (last_turn == 2) {
      drive(INNER_SHARP, OUTER_SHARP);   // 左へ鋭く
    } else {
      drive(BASE_SPEED, BASE_SPEED);     // 履歴なし → 直進
    }
  }

  // デバッグ出力
  Serial.print(val_l); Serial.print('\t');
  Serial.print(val_r); Serial.print('\t');
  Serial.print(on_l);  Serial.print(on_r); Serial.print('\t');
  Serial.println(last_turn);
}
