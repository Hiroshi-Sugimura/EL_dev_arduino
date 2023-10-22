// ESP32-S3-DevKitC-1
// https://akizukidenshi.com/catalog/g/gM-17073/
#include <Arduino.h>
#include <WiFi.h>
#include "EL.h"


#include <Adafruit_NeoPixel.h>

//--------------LED
#define DIN_PIN 48      // NeoPixel　の出力ピン番号
#define LED_COUNT 1     // LEDの連結数
#define WAIT_MS 1000    // 次の点灯までのウエイト
#define BRIGHTNESS 100  // 輝度（128まで行けるけど100で止める）
Adafruit_NeoPixel pixels(LED_COUNT, DIN_PIN, NEO_GRB + NEO_KHZ800);


//--------------WIFI
#define WIFI_SSID "ssid"  // !!!! change
#define WIFI_PASS "pass"  // !!!! change

WiFiUDP elUDP;
IPAddress myip;

//--------------EL
#define OBJ_NUM 1
byte eojs[OBJ_NUM][3] = { { 0x02, 0x90, 0x01 } };
EL echo(elUDP, eojs, OBJ_NUM);

void printNetData();


//====================================================================
// user用のcallback
// 受信したらこの関数が呼ばれるので、SetやGetに対して動けばよい、基本はSETだけ動けばよい
// 戻り値や引数は決まっている

// bool (*ELCallback) (   tid,  seoj,   deoj,   esv,  opc,  epc, pdc, edt);
bool callback(byte tid[], byte seoj[], byte deoj[], byte esv, byte opc, byte epc, byte pdc, byte edt[]) {
  bool ret = false;                                          // デフォルトで失敗としておく
  if (deoj[0] != 0x02 || deoj[1] != 0x90) { return false; }  // 照明ではないので無視
  if (deoj[2] != 0x00 && deoj[2] != 0x01) { return false; }  // インスタンスがないので無視

  // -----------------------------------
  // ESVがSETとかGETとかで動作をかえる、基本的にはSETのみ対応すればよい
  switch (esv) {
    // -----------------------------------
    // 動作状態の変更 Set対応
    case EL_SETI:
    case EL_SETC:
      switch (epc) {
        case 0x80:                                                                      // 電源
          if (edt[0] == 0x30) {                                                         // ON
            Serial.println("ON");                                                       // 設定した値にする
            pixels.clear();                                                             // クリア
            pixels.setPixelColor(0, pixels.Color(BRIGHTNESS, BRIGHTNESS, BRIGHTNESS));  // 白
            pixels.show();
            echo.devices[0][epc] = { 0x01, 0x30 };   // ON
            echo.devices[0][0xB6] = { 0x01, 0x42 };  // カラー灯モード（白）
            ret = true;                              // 処理できたので成功
          } else if (edt[0] == 0x31) {               // OFF
            Serial.println("OFF");
            pixels.clear();                                  // クリア
            pixels.setPixelColor(0, pixels.Color(0, 0, 0));  // 黒
            pixels.show();
            echo.devices[0][epc] = { 0x01, 0x31 };  // 設定した値にする
            ret = true;                             // 処理できたので成功
          }
          break;

        case 0xB0:                                                          // 照度レベル
          if (0 <= edt[0] && edt[0] <= 100) {                               // [0--100]
            Serial.printf("B0: %d\n", edt[0]);                              // 設定した値にする
            pixels.clear();                                                 // クリア
            pixels.setPixelColor(0, pixels.Color(edt[0], edt[0], edt[0]));  // 照度調整白
            pixels.show();
            echo.devices[0][epc] = { 0x01, edt[0] };
            ret = true;  // 処理できたので成功
          } else {
            ret = false;  // 処理で着ない範囲は失敗
          }
          break;

        case 0xC0:                                                                                                                 // 色設定
                                                                                                                                   // 本当はエラーチェックすべき
          Serial.printf("C0: %d, %d, %d\n", edt[0], edt[1], edt[2]);                                                               // 設定した値にする
          pixels.clear();                                                                                                          // クリア
          pixels.setPixelColor(0, pixels.Color(edt[0] * BRIGHTNESS / 255, edt[1] * BRIGHTNESS / 255, edt[2] * BRIGHTNESS / 255));  // 照度調整白
          pixels.show();
          echo.devices[0][epc] = { 0x03, edt[0], edt[1], edt[2] };  // 色設定した
          echo.devices[0][0xB6] = { 0x01, 0x45 };                   // カラー灯モード
          ret = true;                                               // 処理できたので成功
          break;
      }
      break;  // SETI, SETCここまで

    default:  // 基本はtrueを返却
      ret = true;
      break;
  }

  return ret;
}


//====================================================================
// main loop
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pixels.begin();  //NeoPixel制御開始

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  printNetData();  // to serial (debug)

  // print your WiFi IP address:
  myip = WiFi.localIP();

  echo.begin(callback);  // EL 起動シーケンス

  // 初期値設定
  echo.update(0, 0x80, { 0x01, 0x31 });             // off
  echo.devices[0][0xB0] = { 0x01, 100 };            // 照度
  echo.devices[0][0xC0] = { 0x03, 100, 100, 100 };  // 色設定
  echo.devices[0][0xB6] = { 0x01, 0x42 };           // カラー灯モード（白）

  echo.printAll();  // 全設定値の確認

  // 一般照明の状態，繋がった宣言として立ち上がったことをコントローラに知らせるINFを飛ばす
  const byte deoj[] = { 0x05, 0xff, 0x01 };
  const byte edt[] = { 0x01, 0x31 };
  echo.sendMultiOPC1(deoj, EL_INF, 0x80, edt);
}



//====================================================================
// main loop
void loop() {

  echo.recvProcess();

  delay(300);
}



//////////////////////////////////////////////////////////////////////
// debug用
//////////////////////////////////////////////////////////////////////
void printNetData() {
  Serial.println("-----------------------------------");

  // IP
  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP  Address: ");
  Serial.println(ip);

  IPAddress dgwip = WiFi.gatewayIP();
  Serial.print("DGW Address: ");
  Serial.println(dgwip);

  IPAddress smip = WiFi.subnetMask();
  Serial.print("SM  Address: ");
  Serial.println(smip);

  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("Arduino MAC: ");
  Serial.print(mac[5], HEX);
  Serial.print(":");
  Serial.print(mac[4], HEX);
  Serial.print(":");
  Serial.print(mac[3], HEX);
  Serial.print(":");
  Serial.print(mac[2], HEX);
  Serial.print(":");
  Serial.print(mac[1], HEX);
  Serial.print(":");
  Serial.println(mac[0], HEX);

  Serial.print("M5 MAC: ");
  Serial.println(WiFi.macAddress());

  Serial.print("M5 MAC(AP): ");
  Serial.println(WiFi.softAPmacAddress());

  Serial.println("-----------------------------------");
}
