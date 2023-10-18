#include <M5Core2.h>
#include <WiFi.h>
#include "EL.h"

#define WIFI_SSID "ssid"  // !!!! change
#define WIFI_PASS "pass"  // !!!! change

WiFiUDP elUDP;

EL echo(elUDP, 0x02, 0x90, 0x01);  // single
// byte eojs[1][3] = { { 0x02, 0x90, 0x01 } };  // single (multi)
// EL echo(elUDP, eojs, 1);

IPAddress myip;

void printNetData();

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

  echo.begin();  // EL 起動シーケンス

  // 一般照明の状態，繋がった宣言として立ち上がったことをコントローラに知らせるINFを飛ばす
  const byte deoj[] = { 0x05, 0xff, 0x01 };
  const byte edt[] = { 0x01, 0x31 };
  echo.sendMultiOPC1(deoj, EL_INF, 0x80, edt);
}


bool cb(byte[], byte[], byte[], byte, byte, byte, PDCEDT)
{
}




void light(byte esv, byte epc, byte pdc, byte* edt) {
  // -----------------------------------
  // ESVがSETとかGETとかで動作をかえる
  switch (esv) {
    // -----------------------------------
    // 動作状態の変更 Set対応
    case EL_SETI:
    case EL_SETC:
      switch (epc) {
        case 0x80:               // 電源
          if (edt[0] == 0x30) {  // ON
            M5.Lcd.fillCircle(160, 120, 80, WHITE);
            echo.devices[0][epc].setEDT({ 0x30 });  // ECHONET Liteの状態を変更
          } else if (edt[0] == 0x31) {              // OFF
            M5.Lcd.fillCircle(160, 120, 80, BLACK);
            echo.devices[0][epc].setEDT({ 0x31 });  // ECHONET Liteの状態を変更
          }
          break;

        default:  // 不明なEPC
          M5.Lcd.print("??? packet esv, epc, edt is : ");
          // set
          // ESV, EPC, EDT
          M5.Lcd.print(esv, HEX);
          M5.Lcd.print(" ");
          M5.Lcd.print(epc, HEX);
          M5.Lcd.print(" ");
          M5.Lcd.println(edt[0], HEX);
          break;
      }
      break;  // SETI, SETCここまで

    // -----------------------------------
    // Get,INF_REQ対応
    // SETの時にきちんとupdate関数でECHONET Liteの状態変更をライブラリに教えておけばここは簡素になる
    case EL_GET:
    case EL_INF_REQ:
      break;  // GetとINF_REQここまで

    case EL_INF:
      break;

    default:  // 解釈不可能なESV
      M5.Lcd.print("error? ESV = ");
      Serial.println(esv);
      break;
  }
}


void loop() {
  M5.update();

  echo.recvProcess();

  delay(300);
}


// debug
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
