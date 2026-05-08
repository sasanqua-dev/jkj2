// ACM0802C-NLW-BBH （8文字×2行 キャラクタLCD / HD44780互換）に
// 顔文字を順に表示するサンプル。
// 標準の LiquidCrystal ライブラリで 4bit モード接続。
//
// 配線（Arduino Uno）:
//   LCD  1番 Vss  -> GND
//   LCD  2番 Vdd  -> +5V
//   LCD  3番 V0   -> 10kΩ可変抵抗の中点（コントラスト調整）
//   LCD  4番 RS   -> D7
//   LCD  5番 R/W  -> GND（必ず GND。書き込み専用）
//   LCD  6番 E    -> D4（本機は D8 不良のため D4 を使用）
//   LCD  7-10番 D0-D3 -> 未接続（4bitモード）
//   LCD  11番 D4  -> D9
//   LCD  12番 D5  -> D6
//   LCD  13番 D6  -> D13
//   LCD  14番 D7  -> D12
//   LCD  15番 A   -> +5V（必要なら抵抗経由）
//   LCD  16番 K   -> GND
//
// ※ line_trace.ino と同時に動かす場合は、モータ用に使っている
//   D2〜D5,D10,D11 を避けてピンを選んでいます。

#include <LiquidCrystal.h>

// rs, e, d4, d5, d6, d7
// E=D2, RS=D3 に逃がした版（D4/D7 が怪しいので切り分け）
LiquidCrystal lcd(3, 2, 9, 6, 13, 12);
// 元の構成（戻すならこちら）：
// LiquidCrystal lcd(7, 4, 9, 6, 13, 12);
// もしデータ線が逆順なら：
// LiquidCrystal lcd(7, 4, 12, 13, 6, 9);

// ---- 表示する顔文字 ----
// 8文字×2行に収めるため、1行あたり最大8文字までにする。
// HD44780 標準フォントに含まれる ASCII 文字だけで構成。
const char* kaomojiList[] = {
  "(^_^)",   // にこにこ
  "(>_<)",   // ぎゅっ
  "(T_T)",   // 泣き
  "(^o^)",   // わーい
  "(-_-)",   // むすっ
  "(*_*)",   // びっくり
  "(=_=)",   // ねむい
  "(^^)v",   // ピース
};
const int KAOMOJI_NUM = sizeof(kaomojiList) / sizeof(kaomojiList[0]);

// ---- カスタム文字（ハート）----
// CGRAM スロット 0 に 5x8 のハートを登録して \0 で参照する。
byte heart[8] = {
  0b00000,
  0b01010,
  0b11111,
  0b11111,
  0b11111,
  0b01110,
  0b00100,
  0b00000,
};

void setup() {
  Serial.begin(9600);
  Serial.println("setup start");
  delay(100);               // 電源安定待ち（LCD初期化前）

  lcd.begin(8, 2);          // 8桁×2行
  Serial.println("lcd.begin done");
  lcd.createChar(0, heart); // CGRAM[0] にハートを登録
  lcd.clear();

  // 起動メッセージ
  lcd.setCursor(0, 0);
  lcd.print("Hello!  ");
  lcd.setCursor(0, 1);
  lcd.print("Kaomoji ");
  delay(1500);
}

void loop() {
  for (int i = 0; i < KAOMOJI_NUM; i++) {
    lcd.clear();

    // 1行目：番号とハート
    lcd.setCursor(0, 0);
    lcd.print("No.");
    lcd.print(i + 1);
    lcd.print(" ");
    lcd.write((byte)0); // ハート

    // 2行目：顔文字（中央寄せっぽく1文字下げ）
    lcd.setCursor(1, 1);
    lcd.print(kaomojiList[i]);

    delay(1200);
  }
}
