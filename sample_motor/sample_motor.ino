// Arduino Uno のピン配置
// モータ
const int motor_r1 = 2; // Arduinoの2番ピンに対応
const int motor_r2 = 3;
const int pwm_motor_r = 10; // PWM信号生成可能なピンを選ぶ
const int motor_l1 = 4;
const int motor_l2 = 5;
const int pwm_motor_l = 11;
// フォトリフレクタ
const int sensor = A3;
// センサから読み取った値（analog）
int val = 0;

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

void forward(int speedBase = 200) { // 前進させる関数
  digitalWrite(motor_l1, HIGH);
  digitalWrite(motor_l2, LOW);
  analogWrite(pwm_motor_l, speedBase);
  digitalWrite(motor_r1, HIGH);
  digitalWrite(motor_r2, LOW);
  analogWrite(pwm_motor_r, speedBase);
}

void backward(int speedBase = 200) { // 後退させる関数
  digitalWrite(motor_l1, LOW);
  digitalWrite(motor_l2, HIGH);
  analogWrite(pwm_motor_l, speedBase);
  digitalWrite(motor_r1, LOW);
  digitalWrite(motor_r2, HIGH);
  analogWrite(pwm_motor_r, speedBase);
}

void turnLeft(int speedBase = 200) { // 左に曲がる関数
  digitalWrite(motor_l1, LOW);
  digitalWrite(motor_l2, HIGH);
  analogWrite(pwm_motor_l, speedBase + 25);
  digitalWrite(motor_r1, HIGH);
  digitalWrite(motor_r2, LOW);
  analogWrite(pwm_motor_r, speedBase - 25);
}

void turnRight(int speedBase = 200) { // 右に曲がる関数
  digitalWrite(motor_l1, HIGH);
  digitalWrite(motor_l2, LOW);
  analogWrite(pwm_motor_l, speedBase - 25);
  digitalWrite(motor_r1, LOW);
  digitalWrite(motor_r2, HIGH);
  analogWrite(pwm_motor_r, speedBase + 25);
}

void loop() { // 制御プログラム
  // センサの値をアナログ値で読み取る
  val = analogRead(sensor);
  // シリアルポートに読み取った値を表示させる
  Serial.println(val); // 表示して改行
  // ここから制御分岐
  forward(255);
  delay(20); // 0.02 秒待つ（動作を続ける）
}
