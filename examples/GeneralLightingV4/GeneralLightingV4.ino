#include <M5Core2.h>
#include <WiFi.h>
#include "EL.h"

#define WIFI_SSID "ssid"  // !!!! change
#define WIFI_PASS "pass"  // !!!! change

WiFiUDP elUDP;
IPAddress myip;


#define OBJ_NUM 1
byte eojs[OBJ_NUM][3] = { { 0x02, 0x90, 0x01 } };
EL echo(elUDP, eojs, OBJ_NUM);

void printNetData();


//====================================================================
// user用のcallback
// 受信したらこの関数が呼ばれるので、SetやGetに対して動けばよい、基本はSETだけ動けばよい
// 戻り値や引数は決まっている

//      bool (*ELCallback) (   tid,  seoj,   deoj,   esv,  opc,  epc, pdcedt);
bool cb(byte[] tid, byte[] seoj, byte[] deoj, byte esv, byte opc, byte epc, PDCEDT pdcedt) {
  bool ret = false;                                          // デフォルトで失敗としておく
  if (deoj[0] != 0x02 || deoj[1] != 0x90) { return false; }  // 照明ではない
  if (deoj[2] != 0x00 || deoj[2] != 0x01) { return false; }  // インスタンスがない

  // -----------------------------------
  // ESVがSETとかGETとかで動作をかえる、基本的にはSETのみ対応すればよい
  switch (esv) {
    // -----------------------------------
    // 動作状態の変更 Set対応
    case EL_SETI:
    case EL_SETC:
      switch (epc) {
        case 0x80:               // 電源
          if (edt[0] == 0x30) {  // ON
            M5.Lcd.fillCircle(160, 120, 80, WHITE);
            echo.update(0, epc, { 0x30 });  // 設定した値にする
            ret = true;                     // 処理できたので成功
          } else if (edt[0] == 0x31) {      // OFF
            M5.Lcd.fillCircle(160, 120, 80, BLACK);
            echo.update(0, epc, { 0x31 });  // 設定した値にする
            ret = true;                     // 処理できたので成功
          }
          break;
      }
      break;  // SETI, SETCここまで
  }

  return ret;
}


//====================================================================
// main loop
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  M5.begin();
  M5.Lcd.setTextSize(2);
  M5.Lcd.println("wifi connect start");

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  M5.Lcd.println("wifi connect ok");
  printNetData();  // to serial (debug)

  // print your WiFi IP address:
  myip = WiFi.localIP();
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextSize(2);
  M5.Lcd.print("IP: ");
  M5.Lcd.println(myip);

  echo.begin(cb);  // EL 起動シーケンス

  // 一般照明の状態，繋がった宣言として立ち上がったことをコントローラに知らせるINFを飛ばす
  const byte deoj[] = { 0x05, 0xff, 0x01 };
  const byte edt[] = { 0x01, 0x31 };
  echo.sendMultiOPC1(deoj, EL_INF, 0x80, edt);
}



//====================================================================
// main loop
void loop() {
  M5.update();

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
