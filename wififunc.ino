#include "itlapps.h"





// 周囲のWiFIをスキャンし、SSID,BSSID,RSSIを表示
void appScanWiFi() {
    MyMenu.drawAppMenu(F("Scan Ambient WiFi"),F(""),F("BACK"),F(""));

    while(M5.BtnB.wasPressed()) {
        M5.update();
    }

    // WiFiをステーションモードに切り替える
    WiFi.mode(WIFI_MODE_STA);
    // 既にAPに接続済みの場合、切断
    WiFi.disconnect();
    delay(100);


    // WiFi.scanNetworks()で周囲のWiFiをスキャン
    // wifiNumにはスキャンしたWiFiの数を格納
    printLcdY4("Scanning...");
    int wifiNum = WiFi.scanNetworks();

    MyMenu.windowClr();

    for (int i = 0; i < wifiNum; i++) {
        M5.Lcd.drawCentreString((String(WiFi.SSID(i))+","+WiFi.BSSIDstr(i)+","+String(WiFi.RSSI(i))+"dB"),
                                 LCDcenterX, LCDcenterY0 + i * 20, 2);
    }

    // メインループ
    while(!M5.BtnB.wasPressed()) {
        M5.update();
    }

  MyMenu.show();
}





// 周囲のWiFiのBSSIDと強度を取得して表示
void appGetAmbientBSSID() {
  MyMenu.drawAppMenu(F("Ambient BSSID"),F(""),F(""),F(""));

  while(M5.BtnB.wasPressed()) M5.update();

    // WiFiをステーションモードに切り替える
    WiFi.mode(WIFI_MODE_STA);
    // 既にAPに接続済みの場合、切断
    WiFi.disconnect();
    delay(100);


    // WiFi.scanNetworks()で周囲のWiFiをスキャン
    // wifiNumにはスキャンしたWiFiの数を格納
    printLcdY4("Scanning...");
    int wifiNum = WiFi.scanNetworks();

    // スキャン終了後、表示
    MyMenu.windowClr();
    for (int i = 0; i < wifiNum; i++) {
      M5.Lcd.drawCentreString((WiFi.BSSIDstr(i) + " , " +  String(WiFi.RSSI(i)) + "dB"),
                               LCDcenterX, LCDcenterY0 + i * 20, 2);
    }

  while(!M5.BtnB.wasPressed()) { // メインループ
    M5.update();
  }

  MyMenu.show();
}






// 周囲のWiFiのBSSIDと強度を取得してString形式で返す
// "BSSID1,RSSI1,BSSID2,RSSI2,BSSID3,RSSI3,..."
// 位置即位に使えないモバイルWiFi系列をはじく場合はTrue引数
String getBSSIDandRSSI(bool avoidMobileWiFi) {

  Serial.println("String getBSSIDandRSSI start");

  // WiFiをステーションモードに切り替える
    WiFi.mode(WIFI_MODE_STA);
    // 既にAPに接続済みの場合、切断
    WiFi.disconnect();
    delay(100);


    // WiFi.scanNetworks()で周囲のWiFiをスキャン
    // wifiNumにはスキャンしたWiFiの数を格納
    Serial.println("Scanning ambient AP");
    int wifiNum = WiFi.scanNetworks();

    // 戻り値用String配列
    String list = "";

    for(int i=0; i < wifiNum; i++) {
      // 配列にBSSIDとRSSIをつなげていく。
      // BSSID1,RSSI1,BSSID2,RSSI2...という形。

      // aboidMobileWiFiがTrueかつSSIDがモバイルWiFi特有のものであればそのAPを無視
      if (avoidMobileWiFi && isAvoidSSID(WiFi.SSID(i))) {
        // 何もしない
        Serial.println("Avoided SSID: " + String(WiFi.SSID(i)));
      } else { // ブラックリストにないAPだった場合はBSSIDを追加
      list += WiFi.BSSIDstr(i); list += ",";
      list += String(WiFi.RSSI(i)); list += (i == wifiNum-1 ? "" : ",");
      Serial.println("Added SSID: " + String(WiFi.SSID(i)));
      }
    }

    Serial.println(list);

  return list;
}





// 位置即位には使えないWiFiのSSIDたち
const int blacklistNum = 6;
char blacklist[blacklistNum][8] = {
    // WiMAX
    "SPWN",
    "W0",
    "wx0",
    // ワイモバイル
    "HWa",
    // b-mobile他
    "mobile",
    // ～のiPhone他
    "Phone"
};


bool isAvoidSSID(const String &ssid) {
  // ITLモバイルルータは無視
  if(ssid.endsWith("_nomap")) return true;

  for(int i=0; i<blacklistNum; i++) {
    // 上記のブラックリスト内のSSIDを発見したらTrueを返す
    if(ssid.indexOf(blacklist[i]) != -1) return true;
  }

  // ブラックリスト内のいずれとも合致しないSSIDだったとき
  return false;
}

// WiFiに接続
void connectWiFi() {
  WiFiMulti wifiMulti;

  // wificonfig.hの中で定義したAPたちを追加
  for (int i = 0; i < ssidnum; i++)
    wifiMulti.addAP(ssidlist[i], passlist[i]);

  if (wifiMulti.run() == WL_CONNECTED) { // 追加したAPの中から自動的に選択して接続
    MyMenu.windowClr();
    Serial.println("WiFi connected!");
    Serial.println("SSID -> " + String(WiFi.SSID()));
    Serial.println("RSSI -> " + String(WiFi.RSSI()) + "dB");
    Serial.print("IPv4 -> "); Serial.println(WiFi.localIP());
  }
}