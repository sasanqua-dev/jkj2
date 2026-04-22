// Arduino Uno のピン配置
// モータ
const int motor_r1 = 2; // Arduinoの2番ピンに対応
const int motor_r2 = 3;
const int pwm_motor_r = 10; // PWM信号生成可能なピンを選ぶ
const int motor_l1 = 4;
const int motor_l2 = 5;
const int pwm_motor_l = 11;

int last_turn_flag = 0; // 1 = 右 2 = 左
// フォトリフレクタ
const int sensor = A3;
// センサから読み取った値（analog）
int val = 0;

// ライントレース用のパラメータ
// 黒い線の上では反射が少なく analogRead の値が大きくなる想定
// 実機に合わせて threshold を調整する
const int threshold = 500;    // 白／黒を判定するしきい値
const int traceSpeed = 180;   // ライントレース中の基本速度（0-255）

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

void turnLeft(int speedBase = 150) { // 左に曲がる関数
  digitalWrite(motor_l1, LOW);
  digitalWrite(motor_l2, HIGH);
  analogWrite(pwm_motor_l, speedBase + 25);
  digitalWrite(motor_r1, HIGH);
  digitalWrite(motor_r2, LOW);
  analogWrite(pwm_motor_r, speedBase);
}

void turnRight(int speedBase = 150) { // 右に曲がる関数
  digitalWrite(motor_l1, HIGH);
  digitalWrite(motor_l2, LOW);
  analogWrite(pwm_motor_l, speedBase);
  digitalWrite(motor_r1, LOW);
  digitalWrite(motor_r2, HIGH);
  analogWrite(pwm_motor_r, speedBase + 25);
}

// ライントレース用：両輪とも前進させつつ左右の速度差で緩やかに曲がる
// diff が大きいほど曲がりが鋭くなる
void curveLeft(int speedBase = 180, int diff = 80) { // 左方向へ緩やかに曲がる
  int l = speedBase - diff;
  int r = speedBase + diff;
  if (l < 0) l = 0;
  if (r > 255) r = 255;
  digitalWrite(motor_l1, HIGH);
  digitalWrite(motor_l2, LOW);
  analogWrite(pwm_motor_l, l);
  digitalWrite(motor_r1, HIGH);
  digitalWrite(motor_r2, LOW);
  analogWrite(pwm_motor_r, r);
}

void curveRight(int speedBase = 180, int diff = 80) { // 右方向へ緩やかに曲がる
  int l = speedBase + diff;
  int r = speedBase - diff;
  if (l > 255) l = 255;
  if (r < 0) r = 0;
  digitalWrite(motor_l1, HIGH);
  digitalWrite(motor_l2, LOW);
  analogWrite(pwm_motor_l, l);
  digitalWrite(motor_r1, HIGH);
  digitalWrite(motor_r2, LOW);
  analogWrite(pwm_motor_r, r);
}

void loop() { // 制御プログラム（1センサによるエッジ追従式ライントレース）
  // センサの値をアナログ値で読み取る
  val = analogRead(sensor);
  // シリアルポートに読み取った値を表示させる（threshold 調整に利用）
  Serial.println(val);

  // 1つのフォトリフレクタで線の「縁」を追う
  //   黒い線の上（val >= threshold）… 線の内側なので外側（右）へ舵を切る
  //   白い床の上（val <  threshold）… 線から外れたので内側（左）へ戻す
  // curveLeft/curveRight は両輪とも前進させつつ左右速度差で旋回するため、
  // 前進しながら小刻みに蛇行して線の縁をトレースできる
  if (val >= threshold) {
    curveRight(traceSpeed);
  } else {
    curveLeft(traceSpeed);
  }
  delay(20); // 0.02 秒待つ（動作を続ける）
}
