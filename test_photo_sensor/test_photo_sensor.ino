void setup() {
  // put your setup code here, to run once:
  pinMode(13, OUTPUT);
  Serial.begin(9600);
}

void loop() {
     // LEDを点灯(HIGHは電圧レベルを5Vにする)    // LEDを消灯(LOWは電圧レベルを0Vにする)                   // 1秒間(1000ms)待つ
  int val_right = analogRead(A3);
  int val_left = analogRead(A4);
  analogWrite(11, 150);
  Serial.println(val_right, val_left);
  if (val_left >= 750 && val_right >= 750) {
    // L: 黒 R: 黒 → 直進
    forward();
  } else if (val_left >= 750 && val_right < 750) {
    // L: 黒 R: 白 → 左に曲がる
    turnLeft();
  } else if (val_left < 750 && val_right < 750) {
    // L: 白 R: 白 → 要検討
    forward();
  } else if (var_left < 750 && var_right >= 750) {
    // L: 白 R: 黒 → 右に曲がる
    turnRight();
  }
  delay(500);
}