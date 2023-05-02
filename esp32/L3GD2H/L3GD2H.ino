#include <Wire.h>

#define SDA_PIN 21
#define SCL_PIN 22
#define DATA_SIZE 16
#define X_DEVIATION 2.5
#define Y_DEVIATION 2.0

const byte L3GD2H = 0x6A;

float senser_data_x[DATA_SIZE], senser_data_y[DATA_SIZE];
int head = -3;
float ave_x, ave_y;
int out = 0;

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

// 配列の平均値を求める関数(配列、配列の長さ) : 平均値
float getAverage(float senser_data[]) {
  float sum = 0.0;
  
  for (int i = 0; i < DATA_SIZE; i++) {
    sum += senser_data[i];
  }
  return sum / DATA_SIZE;
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
  byte xr, yr;
  float x, y;
  bool flagx, flagy, flag;
  
  // xの角速度の値の求める  
  l = readRegister(0x28);
  h = readRegister(0x29);
  xr = h << 8 | l;
  x = (float) xr * 0.07;

  // yの角速度の値の求める
  l = readRegister(0x2A);
  h = readRegister(0x2B);
  yr = h << 8 | l;
  y = (float) yr * 0.07;

  if (head == DATA_SIZE) { // DATA_SIZEの数のセンサーの値が求められたとき
    ave_x = getAverage(senser_data_x);
    ave_y = getAverage(senser_data_y);
    head = DATA_SIZE + 1;
    return;
  } else if(head < DATA_SIZE) {
    head++;
    // 最初の16個のx,yの値を記録する
    if (head >= 0) { // まだセンサーの値が足りないときセンサーの値を配列に記録する
      senser_data_x[head] = x;
      senser_data_y[head] = y;
    }
    delay(500);
    return;
  }
  
  // センサーの初期の平均値が求まったあと
  // x, yの角速度の結果を出力する    
  Serial.print(x);
  Serial.print(",");
  Serial.println(y);
    
  // x, yの角速度が大幅に変動したことを確認する
  flagx = ((ave_x + X_DEVIATION) < x) || ((ave_x - X_DEVIATION) > x);
  flagy = ((ave_y + Y_DEVIATION) < y) || ((ave_y - Y_DEVIATION) > y);
  flag  = flagx && flagy;
  if (flag) { // x, y軸の角速度の両方が大幅に変動したとき
    Serial.println("HIT!!");
  } else {
    if (flagx) { // X軸の角速度の値が大幅に変動したとき
      Serial.println("X !!");
    }   
    if (flagy) { // y軸の角速度の値が大幅に変動したとき
      Serial.println("Y !!"); 
    }
  }
    
  delay(1000);
}
