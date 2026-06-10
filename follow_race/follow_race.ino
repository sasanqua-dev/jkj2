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
int left_foward_trig_pin = 9;    // Trigger
int left_foward_echo_pin = 8;    // Echo
int right_foward_trig_pin = 6;    // Trigger
int right_foward_echo_pin = 7;    // Echo

long duration, cm, inches;


void setup() { // 実行時に1回だけ実行
  pinMode(motor_r1, OUTPUT); // motor_r1 に対応するピン（2番）を出力ポートに設定
  pinMode(motor_r2, OUTPUT);
  pinMode(motor_l1, OUTPUT);
  pinMode(motor_l2, OUTPUT);
  pinMode(pwm_motor_r, OUTPUT);
  pinMode(pwm_motor_l, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
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
  // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  // Generate Signal
  digitalWrite(left_foward_trig_pin, LOW);
  digitalWrite(right_foward_trig_pin, LOW);
  delayMicroseconds(5);
  digitalWrite(left_foward_trig_pin, HIGH);
  digitalWrite(right_foward_trig_pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(left_foward_trig_pin, LOW);
  digitalWrite(right_foward_trig_pin, LOW);
 
  // Read Signal
  pinMode(left_foward_echo_pin, INPUT);
  pinMode(right_foward_echo_pin, INPUT);
  left_duration = pulseIn(left_foward_echo_pin, HIGH);
  right_duration = pulseIn(right_foward_echo_pin, HIGH);

  // Convert the time into a distance
  left_cm = (left_duration/2) / 29.1;     // Divide by 29.1 or multiply by 0.0343
  right_cm = (right_duration/2) / 29.1;   // Divide by 29.1 or multiply by 0.0343

  Serial.print("Left: ");
  Serial.print(left_cm);
  Serial.print("cm, Right: ");
  Serial.print(right_cm);
  Serial.print("cm");
  Serial.println();
  
  delay(250);
}