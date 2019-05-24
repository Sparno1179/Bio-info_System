#include "itlapps.h"

// プロトタイプ宣言
int fileCount(fs::FS &fs, const char * dirname, uint8_t levels);


/**
 * @brief SD/accフォルダ内のファイルの数を数えて表示
 *
 */
void appCsvFileCount(){
  MyMenu.drawAppMenu(F("CSV File Count"),F(""),F("ESC"),F(""));

  while(M5.BtnB.wasPressed()){
    M5.update();
  }

  MyMenu.windowClr();
  printLcdY1(String(fileCount(SD, "/acc/walk", 0))    + " CSVs in WALK");
  printLcdY2(String(fileCount(SD, "/acc/stand", 0))   + " CSVs in STAND");
  printLcdY3(String(fileCount(SD, "/acc/sit", 0))     + " CSVs in SIT");
  printLcdY4(String(fileCount(SD, "/acc/stdown", 0))  + " CSVs in STAIR_DOWN");
  printLcdY5(String(fileCount(SD, "/acc/stup", 0))    + " CSVs in STAIR_UP");
  printLcdY6(String(fileCount(SD, "/acc/lying", 0))   + " CSVs in LYING");
  printLcdY7(String(fileCount(SD, "/acc/vehicle", 0)) + " CSVs in VEHICLE");

  while(!M5.BtnB.wasPressed()){
    M5.update();
  }
  MyMenu.show();
}


/**
 * @brief 指定されたフォルダのファイル数をカウントする。
 *
 * @param[in] fs 検索対象ファイルシステム。たいていはSD
 * @param[in] dirname フォルダパス名
 * @param[in] levels 探査階層。0の場合は子フォルダを無視する
 * @return ファイル数。フォルダはカウントされない
 * @retval -1 エラー。フォルダが開けなかったか、フォルダを指定しなかった。
 */
int fileCount(fs::FS &fs, const char * dirname, uint8_t levels) {

    int filecount = 0;

    File root = fs.open(dirname);

    if(!root) {
      Serial.println("no such directory");
      return -1; // フォルダを開けなかった
    }
    if(!root.isDirectory()) {
      Serial.println("not directory");
      return -1;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            if(levels) {
              filecount += fileCount(fs, file.name(), levels -1);
            }
        } else {
              filecount++;
            }
        file = root.openNextFile();
    }
    return filecount;
}