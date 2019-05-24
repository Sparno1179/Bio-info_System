#include "itlapps.h"
#include "wificonfig.h"

#include <ArduinoJson.h>
#include <ThingsBoard.h>

// 加速度格納用構造体(String)
struct sensorDataString {
  String accX;
  String accY;
  String accZ;
  String gyroX;
  String gyroY;
  String gyroZ;
};

// プロトタイプ宣言
void calibrateIMU(MPU9250* IMU);
void getAccString(MPU9250 *IMU, sensorDataString *pSensorData);
void measureBPM(PulseSensorPlayground* pulseSensor, uint8_t* bpm);
void measureAcc(MPU9250* IMU, sensorDataString* pSensorData);
void initSensorVars(uint8_t* bpm, sensorDataString* acc, unsigned long* stepCount, unsigned long* stepTime, String* BSSIDs);
void measureSteps(MPU9250_DMP* IMU_DMP, unsigned long* stepCount, unsigned long* stepTime);

// ThingsBoardのセッション
WiFiClient espClient;
ThingsBoard tb(espClient);





// ThingsBoardに加速度ファイルを送る
void appSendAcc() {
  MyMenu.drawAppMenu(F("Send Acc to ThingsBoard"), F("OK"), F("BACK"), F(""));

  while (M5.BtnB.wasPressed())
    M5.update();

  // 加速度・ジャイロセンサの宣言・初期化
  printLcdY2("Calibrating IMU...");
  delay(500);
  MPU9250 IMU;
  struct sensorData sensorData = {0, 0, 0, 0, 0, 0};

  while (true) {
    delay(10);
    IMU.calibrateMPU9250(IMU.gyroBias, IMU.accelBias);
    delay(10);
    IMU.initMPU9250();
    delay(10);
    getAcc(&IMU, &sensorData);

    if (abs(sensorData.accX - sensorData.accY) < 70)
      break;

    delay(50);
  }

  MyMenu.windowClr();

  // WiFiに接続
  connectWiFi();

  // ThingsBoardへ接続
  if(keepTbConn()) {
    printLcdY2("connected");
  } else {
      printLcdY2("Failed to connect ThingsBoard");
  }

  // 計測開始前確認画面
  printLcdY2("Press A to start send.");
  while (!M5.BtnA.wasPressed())
    M5.update();

  MyMenu.windowClr();

  // メインループ
  while (!M5.BtnB.isPressed()) {

    struct sensorDataString acc = {"", "", "", "", "", ""};

    // Stringをバッファとして利用。
    // Stringは配列の動的な再確保を繰り返すので、reserveで初期サイズを指定しておく。
    acc.accX.reserve(256);
    acc.accY.reserve(256);
    acc.accZ.reserve(256);
    acc.gyroX.reserve(256);
    acc.gyroY.reserve(256);
    acc.gyroZ.reserve(256);

    printLcdY1("Measuring...");

    // 2秒分加速度を計測
    for (int i = 0; i < 125; i++) {
      getAccString(&IMU, &acc);
      delay(16);
    }

    MyMenu.windowClr();

    // JSONを作る。＊ThingsBoardは今のところネスト禁止
    // {"type":"acc", "x":"1,2,3,2,...", "y"...}
    DynamicJsonBuffer jsonBuffer(JSON_OBJECT_SIZE(7));
    JsonObject &root = jsonBuffer.createObject();
    root["type"] = "acc";
    root["x"] = acc.accX;
    root["y"] = acc.accY;
    root["z"] = acc.accZ;
    root["gx"] = acc.gyroX;
    root["gy"] = acc.gyroX;
    root["gz"] = acc.gyroX;
    Serial.println("send");

    char value[4096];
    root.printTo(value);
    tb.sendTelemetryJson(value);
    Serial.println(value);

    printLcdY1("Send Acc!");

    // 加速度送信後、５秒のインターバル
    keepTbConn();
    printLcdY2("Interval...");
    for (int i = 5; i > 0; i--) {
      printLcdY3((String(i) + "seconds left"));
      delay(1000);
    }

    keepTbConn();
    MyMenu.windowClr();

    M5.update();
  }

  MyMenu.show();
}






// BSSIDとその強度をJSON形式で送信
void appSendBSSID() {
  MyMenu.drawAppMenu(F("Send BSSID"),F(""),F(""),F(""));

  while(M5.BtnB.wasPressed()) M5.update();

  // 周囲のWiFiのBSSIDとRSSIを取得し、String型で受け取る
  String BSSIDlist = getBSSIDandRSSI(true);

  // WiFiに接続
  connectWiFi();
  // ThingsBoardへ接続
  if(keepTbConn()) {
    printLcdY2("connected");
    Serial.println("Connect to ThingsBoard Successfully");
  } else {
      printLcdY2("Failed to connect ThingsBoard");
  }


  // JSONを作る。
  // {"type":"bssid", "list":"BSSID1,RSSI1,BSSID2,RSSI2,..."}
  DynamicJsonBuffer jsonBuffer(JSON_OBJECT_SIZE(2));
  JsonObject &root = jsonBuffer.createObject();
  root["type"] = "bssid";
  root["list"] = BSSIDlist;
  Serial.println("send");

  char value[4096];
  root.printTo(value);
  tb.sendTelemetryJson(value);
  Serial.println(value);

  printLcdY1("Send BSSID!");


  while(!M5.BtnB.wasPressed()) { // メインループ

    M5.update();
  }

  MyMenu.show();
}







// 加速度・位置情報・心拍数をすべて送る
// WiFi切断 -> 5秒間心拍計測 -> 6秒間加速度計測 -> 10秒間歩数計測
//          -> BSSID取得 -> json作成 -> 送信 -> 2秒待つ -> 初めへ
void appSendAll() {
  MyMenu.drawAppMenu(F("SendAll"),F("OK"),F("EXIT"),F(""));

  while(M5.BtnB.wasPressed()) M5.update();

  // 各種変数を定義
  uint8_t bpm; // 心拍数(Beats Per Minute)
  struct sensorDataString acc = {"", "", "", "", "", ""}; // 加速度 {x, y, z, gx, gy, gz}
  String BSSIDs; // 周囲のAPのBSSIDとRSSI
  MPU9250 IMU; // 加速度センサ
  PulseSensorPlayground pulseSensor; // 心拍センサ
  MPU9250_DMP IMU_DMP; // 加速度センサのDMP(Digital Motion Processor)
  unsigned long stepCount = 0; // 歩数
  unsigned long stepTime = 0; // 上記の歩数を歩くのにかかった時間(ms)

  // 開始画面の表示
  printLcdY1("Send All Biological Information");
  printLcdY2("HeartBeatRate - Acceleration - AmbientWiFi");
  printLcdY3("- Steps");
  printLcdY4("Please put M5Stack on flat surface");
  printLcdY5("to calibrate Sensor");
  printLcdY6("Press OK to start calibrate sensor");
  delay(200);

  // 開始前確認画面
  while(!M5.BtnA.wasPressed()) M5.update();
  MyMenu.windowClr();


  // IMUの初期化処理
  printLcdY1("Calibrating Acceleration Sensor...");
  calibrateIMU(&IMU);
  clearLCDY1();
  printLcdY1("Calibrating Acceleration Sensor... OK!");

  // 心拍センサ初期化処理
  printLcdY2("Calibrating HeartBeat Sensor...");
  pulseSensor.analogInput(36);
  pulseSensor.setThreshold(2500);
  pulseSensor.begin();
  clearLCDY2();
  printLcdY2("Calibrating HeartBeat Sensor... OK!");

  // 歩数計初期化処理
  printLcdY3("Calibrating Pedometer Sensor...");
  IMU_DMP.begin();
  IMU_DMP.dmpBegin(DMP_FEATURE_PEDOMETER); // 歩数計モードで開始
  clearLCDY3();
  printLcdY3("Calibrating Pedometer Sensor... OK!");

  // 各種センサデータ格納用変数の初期化
  printLcdY5("Initializing Variables...");
  initSensorVars(&bpm, &acc, &stepCount, &stepTime, &BSSIDs);
  clearLCDY5();
  printLcdY5("Initializing Variables... OK!");

  printLcdY7("Measuring starts in 5 seconds...");
  delay(5000);

  while(!M5.BtnB.wasPressed()) { // メインループ
    MyMenu.windowClr();

    // WiFi切断
    WiFi.disconnect();
    Serial.println("Disconnected WiFi!");
    delay(100);

    // 心拍計測開始
    printLcdY1("Measuring BPM...");
    measureBPM(&pulseSensor, &bpm);
    printLcdY1("Measuring BPM... OK!");

    // 加速度計測開始
    printLcdY2("Measuring Acc...");
    measureAcc(&IMU, &acc);
    printLcdY2("Measuring Acceleration... OK!");

    // 歩数計測開始
    printLcdY3("Measuring Steps...");
    measureSteps(&IMU_DMP, &stepCount, &stepTime);
    printLcdY3("Measuring Steps... OK!");
    Serial.println("\n\n measure loop finished! \n\n");

    // 周囲のWiFiのBSSIDとRSSIを取得
    printLcdY4("Scanning Ambient Wifi...");
    BSSIDs = getBSSIDandRSSI(true);
    printLcdY4("Scanning Ambient Wifi... OK!");


    // ---------- 全計測終了、ThignsBoardへ送信 ----------
    printLcdY6("Creating JSON...");
    // 各種JSONファイルのバッファを生成
    DynamicJsonBuffer BPMjsonBuffer(JSON_OBJECT_SIZE(2));
    DynamicJsonBuffer AccjsonBuffer(JSON_OBJECT_SIZE(7));
    DynamicJsonBuffer StepsjsonBuffer(JSON_OBJECT_SIZE(2));
    DynamicJsonBuffer BSSIDjsonBuffer(JSON_OBJECT_SIZE(2));
    // 上記バッファからJSONオブジェクトを生成
    JsonObject &root_bpm = BPMjsonBuffer.createObject();
    JsonObject &root_acc = AccjsonBuffer.createObject();
    JsonObject &root_steps = StepsjsonBuffer.createObject();
    JsonObject &root_BSSID = BSSIDjsonBuffer.createObject();

    // JSONオブジェクトに計測したデータを代入する
    root_bpm["type"] = "bpm";
    root_bpm["bpm"] = bpm;

    root_acc["type"] = "acc";
    root_acc["x"] = acc.accX;
    root_acc["y"] = acc.accY;
    root_acc["z"] = acc.accZ;
    root_acc["gx"] = acc.gyroX;
    root_acc["gy"] = acc.gyroX;
    root_acc["gz"] = acc.gyroX;

    root_steps["type"] = "steps";
    root_steps["steps"] = stepCount;

    root_BSSID["type"] = "bssid";
    root_BSSID["bssid_list"] = BSSIDs;

    char value_bpm[256];
    char value_acc[4096];
    char value_steps[256];
    char value_BSSID[1024];

    root_bpm.printTo(value_bpm);
    root_acc.printTo(value_acc);
    root_steps.printTo(value_steps);
    root_BSSID.printTo(value_BSSID);

    // WiFiに接続
    Serial.println("Connecting WiFi...");
    connectWiFi();

    // ThingsBoardへ接続
    if(keepTbConn()) { Serial.println("Connect to ThingsBoard Successfully");
    } else           { Serial.println("Failed to connect ThingsBoard"); }

    // JSONをThingsBoardに送信
    tb.sendTelemetryJson(value_bpm);
    tb.sendTelemetryJson(value_acc);
    tb.sendTelemetryJson(value_steps);
    tb.sendTelemetryJson(value_BSSID);
    Serial.println("JSON telemetry sent!");


    // 次の計測に備え各種変数を初期化
    initSensorVars(&bpm, &acc, &stepCount, &stepTime, &BSSIDs);

    // ちょっと待つ
    delay(2000);

    M5.update();
  }

  MyMenu.show();
}






// IMUの初期化処理
// XとYの加速度の差が50以内にする
void calibrateIMU(MPU9250* IMU) {
  struct sensorData sensorData = {0, 0, 0, 0, 0, 0};
  while (true) {
    delay(10);
    IMU->calibrateMPU9250(IMU->gyroBias, IMU->accelBias);
    delay(10);
    IMU->initMPU9250();
    delay(10);
    getAcc(IMU, &sensorData);

    if (abs(sensorData.accX - sensorData.accY) < 50)
      break;

    delay(50);
  }
}






// 心拍数計測関数(6秒間)
// 心拍センサと計測した数値を代入する変数のポインタを引数とする
void measureBPM(PulseSensorPlayground* pulseSensor, uint8_t* bpm) {
  Serial.println("measure BPM Start!");
  for(int i=0; i<300; i++) {
    if (pulseSensor -> sawStartOfBeat()) {
      int ibi = pulseSensor -> getInterBeatIntervalMs();
      Serial.println("InterBeatInterval = " + String(ibi) + "ms");

      *bpm = uint8_t(60000 / ibi);
    }
    delay(20);
  }
  Serial.println("BPM = " + String(*bpm));
  Serial.println("measure BPM finished!");
}







// 加速度計測関数(2秒間)
void measureAcc(MPU9250* IMU, sensorDataString* pSensorData) {
  Serial.println("measure Acceleration Strat!");
  for(int i=0; i<125; i++) {
    getAccString(IMU, pSensorData);
    delay(16);
  }
  Serial.println("measure Acceleration finished!");
}






// 心拍数変数・加速度配列・歩数・BSSID変数のリセット
void initSensorVars(uint8_t* bpm, sensorDataString* acc, unsigned long* stepCount, unsigned long* stepTime, String* BSSIDs) {
  *bpm = 0;
  *stepCount = 0;
  *stepTime = 0;
  *acc = {"", "", "", "", "", ""};
  *BSSIDs = "";

  // Stringのバッファの確保
  acc -> accX.reserve(256);
  acc -> accY.reserve(256);
  acc -> accZ.reserve(256);
  acc -> gyroX.reserve(256);
  acc -> gyroY.reserve(256);
  acc -> gyroZ.reserve(256);

  Serial.println("Initialize Variables finished!");
}





// 歩数の計測関数(10秒間)
void measureSteps(MPU9250_DMP* IMU_DMP, unsigned long* stepCount, unsigned long* stepTime) {
  // DMPの歩数・時間をリセット
  IMU_DMP -> dmpSetPedometerSteps(*stepCount); // 歩数リセット
  IMU_DMP -> dmpSetPedometerTime(*stepTime); // 時間リセット
  Serial.println("reset Pedometer param finished!");

  // 計測開始
  for(int i=0; i<500; i++) {
  *stepCount = IMU_DMP -> dmpGetPedometerSteps();
  *stepTime = IMU_DMP -> dmpGetPedometerTime();
  if(i % 25 == 0) Serial.println(*stepCount);
  delay(20);
  }
  Serial.println(String(*stepCount) + "steps in 10 seconds");
  Serial.println("measure stepCount finished!");
}





// 加速度取得（String対応版）
void getAccString(MPU9250 *IMU, sensorDataString *pSensorData)
{
  // センサから各種情報を読み取り
  IMU->readAccelData(IMU->accelCount);
  IMU->getAres();
  IMU->readGyroData(IMU->gyroCount);
  IMU->getGres();

  // 取得した加速度に解像度をかけて、バイアス値を引く
  IMU->ay = (float)IMU->accelCount[1] * IMU->aRes - IMU->accelBias[1];
  IMU->az = (float)IMU->accelCount[2] * IMU->aRes - IMU->accelBias[2];
  IMU->ax = (float)IMU->accelCount[0] * IMU->aRes - IMU->accelBias[0];

  // 取得したジャイロに解像度をかける
  IMU->gx = (float)IMU->gyroCount[0] * IMU->gRes;
  IMU->gy = (float)IMU->gyroCount[1] * IMU->gRes;
  IMU->gz = (float)IMU->gyroCount[2] * IMU->gRes;

  // 四元数を更新する際に必ず呼び出し
  IMU->updateTime();

  // 加速度・ジャイロを与えられた構造体に代入
  pSensorData->accX += String((int)(1000 * IMU->ax)) + ",";
  pSensorData->accY += String((int)(1000 * IMU->ay)) + ",";
  pSensorData->accZ += String((int)(1000 * IMU->az)) + ",";
  pSensorData->gyroX += String((int)(IMU->gx)) + ",";
  pSensorData->gyroY += String((int)(IMU->gy)) + ",";
  pSensorData->gyroZ += String((int)(IMU->gz)) + ",";
}





// サーバーとのコネクション維持
bool keepTbConn() {
  if (!tb.connected()) {   // Connect to the ThingsBoard
    Serial.print("Connecting to ThingsBoard...");
    if (!tb.connect(thingboardHost, key)) {
      Serial.println("Failed to connect");
      return false;
    }
    Serial.print("Connect to ThingsBoard done");
  }
  return true;
}