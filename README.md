# Overview (EL\_dev\_arduino)

このモジュールはArduinoの**ECHONET Liteプロトコル**通信をサポートします．
(ECHONET Liteプロトコルはスマートハウス機器の通信プロトコルです．)
M5Stack Core2を中心にテストしています。
**本モジュールはECHONET Lite認証を受けたものではありませんので、商品化する際には開発者自らが認証を受ける必要があります。**


This module provides the communication method, **ECHONET Lite protocol**, for Arduino.
(ECHONET Lite protocol is a common communication specification for Smart home)
We are developping and debugging with M5stack Core2 mainly.


## Install

Arduino IDEのライブラリマネージャからモジュールをインストールできます．
sugimuraでフィルタするとすぐ出ます。

1. Arduino IDEを起動する
2. メニュー > スケッチ > ライブラリをインクルード > ライブラリを管理
3. sugimuraでフィルタ
4. EL_dev_arduinoをインストール

You can install the module by using Library Manager of ArduinoIDE as following.
It can be found to use filter text 'sugimura', author's name.

1. Start ArduinoIDE
2. Menu > Sketch > Include Library > Library Manager
3. Filter [sugimura]
4. Install EL_dev_arduino


# API Manual

https://hiroshi-sugimura.github.io/EL_dev_arduino/documents/html/index.html



# Example (General Lighting)

```
#include <M5Stack.h>
#include <WiFi.h>
#include "EL.h"

#define WIFI_SSID "change your wifi ssid"
#define WIFI_PASS "change your wifi password"

WiFiClient client;
WiFiUDP elUDP;
EL echo(elUDP, 0x02, 0x90, 0x01 );

void printNetData();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("");
  M5.begin();
  M5.Lcd.setTextSize(2);
  M5.Lcd.println("wifi connect start");

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("wait...");
    delay(1000);
  }
  M5.Lcd.println("wifi connect ok");
  M5.update();

  printNetData();
  echo.begin();            // EL 起動シーケンス

  // 一般照明の状態，繋がった宣言として立ち上がったことをコントローラに知らせるINFを飛ばす
  const byte deoj[] = {0x05, 0xff, 0x01};
  const byte edt[] = {0x01, 0x30};
  echo.sendMultiOPC1(deoj, EL_INF, 0x80, edt);

  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.setTextSize(3);
}


int packetSize = 0;     // 受信データ量
byte *pdcedt = nullptr; // テンポラリ



void loop() {
  M5.update();
  delay(200);

  // パケット貰ったらやる
  packetSize = 0;
  pdcedt = nullptr;

  if (0 != (packetSize = echo.read()) ) // 0!=はなくてもよいが，Warning出るのでつけとく
  { // 受け取った内容読み取り，あったら中へ

    // -----------------------------------
    // ESVがSETとかGETとかで動作をかえる
    switch (echo._rBuffer[EL_ESV])
    {

      // -----------------------------------
      // 動作状態の変更 Set対応
      case EL_SETI:
      case EL_SETC:
        switch (echo._rBuffer[EL_EPC])
        {
          case 0x80: // 電源
            if (echo._rBuffer[EL_EDT] == 0x30)
            { // ON
              M5.Lcd.fillCircle(160, 120, 80, WHITE);
              pdcedt = (byte[]){0x01, 0x30};       // ECHONET Liteの状態を変更（ライブラリに教えておく）
              echo.update(echo._rBuffer[EL_EPC], pdcedt); // ECHONET Liteの状態を変更
            }
            else if (echo._rBuffer[EL_EDT] == 0x31)
            { // OFF
              M5.Lcd.fillCircle(160, 120, 80, BLACK);
              pdcedt = (byte[]){0x01, 0x31};       // ECHONET Liteの状態を変更
              echo.update(echo._rBuffer[EL_EPC], pdcedt); // ECHONET Liteの状態を変更
            }
            break;

          default: // 不明なEPC
            M5.Lcd.print("??? packet esv, epc, edt is : ");
            // set
            // ESV, EPC, EDT
            M5.Lcd.print(echo._rBuffer[EL_ESV], HEX);
            M5.Lcd.print(" ");
            M5.Lcd.print(echo._rBuffer[EL_EPC], HEX);
            M5.Lcd.print(" ");
            M5.Lcd.println(echo._rBuffer[EL_EDT], HEX);
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
        // update関数でdetailsに状態が登録されていれば自動で返信する
        echo.returner();
        break; // GetとINF_REQここまで

      case EL_INF:
        break;

      default: // 解釈不可能なESV
        M5.Lcd.print("error? ESV = ");
        Serial.println(echo._rBuffer[EL_ESV]);
        break;
    }
  }
  // EL処理ここまで
  // -----------------------------------
  // パケットなかったとき、ふつうは何もしなくてよい
  // else {
  //}

  delay(500);
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
```


# Description for the example

## Constractor

ELクラスはWiFiのUDP（受信は3610，送信は適当）とECHONET Liteプロトコルをサポートします。ECHONET Liteネットワークにデバイスを参加させるためには専門のオブジェクトコードが必要です。オブジェクトコードはECHONET規格書に書かれているものを参照してください（https://echonet.jp/spec_g/#standard-01）。
サンプルとして一般照明（0x029001）をM5Stackで実装した例を示しています。

EL class manages WiFiUDP (recv is 3610 port, sned is any port) and ECHONET Lite protocol.
To join ECHONET Lite network, the device needs ECHONET Lite object code.
ECHONET Lite object code is defined in the specifications. (see https://echonet.jp/spec-en/#standard-01)
In this example, we deals with M5stack as a general lighting (0x029001).

- single object
```C++
EL echo(elUDP, 0x02, 0x90, 0x01 );
```

- multi object
```C++
byte devices[][3] = { {0x02, 0x90, 0x01}, {0x02, 0x90, 0x02}};
EL echo(elUDP, devices, 2 );
```


### Initialize and Start communication（初期化と通信開始）

begin()メソドを呼ぶと受信開始します。

To call 'begin()' method begins receiving ECHONET Lite protocol.

- void begin(void);

```
EL echo(elUDP, 0x02, 0x90, 0x01 );

void setup() {
  echo.begin();
}
```

### Receiver（受信）

- int read();

受信データを読む。割り込みやコールバックではなくポーリングで実装してください。

- IPAddress remoteIP(void);

受信データの送信元のIPアドレスを取得する。

- void returner(void);

update関数でdetailsに状態が登録されていれば自動で返信する。

- byte \_rBuffer[EL\_BUFFER\_SIZE]; // receive buffer

Received data.
受信したデータ




# Version

- 2.8.0 bugfix and, Add methods of class PDCEDT
- 2.7.0 Add methods of class ELOBJ
- 2.6.0 Add methods of class PDCEDT, ELOBJ
- 2.5.0 Add methods of class PDCEDT, ELOBJ
- 2.4.0 multi set epc
- 2.3.1 bug fix
- 2.3.0 bug fix, and multi get epc
- 2.2.1 bug fix
- 2.2.0 bug fix, support multi device
- 2.1.0 considering TID
- 2.0.0 change memory management, cope with memory leak.
- 1.3.0 deal with Node profile object, error case EPC, be searched and debug mode define.
- 1.2.2 details bug fix
- 1.2.1 EL Object code defines
- 1.2.0 Readme, keywords, library.json, library.properties
- 1.1.0 organized for m5stack
- 1.0.0 M5Stack
- 0.10.0 commit and publish


# License

MIT

## Author

神奈川工科大学  創造工学部  ホームエレクトロニクス開発学科

Dept. of Home Electronics, Faculty of Creative Engineering, Kanagawa Institute of Technology.


杉村　博

SUGIMURA Hiroshi
