#include "itlapps.h"


#include <drawPulse.h>

// const int PIN_INPUT = 36;
// const int THRESHOLD = 2200;

// PulseSensorPlayground pulseSensor;
// DrawPulse drawPulse;

// void appPlotHeartbeat() {
//   MyMenu.drawAppMenu(F("Heartbeat Sensor"), F(""), F("BACK"), F(""));

//     while(M5.BtnB.wasPressed()){
//       M5.update();
//     }

//     pulseSensor.analogInput(PIN_INPUT);
//     pulseSensor.setThreshold(THRESHOLD);

//     while (!pulseSensor.begin()) {
//         Serial.println("PulseSensor.begin: failed");
//         delay(500);
//     }
// }

const int HR_PIN = 36;
const int THRESHOLD = 2500;

#define REDRAW 20 // msec
#define PERIOD 60 // sec

int loopcount = 0;
int pointer = 0;
int ibis[256];

unsigned long pedLastStepCount = 0;

void appPlotHeartbeat(){
  MyMenu.drawAppMenu(F("heartbeat sensor"),F(""),F("BACK"),F(""));

  while(M5.BtnB.wasPressed()){
    M5.update();
  }

  // 心拍センサ初期化処理
  PulseSensorPlayground pulseSensor;
  pulseSensor.analogInput(HR_PIN);
  pulseSensor.setThreshold(THRESHOLD);
  DrawPulse drawPulse;

  drawPulse.init();

  if (!pulseSensor.begin()) {
        Serial.println("PulseSensor.begin: failed");
        M5.Lcd.drawCentreString("PulseSensor.bigin: failed", LCDcenterX, LCDcenterY2, 1);
  }


  // メインループ処理
  while(!M5.BtnB.wasPressed()){
    delay(REDRAW);
    if (pulseSensor.sawStartOfBeat()) {            // Constantly test to see if "a beat happened".
        int ibi = pulseSensor.getInterBeatIntervalMs();

        ibis[pointer] = ibi;
        pointer++;

        M5.Lcd.setCursor(0, 0);
        M5.Lcd.setTextSize(3);
        M5.Lcd.printf("BPM: %03d", 60000 /ibi);
    }

    int y = pulseSensor.getLatestSample();
    drawPulse.addValue(y);

    M5.update();
  }

  MyMenu.show();
}
