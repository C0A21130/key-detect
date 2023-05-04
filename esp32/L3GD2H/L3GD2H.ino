#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>

#define SDA_PIN 21
#define SCL_PIN 22
#define DATA_SIZE 16
#define X_DEVIATION 2.5
#define Y_DEVIATION 2.0
const byte L3GD2H = 0x6A;
const char* ssid = "<MY_SSID>";
const char* passwd = "<MY_PASSWORD>";

float senser_data_x[DATA_SIZE], senser_data_y[DATA_SIZE];
int head = -3;
float ave_x, ave_y;
bool key_status = false;
WebServer server(80);

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

// 鍵の情報をjsonで返却する関数(鍵を置いてある・置いてない)
void handleJson() {
  String json;
  if (key_status) { // 鍵が置いてあるとき
    json = "{\"status\" : \"HIT\"}";
  } else { // 鍵が置いてないとき
    json = "{\"status\" : \"NO\"}";
  }
  server.send(200, "application/json", json);
}

void handleNotFound(void) {
  server.send(404, "application/json", "{\"status\" : \"ERROR\"}");
}

void setup() {
  Serial.begin(9600);
  
  // ジャイロセンサーと通信の開始
  Wire.begin();
  byte who_i_am = readRegister(0x0F);
  Serial.println(who_i_am, HEX);
  writeRegister(0x20, 0x0F); // ジャイロセンサーの設定(CTRL1をDR:0,BW:0,PD:NormalMode,Z/y/X:Default)
  
  // SDA, SCLの内部プルアップを明示
  pinMode(SDA_PIN, INPUT_PULLUP);
  pinMode(SCL_PIN, INPUT_PULLUP);

  // WebServerの開始
  WiFi.begin(ssid, passwd);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  server.on("/", handleJson);
  server.onNotFound(handleNotFound);
  server.begin();

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
    key_status = key_status ? false : true;
  } else {
    if (flagx) { // X軸の角速度の値が大幅に変動したとき
      Serial.println("X !!");
    }   
    if (flagy) { // y軸の角速度の値が大幅に変動したとき
      Serial.println("Y !!"); 
    }
  }

  // クライエントからのアクセスを処理
  server.handleClient();
    
  delay(1000);
}
