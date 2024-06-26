//////////////////////////////////////////////////////////////////////
/// @brief EL_dev_arduinoのサンプルプログラム
/// @author SUGIMURA Hiroshi
/// @date 2023.10.24
/// @details https://github.com/Hiroshi-Sugimura/EL_dev_arduino
/// ESP32-S3-DevKitC-1 https://akizukidenshi.com/catalog/g/gM-17073/
//////////////////////////////////////////////////////////////////////

#include <Arduino.h>
#include <WiFi.h>
#include "EL.h"

#include <Adafruit_NeoPixel.h>

// debug flag
// #define ___DEBUG___ 1

//--------------LED
#define DIN_PIN 48     // NeoPixel　の出力ピン番号
#define LED_COUNT 1    // LEDの連結数
#define WAIT_MS 1000   // 次の点灯までのウエイト
#define BRIGHTNESS 100 // 輝度（128まで行けるけど100で止める）
Adafruit_NeoPixel pixels(LED_COUNT, DIN_PIN, NEO_GRB + NEO_KHZ800);

//--------------WIFI
#define WIFI_SSID "ssid" // !!!! change
#define WIFI_PASS "pass" // !!!! change

WiFiUDP elUDP;
IPAddress myip;

//--------------EL
#define OBJ_NUM 1

// V4
EL echo(elUDP, {{0x02, 0x90, 0x01}});

void printNetData();

//====================================================================
// user用のcallback
// 受信したらこの関数が呼ばれるので、SetやGetに対して動けばよい、基本はSETだけ動けばよい
// 戻り値や引数は決まっている

// bool (*ELCallback) (   tid,  seoj,   deoj,   esv,  opc,  epc, pdc, edt);
bool callback(byte tid[], byte seoj[], byte deoj[], byte esv, byte opc, byte epc, byte pdc, byte edt[])
{
  bool ret = false; // デフォルトで失敗としておく
  if (deoj[0] != 0x02 || deoj[1] != 0x90)
  {
    return false;
  } // 照明ではないので無視
  if (deoj[2] != 0x00 && deoj[2] != 0x01)
  {
    return false;
  } // インスタンスがないので無視

  // -----------------------------------
  // ESVがSETとかGETとかで動作をかえる、基本的にはSETのみ対応すればよい
  switch (esv)
  {
  // -----------------------------------
  // 動作状態の変更 Set対応
  case EL_SETI:
  case EL_SETC:
    switch (epc)
    {
    case 0x80: // 電源
      if (edt[0] == 0x30)
      { // ON
#if ___DEBUG___
        Serial.println("ON"); // 設定した値にする
#endif
        pixels.clear();                                                            // クリア
        pixels.setPixelColor(0, pixels.Color(BRIGHTNESS, BRIGHTNESS, BRIGHTNESS)); // 白
        pixels.show();
        echo.update(0, epc, {0x30});  // ON
        echo.update(0, 0xB6, {0x42}); // カラー灯モード（白）
        ret = true;                   // 処理できたので成功
      }
      else if (edt[0] == 0x31)
      { // OFF
#if ___DEBUG___
        Serial.println("OFF");
#endif
        pixels.clear();                                 // クリア
        pixels.setPixelColor(0, pixels.Color(0, 0, 0)); // 黒
        pixels.show();
        echo.update(0, epc, {0x31}); // 設定した値にする
        ret = true;                  // 処理できたので成功
      }
      break;

    case 0x81:                       // 位置情報
      echo.update(0, epc, {edt[0]}); // ON
      break;

    case 0x88:                       // エラー情報
      echo.update(0, epc, {edt[0]}); // ON
      break;

    case 0xB0: // 照度レベル
      if (0 <= edt[0] && edt[0] <= 100)
      { // [0--100]
#if ___DEBUG___
        Serial.printf("B0: Lighting level: %d\n", edt[0]); // 設定した値にする
#endif
        pixels.clear();                                                // クリア
        pixels.setPixelColor(0, pixels.Color(edt[0], edt[0], edt[0])); // 照度調整白
        pixels.show();
        echo.update(0, 0x80, {0x30}); // ON
        echo.update(0, epc, {edt[0]});
        ret = true; // 処理できたので成功
      }
      else
      {
        ret = false; // 処理で着ない範囲は失敗
      }
      break;

    case 0xB6: // 点灯モード設定
#if ___DEBUG___
      Serial.printf("B6: Mode:", edt[0]);
#endif
      switch (edt[0])
      {
      case 0x41: // 自動
#if ___DEBUG___
        Serial.printf("auto\n");
#endif
        pixels.clear();                                                                              // クリア
        pixels.setPixelColor(0, pixels.Color(BRIGHTNESS * 0.8, BRIGHTNESS * 0.8, BRIGHTNESS * 0.8)); // 白（ちょっと暗い）
        pixels.show();
        echo.update(0, 0x80, {0x30}); // ON
        echo.update(0, epc, {edt[0]});
        ret = true; // 処理できたので成功
        break;

      case 0x42: // 通常灯
#if ___DEBUG___
        Serial.printf("nomal\n");
#endif
        pixels.clear();                                                            // クリア
        pixels.setPixelColor(0, pixels.Color(BRIGHTNESS, BRIGHTNESS, BRIGHTNESS)); // 白
        pixels.show();
        echo.update(0, 0x80, {0x30}); // ON
        echo.update(0, epc, {edt[0]});
        ret = true; // 処理できたので成功
        break;

      case 0x43: // 常夜灯
#if ___DEBUG___
        Serial.printf("night\n");
#endif
        pixels.clear();                                                // クリア
        pixels.setPixelColor(0, pixels.Color(BRIGHTNESS * 0.2, 0, 0)); // 暗いオレンジ
        pixels.show();
        echo.update(0, 0x80, {0x30}); // ON
        echo.update(0, epc, {edt[0]});
        ret = true; // 処理できたので成功
        break;

      case 0x45: // カラー灯
#if ___DEBUG___
        Serial.printf("color\n");
#endif
        pixels.clear();                                          // クリア
        pixels.setPixelColor(0, pixels.Color(0, 0, BRIGHTNESS)); // 青（カラー灯のデフォルトを青とする）
        pixels.show();
        echo.update(0, 0x80, {0x30}); // ON
        echo.update(0, epc, {edt[0]});
        ret = true; // 処理できたので成功
        break;

      default:
        ret = false;
      }
      break;

    case 0xC0: // 色設定、本当はエラーチェックすべき
#if ___DEBUG___
      Serial.printf("C0: color: %d, %d, %d\n", edt[0], edt[1], edt[2]); // 設定した値にする
#endif
      pixels.clear();                                                                                                         // クリア
      pixels.setPixelColor(0, pixels.Color(edt[0] * BRIGHTNESS / 255, edt[1] * BRIGHTNESS / 255, edt[2] * BRIGHTNESS / 255)); // BRIGHTNESSを最大として255段階
      pixels.show();
      echo.update(0, 0x80, {0x30});                  // ON
      echo.update(0, epc, {edt[0], edt[1], edt[2]}); // 色設定した
      echo.update(0, 0xB6, {0x45});                  // カラー灯モード
      ret = true;                                    // 処理できたので成功
      break;
    }
    break; // SETI, SETCここまで

  default: // それ以外はfalseを返却
    ret = false;
    break;
  }

  return ret;
}

//====================================================================
// setup
void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  pixels.begin(); // NeoPixel制御開始

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(1000);
  }
  printNetData(); // to serial (debug)

  // print your WiFi IP address:
  myip = WiFi.localIP();

  // 初期値設定
  echo.update(0, 0x80, {0x31});                                                                   // off
  echo.update(0, 0xB0, {BRIGHTNESS});                                                             // 照度
  echo.update(0, 0xC0, {BRIGHTNESS, BRIGHTNESS, BRIGHTNESS});                                     // 色設定
  echo.update(0, 0xB6, {0x42});                                                                   // カラー灯モード（白）
  echo.update(0, 0x9D, {0x80, 0xd6});                                                             // INFプロパティマップ
  echo.update(0, 0x9E, {0x80, 0xB0, 0xB6, 0xC0});                                                 // Setプロパティマップ
  echo.update(0, 0x9F, {0x80, 0x81, 0x82, 0x83, 0x88, 0x8a, 0xB0, 0xB6, 0xC0, 0x9d, 0x9e, 0x9f}); // Getプロパティマップ

  echo.printAll(); // 全設定値の確認

  echo.begin(callback); // EL 起動シーケンス

  // 一般照明の状態，繋がった宣言として立ち上がったことをコントローラに知らせるINFを飛ばす
  const byte deoj[] = {0x05, 0xff, 0x01};
  const byte edt[] = {0x01, 0x31};
  echo.sendMultiOPC1(deoj, EL_INF, 0x80, edt);
}

//====================================================================
// main loop
void loop()
{
  // ECHONET Lite通信の受信処理
  echo.recvProcess();

  delay(300);
}

//////////////////////////////////////////////////////////////////////
// debug用
//////////////////////////////////////////////////////////////////////
void printNetData()
{
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
//////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////
