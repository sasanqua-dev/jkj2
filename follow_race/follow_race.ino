//ライントレース一周 time34.60
// Arduino Uno のピン配置
// モータ
const int motor_r1 = 2; // Arduinoの2番ピンに対応
const int motor_r2 = 3;
const int pwm_motor_r = 10; // PWM信号生成可能なピンを選ぶ
const int motor_l1 = 4;
const int motor_l2 = 5;
const int pwm_motor_l = 11;
const int traceSpeed = 125;   // 基本速度（0-255）

// 追従用のパラメータ
<<<<<<< HEAD
int left_foward_trig_pin = 12;    // Trigger
int left_foward_echo_pin = 13;    // Echo
int right_foward_trig_pin = 6;    // Trigger
int right_foward_echo_pin = 7;    // Echo
=======
int left_foward_trig_pin = 6;    // Trigger
int left_foward_echo_pin = 7;    // Echo
int right_foward_trig_pin = 12;    // Trigger
int right_foward_echo_pin = 13;    // Echo
>>>>>>> 677ba7e37677fe859518fa7e69407aa2cfc53b1b

long left_duration, right_duration;
float left_cm, right_cm;


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
  analogWrite(pwm_motor_l, speedBase);
  digitalWrite(motor_r1, HIGH);
  digitalWrite(motor_r2, LOW);
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
  digitalWrite(motor_l1, HIGH);
  digitalWrite(motor_l2, LOW);
  analogWrite(pwm_motor_l, speedBase + 50);
  digitalWrite(motor_r1, LOW);
  digitalWrite(motor_r2, HIGH);
  analogWrite(pwm_motor_r, speedBase - 10);
  
}

void turnRight(int speedBase = 150) { // 右に曲がる関数
  digitalWrite(motor_l1, LOW);
  digitalWrite(motor_l2, HIGH);
  analogWrite(pwm_motor_l, speedBase - 10);
  digitalWrite(motor_r1, HIGH);
  digitalWrite(motor_r2, LOW);
  analogWrite(pwm_motor_r, speedBase + 50);
}

// ライントレース用：両輪とも前進させつつ左右の速度差で緩やかに曲がる
// diff が大きいほど曲がりが鋭くなる
void curveLeft(int speedBase = 180, int diff = 30) { // 左方向へ緩やかに曲がる
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

void curveRight(int speedBase = 180, int diff = 30) { // 右方向へ緩やかに曲がる
  int l = speedBase + diff;
  int r = 0;
  if (l > 255) l = 255;
  if (r < 0) r = 0;
  digitalWrite(motor_l1, LOW);
  digitalWrite(motor_l2, HIGH);
  analogWrite(pwm_motor_l, l);
  digitalWrite(motor_r1, HIGH);
  digitalWrite(motor_r2, LOW);
  analogWrite(pwm_motor_r, r);
  delay(1);
}

void loop() {
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
  delay(20);

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

  Serial.print("Left: ");
  Serial.print(left_cm);
  Serial.print("cm, Right: ");
  Serial.print(right_cm);
  Serial.print("cm");
  Serial.println();
  
  delay(250);

  bool on_l = (left_cm >= 10);
  bool on_r = (right_cm >= 10);

  if (!on_l && !on_r) {
    // 両方白 → 連続時間を計測して、一定時間超えたら線を見失ったと判断
    forward(traceSpeed+ 45);
  } else if (on_l && !on_r) {
    // 左が線を検出 → 左へ
    turnRight(traceSpeed );
  } else if (!on_l && on_r) {
    // 右が線を検出 → 右へ
    turnLeft(traceSpeed );
  } else {
    // 両方黒（交差点・太線）→ 直進
    forward(traceSpeed);
  }
}