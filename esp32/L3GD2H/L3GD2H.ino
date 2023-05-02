#include <Wire.h>

#define SDA_PIN 21
#define SCL_PIN 22

const byte L3GD2H = 0x6A;

// レジスタの値を読み取る関数(メモリアドレス)
byte readRegister(byte address) {
  // I2C通信の接続確立 
  Wire.beginTransmission(L3GD2H);
  Wire.write(address);
  Wire.endTransmission(false);
  // 指定したアドレスのレジスタの値の読み取って返却
  Wire.requestFrom(L3GD2H, 1, true);
  return Wire.read();
}

// レジスタの値を読み取る関数(メモリアドレス, 書き込むデータ)
void writeRegister(byte address, byte data) {
  Wire.beginTransmission(L3GD2H);
  Wire.write(address);
  Wire.write(data);
  Wire.endTransmission();
}

void setup() {
  pinMode(SDA_PIN, INPUT_PULLUP);
  pinMode(SCL_PIN, INPUT_PULLUP);
  // I2C通信の開始
  Wire.begin();
  Serial.begin(9600);
  
  byte who_i_am = readRegister(0x0F);
  Serial.println(who_i_am, HEX);

  writeRegister(0x20, 0x0F);

  Serial.println("x,y");
}

void loop() {
  byte l, h; // l:last, h:header
  byte x, y, z;

  // xの角速度の値の求める  
  l = readRegister(0x28);
  h = readRegister(0x29);
  x = h << 8 | l;
  Serial.print(x * 0.07);
  Serial.print(",");

  // yの角速度の値の求める
  l = readRegister(0x2A);
  h = readRegister(0x2B);
  y = h << 8 | l;
//  Serial.print("y : ");
  Serial.println(y * 0.07);
  
  delay(1000);
}
