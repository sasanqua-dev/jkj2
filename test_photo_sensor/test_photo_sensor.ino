void setup() {
  // put your setup code here, to run once:
  pinMode(13, OUTPUT);
  Serial.begin(9600);
}

void loop() {
     // LEDを点灯(HIGHは電圧レベルを5Vにする)    // LEDを消灯(LOWは電圧レベルを0Vにする)                   // 1秒間(1000ms)待つ
  int val = analogRead(A3);
  analogWrite(11, 150);
  Serial.println(val);
  if (val < 500) {
    digitalWrite(13, HIGH);
  } else {
    digitalWrite(13, LOW);
  }
  delay(100);
}