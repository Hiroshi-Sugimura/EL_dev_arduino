#include "M5Atom.h"
#include <WiFi.h>
#include "EL.h"

#define WIFI_SSID "YOUR_SSID"
#define WIFI_PASS "YOUR_PASS"

WiFiClient client;
WiFiUDP elUDP;
EL echo(elUDP, 0x01, 0x33, 0x01 );

uint8_t state = 0;

void printNetData();
void toggleSwicth();
void changeMode(int mode);

void setup()
{
    Serial.begin(115200);
    Serial.println("");

    pinMode(26, OUTPUT);
    digitalWrite(26, LOW);
    M5.begin(true, false, true);
    delay(10);

    M5.dis.drawpix(0, 0x00f000);

    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
      Serial.println("wait...");
      delay(1000);
    }
    printNetData();

    M5.dis.drawpix(0, 0x0000f0);
    echo.begin();            // EL 起動シーケンス

    echo.details[0x9D] = new byte[3] {0x02, 0x01, 0x80}; // change set property map
    echo.details[0x9E] = new byte[4] {0x03, 0x02, 0x80, 0xa0}; // change set property map
    echo.details[0x9f] = new byte[12]{0x0b, 0x0a, 0x80, 0x81, 0x82, 0x83, 0x88, 0x8a, 0x9d, 0x9e, 0x9f, 0xa0}; // get property map
    echo.details[0x80] = new byte[2] {0x01, 0x31}; // 音発生設定
    echo.details[0xA0] = new byte[2] {0x01, 0x31}; // 換気風量設定発生設定

    // 自ノードインスタンスリスト通知をINFで出す
    byte s[] = {	
		  0x10, 0x81, 0x00, 0x00, 0x0e, 0xf0, 0x01, 0x0e, 0xf0, 0x01, EL_INF, 0x01, 0xd5, 0x04, 0x01, 0x01, 0x33, 0x01
		};
  	echo.sendMulti(s, sizeof(s) / sizeof(s[0]));
}

uint8_t FSM = 0;

int packetSize = 0;     // 受信データ量
byte *pdcedt = nullptr; // テンポラリ

uint8_t pressed = 0;

void loop()
{
    if (M5.Btn.wasPressed())
    {
      if(state == 0){
        changeMode(-1);
      }
    }

    // パケット貰ったらやる
    packetSize = 0;
    pdcedt = nullptr;

    if (0 != (packetSize = echo.read()) ) // 0!=はなくてもよいが，Warning出るのでつけとく
    { // 受け取った内容読み取り，あったら中へ
        switch (echo._rBuffer[EL_ESV])
        {
            case EL_SETI:
            case EL_SETC:
                Serial.println("EL_SETI");
                switch (echo._rBuffer[EL_EPC])
                {
                    case 0x80: // 電源
                      if (echo._rBuffer[EL_EDT] == 0x30)
                      { // ON
                        // 動作ON処理
                        changeMode(1);
                        pdcedt = new byte[2]{0x01, 0x30}; 
                        echo.update(echo._rBuffer[EL_EPC], pdcedt); // ECHONET Liteの状態を変更 
                      }
                      else if (echo._rBuffer[EL_EDT] == 0x31)
                      {
                        changeMode(0);
                        pdcedt = new byte[2]{0x01, 0x31}; 
                        echo.update(echo._rBuffer[EL_EPC], pdcedt); // ECHONET Liteの状態を変更 
                      }
                      break;

                    case 0xA0: // 換気風量設定
                      break;

                    default:
                        break;
                }
                
                // pdcedtを使ったらクリア
                if (pdcedt != nullptr)
                {
                  delete[] pdcedt;
                  pdcedt = nullptr;
                }

                if (echo._rBuffer[EL_ESV] == EL_SETC)
                { // SETCなら返信必要
                  echo.returner();
                }
                break; // SETI, SETCここまで

            // -----------------------------------
            // Get,INF_REQ対応
            // SETの時にきちんとupdate関数でECHONET Liteの状態変更をライブラリに教えておけばここは簡素になる
            case EL_GET:
            case EL_INF_REQ:
                Serial.println("EL_GET/EL_INF_REQ");
                // update関数でdetailsに状態が登録されていれば自動で返信する
                echo.returner();
                break; // GetとINF_REQここま

            case EL_INF:
                Serial.println("EL_INF");
                break;

            default: // 解釈不可能なESV
                Serial.println(echo._rBuffer[EL_ESV]);
                break;
        }
    }

    delay(100);
    M5.update();
}

void changeMode(int mode){
  switch(mode){
    case 0: // OFF
      if(echo.details[0x80][1] == 0x31) return;

      if(echo.details[0xA0][1] == 0x31){
          toggleSwicth();
          toggleSwicth();
          toggleSwicth();
      } else if(echo.details[0xA0][1] == 0x32){
          toggleSwicth();
          toggleSwicth();
      } else if(echo.details[0xA0][1] == 0x33){
          toggleSwicth();
      }
      M5.dis.drawpix(0, 0x0000f0);
      break;

    case 1: // ON
      if(echo.details[0x80][1] == 0x30) return;

      if(echo.details[0xA0][1] == 0x31){
          toggleSwicth();
      } else if(echo.details[0xA0][1] == 0x32){
          toggleSwicth();
          toggleSwicth();
      } else if(echo.details[0xA0][1] == 0x33){
          toggleSwicth();
          toggleSwicth();
          toggleSwicth();
      }
      M5.dis.drawpix(0, 0xf00000);
      break;

    default:
      toggleSwicth();
      break;
  }
}

void toggleSwicth(){
  digitalWrite(26, HIGH);
  delay(100);
  digitalWrite(26, LOW);
  delay(100);
}

// debug
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

  Serial.println("-----------------------------------");
} 