#ifndef ITLAPPS_H
#define ITLAPPS_H

#include <time.h>
#include <sys/time.h>
#include <string.h> // Cの文字列操作用

#include <M5StackSAM.h>
#include <M5Stack.h>
#include "EEPROM.h"

#include "utility/MPU9250.h"
#include "utility/quaternionFilters.h"

#include <WiFi.h>
#include <WiFiMulti.h>
#include <Wire.h>
#include <Ticker.h>

#include <MyPulseSensorPlayground.h>
#include <SparkFunMPU9250-DMP.h>

// LCDの中央座標を示す。Y座標は3列分用意。
#define LCDcenterX 160
#define LCDcenterY0 32
#define LCDcenterY1 52
#define LCDcenterY2 72
#define LCDcenterY3 92
#define LCDcenterY4 112
#define LCDcenterY5 132
#define LCDcenterY6 152
#define LCDcenterY7 172
#define LCDcenterY8 192

// Y1, Y2, Y3それぞれの行のみをクリアするメソッド
void clearLCDY0() { M5.Lcd.fillRect(0, LCDcenterY0, 320, 20, MyMenu.getrgb(128, 128, 128)); }
void clearLCDY1() { M5.Lcd.fillRect(0, LCDcenterY1, 320, 20, MyMenu.getrgb(128, 128, 128)); }
void clearLCDY2() { M5.Lcd.fillRect(0, LCDcenterY2, 320, 20, MyMenu.getrgb(128, 128, 128)); }
void clearLCDY3() { M5.Lcd.fillRect(0, LCDcenterY3, 320, 20, MyMenu.getrgb(128, 128, 128)); }
void clearLCDY4() { M5.Lcd.fillRect(0, LCDcenterY4, 320, 20, MyMenu.getrgb(128, 128, 128)); }
void clearLCDY5() { M5.Lcd.fillRect(0, LCDcenterY5, 320, 20, MyMenu.getrgb(128, 128, 128)); }
void clearLCDY6() { M5.Lcd.fillRect(0, LCDcenterY6, 320, 20, MyMenu.getrgb(128, 128, 128)); }
void clearLCDY7() { M5.Lcd.fillRect(0, LCDcenterY7, 320, 20, MyMenu.getrgb(128, 128, 128)); }
void clearLCDY8() { M5.Lcd.fillRect(0, LCDcenterY8, 320, 20, MyMenu.getrgb(128, 128, 128)); }

// M5.Lcd.drawCentreString()の単純化
void printLcdY0(String message) { M5.Lcd.drawCentreString(message, LCDcenterX, LCDcenterY0, 2); }
void printLcdY1(String message) { M5.Lcd.drawCentreString(message, LCDcenterX, LCDcenterY1, 2); }
void printLcdY2(String message) { M5.Lcd.drawCentreString(message, LCDcenterX, LCDcenterY2, 2); }
void printLcdY3(String message) { M5.Lcd.drawCentreString(message, LCDcenterX, LCDcenterY3, 2); }
void printLcdY4(String message) { M5.Lcd.drawCentreString(message, LCDcenterX, LCDcenterY4, 2); }
void printLcdY5(String message) { M5.Lcd.drawCentreString(message, LCDcenterX, LCDcenterY5, 2); }
void printLcdY6(String message) { M5.Lcd.drawCentreString(message, LCDcenterX, LCDcenterY6, 2); }
void printLcdY7(String message) { M5.Lcd.drawCentreString(message, LCDcenterX, LCDcenterY7, 2); }
void printLcdY8(String message) { M5.Lcd.drawCentreString(message, LCDcenterX, LCDcenterY8, 2); }


#endif