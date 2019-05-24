// ThingsBoardを経由してサーバにデータを送る場合
const char* key = "";  // Access Token
const char* thingboardHost = "0.0.0.0";  // Server IP
const char* thingboardURL = "/api/v1/{key}/telemetry";
const int httpPort = 8080;

// ThingsBoardを経由せずに直接サーバにデータを送る場合
// const char* thingboardHost = "192.168.1.1";
// const char* thingboardURL = "/acc_post";
// const int httpPort = 5000;

// WiFiアクセスポイントの管理
int ssidnum = 4;
const char ssidlist[4][20] = {"SSID0", "SSID1", "SSID2", "SSID3"};
const char passlist[4][20] = {"Password0", "Password1", "Password2", "Password3"};
