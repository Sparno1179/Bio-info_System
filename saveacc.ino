#include "itlapps.h"

//タイマー
Ticker tickerSensor; // センサの値を読む
Ticker tickerShowTime; // 計測経過時間表示
Ticker tickerWriteData; // バッファにためた加速度データをCSVに書き込み

//バッファ
sensorData *sdBuff = NULL;
int measure_time = 0;
int fileIndex = 0;
char randStr[10];
int elapsedTime = 0;
//バッファのインデックス
volatile int buffPointer = 0;
volatile bool buffSaveFlg = false;

/**
 * @brief 加速度をタイマー割り込みでとるパターン
 *
 * タイマーがミリ秒単位しかないため64Hzに最も近い16ミリ秒間隔（62.5Hz）を指定
 * 60秒計測ー＞新しいファイルで保存ー＞60秒計測.......
 */
void appAccTimer() {
  MyMenu.drawAppMenu(F("Save Acc by Timer"),F(""),F("EXIT"),F(""));

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
    delay(50);
  }

  // ファイル名用ランダム文字列の生成
  rand_text(randStr);

  MyMenu.drawAppMenu(F("Save acc and gyro"),F("OK"),F("EXIT"),F("NEXT"));
  MyMenu.windowClr();

  char movementList[7][15] = {"walk", "stand", "sit", "stdown", "stup", "lying", "vehicle"};
  int movementID = -1;
  MyMenu.clearList();
  MyMenu.setListCaption("Movement Type");
  MyMenu.addList("WALK");
  MyMenu.addList("STAND");
  MyMenu.addList("SIT");
  MyMenu.addList("DOWN_STAIR_DOWN");
  MyMenu.addList("UP_STAIR_UP");
  MyMenu.addList("LYING");
  MyMenu.addList("VEHICLE");
  MyMenu.showList();

  sdBuff = (struct sensorData*)malloc( sizeof(struct sensorData)*4096); //長さ4096を確保 ＊最後にfree(sdBuff)

  // メインループ
  while(!M5.BtnB.wasPressed()){

    // Cボタンでリスト送り。ついでにファイル名用乱数生成
    if(M5.BtnC.wasPressed()) {
      MyMenu.drawAppMenu(F("save acc and gyro"),F("OK"),F("EXIT"),F("NEXT"));
      MyMenu.nextList();
      // ファイル名用ランダム文字列の生成
      fileIndex = 0;
      rand_text(randStr);
    }

    // Aボタンを押すと計測スタート
    if(M5.BtnA.wasPressed()) {

      // 行動IDを取得
      for(int i=-1; i < MyMenu.getListID(); i++) {
        movementID = i + 1;
      }
      // 行動IDによってタイトルやTickerの時間を調整
      MyMenu.drawAppMenu(("SAVE "+String(movementList[movementID])),F(""),F("EXIT"),F("STOP"));
      if(movementID == 3 || movementID == 4) {
        measure_time = 5000; // 階段なら５秒計測
      } else {
        measure_time = 30000; // 階段以外なら30秒計測
      }

      // 計測開始
      M5.Lcd.drawCentreString("Waiting...", LCDcenterX, LCDcenterY2, 2);
      delay(5000);
      MyMenu.windowClr();
      // 16ミリ秒ごと(62.5Hz)にセンサーリード
      tickerSensor.attach_ms<MPU9250*>(16, _readSensor, &IMU);
      // 1秒ごとに経過時間を表示
      tickerShowTime.attach_ms(1000, _showElapsedTime);
      // 30秒ごとにフラグ（buffSaveFlg）を立てる
      tickerWriteData.attach_ms(measure_time, _buffSave);

      while(!M5.BtnC.wasPressed()) {
        M5.update();
        if(buffSaveFlg) {
          Serial.println("buffSaveFlg setted!");
          //タイマーを止める
          tickerSensor.detach();
          tickerShowTime.detach();
          tickerWriteData.detach();
          Serial.println("All ticker detached!");

          // ファイル名のフォーマットは "行動名_MACアドレス_乱数_通し番号(3桁0埋め).csv"
          char fileName[50] = {};
          // movementIDより、保存先パスの設定
          sprintf(fileName, "/acc/%s", movementList[movementID]);
          //ファイル作成
          Serial.println("making csv file...");
          sprintf(fileName, "/acc/%s/%s_%s_%s_%03d.csv", movementList[movementID], movementList[movementID], getMacAddr().c_str(), randStr, fileIndex);
          Serial.println("filename created completely.");
          fileIndex++;
          Serial.print("fileName = "); Serial.println(fileName);
          File file = SD.open(fileName, FILE_WRITE);
          Serial.print("Opened file = "); Serial.println(file);


          //Serial.println(fileName);
          //ファイルが開けないとき
          if(!file) {
              MyMenu.windowClr();
              M5.Lcd.drawCentreString("SD not found?Plz Insert SD and reboot", LCDcenterX, LCDcenterY2, 2);
              Serial.println("SD not found?Plz Insert SD and reboot");
              tickerSensor.detach();
              tickerShowTime.detach();
              tickerWriteData.detach();
              break;
          }
          Serial.println("opened csv file successfully!");
          //バッファを全て書き込み
          clearLCDY1();
          Serial.println("CSV writing...");
          M5.Lcd.drawCentreString("CSV Writing...", LCDcenterX, LCDcenterY1, 2);
          for(int i = 0; i < buffPointer; i++) {
            char buf[64];
            sprintf(buf, "%d, %d, %d, %d, %d, %d", sdBuff[i].accX, sdBuff[i].accY,sdBuff[i].accZ, sdBuff[i].gyroX, sdBuff[i].gyroY, sdBuff[i].gyroZ);
            file.println(buf);
          }

          file.close();
          Serial.println("File closed!");

          clearLCDY1();
          M5.Lcd.drawCentreString("Write Complete!", LCDcenterX, LCDcenterY3, 2);

          //バッファ初期化
          buffPointer = 0;
          buffSaveFlg = false;

          // 計測開始
          MyMenu.windowClr();
          tickerSensor.attach_ms<MPU9250*>(16,_readSensor, &IMU);
          tickerShowTime.attach_ms(1000, _showElapsedTime);
          tickerWriteData.attach_ms(measure_time, _buffSave);
          M5.Lcd.drawCentreString("Measurement Start!", LCDcenterX, LCDcenterY1, 2);
          Serial.println("All ticker attached!");
        }
        delay(100);
      }

      //タイマーを止める
      tickerSensor.detach();
      tickerShowTime.detach();
      tickerWriteData.detach();
      //バッファ初期化
      buffPointer = 0;
      elapsedTime = 0;
      buffSaveFlg = false;

      MyMenu.windowClr();

      // 加速度・ジャイロ保存終了
      MyMenu.drawAppMenu(F("Save acc and gyro"),F("OK"),F("EXIT"),F("NEXT"));
      MyMenu.showList();
    }
    M5.update();
  }
  free(sdBuff);
  MyMenu.show();
}


//ハンドラ－１（センサーを読んでバッファリング）
void _readSensor(MPU9250* IMU) {
  sensorData s;
  getAcc(IMU, &s);
  sdBuff[buffPointer++] = s;
}

//ハンドラ－２（SD保存のフラグを管理）
void _buffSave() {
  buffSaveFlg = true;
}

// ハンドラ3 (経過秒数の表示)
void _showElapsedTime() {
  elapsedTime++;
  clearLCDY2;
  M5.Lcd.drawCentreString((String(elapsedTime)+"seconds"), LCDcenterX, LCDcenterY2, 2);
}
