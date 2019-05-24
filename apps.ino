#include "itlapps.h"

// 加速度・ジャイロを格納する構造体
struct sensorData {
  int accX;
  int accY;
  int accZ;
  int gyroX;
  int gyroY;
  int gyroZ;
};

// プロトタイプ宣言
void getAcc(MPU9250* IMU, sensorData* pSensorData);
void _readSensor(MPU9250* IMU);


/**
 * @brief 加速度・ジャイロをリアルタイムで計測・表示
 *
 * スタート、ストップ機能付き。ループ間隔は15.625ms(64Hz)
 * 結果表示ごとに画面をクリアしているため、画面がちらつきます。
 *
 * @attention 加速度の初期値をそろえるため、既定の初期値になるまで初期化を続けます。
 */
void appDrawAccGyro() {
  MyMenu.drawAppMenu(F("current Acc and Gyro"),F(""),F("EXIT"),F(""));

  while(M5.BtnB.wasPressed()){
    M5.update();
  }

  // 加速度・ジャイロセンサの宣言・初期化
  M5.Lcd.drawCentreString("Loading...", LCDcenterX, LCDcenterY2, 2);
  delay(1000);
  MPU9250 IMU;
  struct sensorData sensorData = {0, 0, 0, 0, 0, 0};

  while(true) {
    IMU.calibrateMPU9250(IMU.gyroBias, IMU.accelBias);
    IMU.initMPU9250();
    getAcc(&IMU, &sensorData);

    if(abs(sensorData.accX - sensorData.accY) < 70) {
      break;
    }
  }

  MyMenu.drawAppMenu(F("Current Acc and Gyro"),F("START"),F("EXIT"),F(""));
  MyMenu.windowClr();
  M5.Lcd.drawCentreString("Press START to start measure", LCDcenterX, LCDcenterY2, 2);

    while(!M5.BtnB.wasPressed()){

      if(M5.BtnA.wasPressed()) {
        MyMenu.drawAppMenu(F("Current Acc and Gyro"),F(""),F("EXIT"),F("STOP"));

        // 加速度・ジャイロ表示ループ
        while(!M5.BtnC.wasPressed()) {
          getAcc(&IMU, &sensorData);
          MyMenu.windowClr();
          // 加速度を表示
          M5.Lcd.drawCentreString("Acceleration", LCDcenterX, LCDcenterY1, 2);
          M5.Lcd.drawCentreString(String(sensorData.accX)+"  "+
                                  String(sensorData.accY)+"  "+
                                  String(sensorData.accZ),
                                  LCDcenterX, LCDcenterY2, 2);
          // ジャイロを表示
          M5.Lcd.drawCentreString("Gyro", LCDcenterX, LCDcenterY3, 2);
          M5.Lcd.drawCentreString(String(sensorData.gyroX)+"  "+
                                  String(sensorData.gyroY)+"  "+
                                  String(sensorData.gyroZ),
                                  LCDcenterX, LCDcenterY3+15, 2);
          delay(15.625);
          M5.update();
        }

        // 加速度・ジャイロ表示ループ終了
        MyMenu.drawAppMenu(F("Current Acc and Gyro"),F("START"),F("EXIT"),F(""));
        MyMenu.windowClr();
        M5.Lcd.drawCentreString("Press START to start measure", LCDcenterX, LCDcenterY2, 2);
      }
    M5.update();
    }
    MyMenu.show();
}


/**
 * @brief 現在時刻を表示する。
 *
 */
void appShowNowTime() {
  MyMenu.drawAppMenu(F("Now Time"),F("REFRESH"),F("EXIT"),F(""));

  while(M5.BtnB.wasPressed()){
    M5.update();
  }

  M5.Lcd.drawCentreString(nowTime(), LCDcenterX, LCDcenterY2, 2);

  while(!M5.BtnB.wasPressed()){
    if(M5.BtnA.wasPressed()) {
      M5.Lcd.drawCentreString(nowTime(), LCDcenterX, LCDcenterY2, 2);
    }
    M5.update();
  }

  MyMenu.show();
}


/**
 * @fn
 * getAcc
 * @brief センサから加速度とジャイロを取得する
 * @param[in] MPU 9軸センサのインスタンス
 * @param[in,out] *pSensorData センサデータ構造体のポインタ
 * @return なし。与えられたポインタ先のメンバに直接書き込む
 */
void getAcc(MPU9250* IMU, sensorData* pSensorData) {
  // センサから各種情報を読み取り
  IMU->readAccelData(IMU->accelCount);
  IMU->getAres();
  IMU->readGyroData(IMU->gyroCount);
  IMU->getGres();

  // 取得した加速度に解像度をかけて、バイアス値を引く
  IMU->ay = (float)IMU->accelCount[1]*IMU->aRes - IMU->accelBias[1];
  IMU->az = (float)IMU->accelCount[2]*IMU->aRes - IMU->accelBias[2];
  IMU->ax = (float)IMU->accelCount[0]*IMU->aRes - IMU->accelBias[0];

  // 取得したジャイロに解像度をかける
  IMU->gx = (float)IMU->gyroCount[0]*IMU->gRes;
  IMU->gy = (float)IMU->gyroCount[1]*IMU->gRes;
  IMU->gz = (float)IMU->gyroCount[2]*IMU->gRes;

  // 四元数を更新する際に必ず呼び出し
  IMU->updateTime();

  // 加速度・ジャイロを与えられた構造体に代入
  pSensorData -> accX = (int)(1000*IMU->ax);
  pSensorData -> accY = (int)(1000*IMU->ay);
  pSensorData -> accZ = (int)(1000*IMU->az);
  pSensorData -> gyroX = (int)(IMU->gx);
  pSensorData -> gyroY = (int)(IMU->gy);
  pSensorData -> gyroZ = (int)(IMU->gz);

}

// 画面の輝度を設定
void appSetBrightness(){
  byte tmp_brigth = byte(EEPROM.read(0));
  byte tmp_lbrigth = 0;

  MyMenu.drawAppMenu(F("DISPLAY BRIGHTNESS"),F("-"),F("OK"),F("+"));

  while(M5.BtnB.wasPressed()){
    M5.update();
  }

  while(!M5.BtnB.wasPressed()){
    if(M5.BtnA.wasPressed() and tmp_brigth >= 16){
      tmp_brigth = tmp_brigth - 16;
    }
    if(M5.BtnC.wasPressed() and tmp_brigth <= 239){
      tmp_brigth = tmp_brigth + 16;
    }
    if(tmp_lbrigth != tmp_brigth){
      tmp_lbrigth = tmp_brigth;
      EEPROM.write(0,tmp_lbrigth);
      EEPROM.commit();
      M5.lcd.setBrightness(byte(EEPROM.read(0)));
      MyMenu.windowClr();
      M5.Lcd.drawNumber(byte(EEPROM.read(0)), 120 , 90, 6);
    }
    M5.update();
  }
  MyMenu.show();
}


// スリープ（充電時などに使用）
void appSleep(){
  M5.setWakeupButton(BUTTON_B_PIN);
  M5.powerOFF();
}

// 乱数ジェネレータ(10文字)
void rand_text(char *randStr) {
  int i;
  char char_set[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  // 起動からの経過時間（マイクロ秒）をシード値に。
  srand(getMicroSec());

  for (i = 0; i < 10; i++) {
    randStr[i] = char_set[rand() % 61 + 1];
  }
  randStr[10] = 0;
}


// 現在時刻のマイクロ秒部分を取得し、返す。rand()のシード用
unsigned long getMicroSec() {
  struct timeval tv = {0};
  gettimeofday(&tv, NULL);

  return tv.tv_usec;
}

// MACアドレスを取得。間のダブルコロンは省きます
String getMacAddr() {
  uint8_t baseMac[6];
  esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
  char baseMacChr[16] = {0};
  sprintf(baseMacChr, "%02X%02X%02X%02X%02X%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
  return String(baseMacChr);
}




// ----- アプリ作成時のテンプレート -----

// void appTEMPLATE() {
//   MyMenu.drawAppMenu(F("TITLE"),F(""),F(""),F(""));

//   while(M5.BtnB.wasPressed()) M5.update();

//   ループ開始前の処理はここで

//   while(!M5.BtnB.wasPressed()) { // メインループ
//     M5.update();
//   }

//   MyMenu.show();
// }