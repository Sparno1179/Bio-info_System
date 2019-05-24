#include "itlapps.h"

// 本編に関係のないテスト用などのアプリたち

/**
 * @fn nowTime
 * @brief 現在時刻を取得し、返す
 *
 * @return    "YYYY-MM-DD_HH-MM-SS"の形の時刻情報
 * @attention 時刻情報を返すのはいいが、M5Stackは起動時に時刻が1970/1/1でスタートするので
 *            ほとんどこのメソッドの意味はないかもしれない。
 * @note      return文はもっときれいな書き方があるはず。
 */
String nowTime()
{
  struct tm tm;
  time_t t = time(NULL);
  localtime_r(&t, &tm);

  return String(tm.tm_year + 1900) + "-" +
         String(tm.tm_mon + 1) + "-" +
         String(tm.tm_mday) + "_" +
         String(tm.tm_hour) + "-" +
         String(tm.tm_min) + "-" +
         String(tm.tm_sec);
}

void appShowPic()
{
  MyMenu.drawAppMenu(F("SEVASTIAN"), F(""), F("BACK"), F(""));

  while (M5.BtnB.wasPressed())
  {
    M5.update();
  }

  while (!M5.BtnB.wasPressed())
  {
    M5.Lcd.drawJpgFile(SD, "/jpg/sev.jpg", 50, 35);
    M5.update();
  }

  MyMenu.show();
}

// いろいろと試してみたいことをテストするメソッド。本機能には絡みません。
void appSandBox()
{
  MyMenu.drawAppMenu(F("SANDBOX"), F("A"), F("B"), F("C"));
  int a = 0, b = 0, c = 0;
  char stringA[10] = "";
  char stringB[11];
  char stringC[128] = "";

  M5.Lcd.drawCentreString("Press A to draw ambientBSSID", LCDcenterX, LCDcenterY1, 2);
  M5.Lcd.drawCentreString("Press B to EXIT", LCDcenterX, LCDcenterY2, 2);
  M5.Lcd.drawCentreString("Press C to show randStr", LCDcenterX, LCDcenterY3, 2);

  while (M5.BtnB.wasPressed())
  {
    M5.update();
  }

  while (!M5.BtnB.wasPressed())
  {

    if (M5.BtnA.wasPressed())
    {
      printLcdY1(getBSSIDandRSSI(false));
    }

    if (M5.BtnB.wasPressed())
    {
      b++;
      sprintf(stringB, "b = %d", b);
      M5.Lcd.fillRect(0, LCDcenterY2, 320, 20, MyMenu.getrgb(128, 128, 128));
      M5.Lcd.drawCentreString(stringB, LCDcenterX, LCDcenterY2, 2);
    }

    if (M5.BtnC.wasPressed())
    {
      c++;
      sprintf(stringC, "c = %d", c);
      M5.Lcd.fillRect(0, LCDcenterY3, 320, 20, MyMenu.getrgb(128, 128, 128));
      rand_text(stringA);
      M5.Lcd.drawCentreString(stringA, LCDcenterX, LCDcenterY3, 2);
    }

    M5.update();
  }

  MyMenu.show();
}













void appLCDYn() {
  MyMenu.drawAppMenu(F("LCD Y"),F(""),F("BACK"),F(""));

  while(M5.BtnB.wasPressed()) {
    M5.update();
  }

    M5.Lcd.drawCentreString("LCDcenterY0", LCDcenterX, LCDcenterY0, 2);
    M5.Lcd.drawCentreString("LCDcenterY1", LCDcenterX, LCDcenterY1, 2);
    M5.Lcd.drawCentreString("LCDcenterY2", LCDcenterX, LCDcenterY2, 2);
    M5.Lcd.drawCentreString("LCDcenterY3", LCDcenterX, LCDcenterY3, 2);
    M5.Lcd.drawCentreString("LCDcenterY4", LCDcenterX, LCDcenterY4, 2);
    M5.Lcd.drawCentreString("LCDcenterY5", LCDcenterX, LCDcenterY5, 2);
    M5.Lcd.drawCentreString("LCDcenterY6", LCDcenterX, LCDcenterY6, 2);
    M5.Lcd.drawCentreString("LCDcenterY7", LCDcenterX, LCDcenterY7, 2);
    M5.Lcd.drawCentreString("LCDcenterY8", LCDcenterX, LCDcenterY8, 2);

  // メインループ
  while(!M5.BtnB.wasPressed()) {
    M5.update();
  }

  MyMenu.show();
}

// void appTEMPLATE() {
//   MyMenu.drawAppMenu(F("APP_TITTLE"),F("BTN_A_TITTLE"),F("BTN_B_TITTLE"),F("BTN_C_TITTLE"));

//   while(M5.BtnB.wasPressed()) {
//     M5.update();
//   }

//   while(!M5.BtnB.wasPressed()) {
//     M5.update();
//   }

//   MyMenu.show();