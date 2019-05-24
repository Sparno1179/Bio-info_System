#include <M5Stack.h>
#include <WiFi.h>
#include <Wire.h>
#include "EEPROM.h"
#include <M5StackSAM.h>

M5SAM MyMenu;

#define EEPROM_SIZE 64
void dummy(){
}

void setup() {
  M5.begin();
  M5.lcd.setBrightness(90);
  Serial.begin(115200);
  Wire.begin();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  Serial.println("WiFi disabled!");
  // Speaker Off
  dacWrite(25, 0);

  if (!EEPROM.begin(EEPROM_SIZE))
  {
    Serial.println("Failed to initialise EEPROM.");
  }else{
    M5.lcd.setBrightness(byte(EEPROM.read(0)));
  }

  // CHANGING COLOR SCHEMA:
  //  MyMenu.setColorSchema(MENU_COLOR, WINDOW_COLOR, TEXT_COLO);
  //  COLORS are uint16_t (RGB565 format)
  // .MyMenu.getrgb(byte R, byte G, byte B); - CALCULATING RGB565 format

  //HERCULES MONITOR COLOR SCHEMA
  //MyMenu.setColorSchema(MyMenu.getrgb(0,0,0), MyMenu.getrgb(0,0,0), MyMenu.getrgb(0,255,0));

  // ADD MENU ITEM
  // MyMenu.addMenuItem(SUBMENU_ID,MENU_TITTLE,BTN_A_TITTLE,BTN_B_TITTLE,BTN_C_TITTLE,SELECTOR,FUNCTION_NAME);
  //    SUBMENU_ID byte [0-7]: TOP MENU = 0, SUBMENUs = [1-7]
  //    SELECTOR
  //           IF SELECTOR = -1 then MyMenu.execute() run function with name in last parameter (FUNCTION_NAME)
  //           IF SELECTOR = [0-7] then MyMenu.execute() switch menu items to SUBMENU_ID
  //    FUNCTION_NAME: name of function to run....


  MyMenu.addMenuItem(0, "Acc", "<", "OK", ">", 1, dummy);
  MyMenu.addMenuItem(0, "ThingsBoard", "<", "OK", ">", 2, dummy);
  MyMenu.addMenuItem(0, "SYSTEM", "<", "OK", ">", 3, dummy);
  MyMenu.addMenuItem(0, "SANDBOX", "<", "OK", ">", 4, dummy);

  MyMenu.addMenuItem(1, "Show Current AccGyro","<","OK",">", -1, appDrawAccGyro);
  MyMenu.addMenuItem(1, "Save Acc","<","OK",">", -1, appAccTimer);
  MyMenu.addMenuItem(1, "RETURN","<","OK",">", 0, dummy);

  MyMenu.addMenuItem(2, "SEND ALL DATA TO THINGSBOARD", "<", "OK", ">", -1, appSendAll);
  MyMenu.addMenuItem(2, "scan ambient wifi", "<", "OK", ">", -1, appScanWiFi);
  MyMenu.addMenuItem(2, "Show ambient BSSID", "<", "OK", ">", -1, appGetAmbientBSSID);
  MyMenu.addMenuItem(2, "Send ambient BSSID", "<", "OK", ">", -1, appSendBSSID);
  MyMenu.addMenuItem(2, "Send Acc", "<", "OK", ">", -1, appSendAcc);
  MyMenu.addMenuItem(2, "HeartBeat sensor", "<", "OK", ">", -1, appPlotHeartbeat);
  MyMenu.addMenuItem(2, "RETURN","<","OK",">", 0, dummy);

  MyMenu.addMenuItem(3, "CSV file count","<","OK",">", -1, appCsvFileCount);
  MyMenu.addMenuItem(3, "BRIGHTNESS", "<", "OK", ">", -1, appSetBrightness);
  MyMenu.addMenuItem(3, "SLEEP", "<", "OK", ">", -1, appSleep);
  MyMenu.addMenuItem(3, "RETURN","<","OK",">", 0, dummy);

  MyMenu.addMenuItem(4, "testFunctions", "<", "OK", ">", -1, appSandBox);
  MyMenu.addMenuItem(4, "LCD Yn", "<", "OK", ">", -1, appLCDYn);
  MyMenu.addMenuItem(4, "PIC", "<", "OK", ">", -1, appShowPic);
  MyMenu.addMenuItem(4, "RETURN","<","OK",">", 0, dummy);

  MyMenu.show();
}

void loop() {
  M5.update();
  if(M5.BtnC.wasPressed())MyMenu.up();
  if(M5.BtnA.wasPressed())MyMenu.down();
  if(M5.BtnB.wasPressed())MyMenu.execute();
}