////////////////////////////////////////////////////////////////////////////////
// Arduino ECHONET Lite Sample
// Copyright (C) Hiroshi SUGIMURA 2013.09.27
////////////////////////////////////////////////////////////////////////////////
#include <SPI.h>
#include <Wire.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include "LCD.h" // AQM0802A-RN-GBW
#include "ELOBJ.h"
#include "EL.h"

EL echo(0x02, 0x90, 0x01); // 一般照明 0290 01
LCD lcd;                   // IPアドレス表示用

// macアドレスは商品別で設定する必要がある。
byte mac[] = {
    0x90, 0xa2, 0xda, 0x0f, 0x92, 0xec};

// 照明用設定
// int lightPin = 13;   // 一般的には13 pinにLEDがついているが、
int lightPin = 9; // Netduinoなど特殊な製品は 9 pinの場合がある。

////////////////////////////////////////////////////////////////////////////////
// 初期化
void setup()
{
  lcd.begin();
  pinMode(lightPin, OUTPUT); // 出力制御

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  Serial.println("DHCP...");
  lcd.setCursor(0, 0);
  lcd.printStr("DHCP...");

  // start the Ethernet connection:
  while (1)
  {
    if (Ethernet.begin(mac) == 0)
    {
      Serial.println("Failed to configure Ethernet using DHCP");
    }
    else
    {
      Serial.println("DHCP ok!");
      break;
    }
  }

  echo.printNetData(&lcd); // 現在の状態表示
  echo.begin();            // EL 起動シーケンス

  // 一般照明の状態，繋がった宣言として立ち上がったことをコントローラに知らせるINFを飛ばす
  const byte deoj[] = {0x05, 0xff, 0x01};
  const byte edt[] = {0x01, 0x30};
  echo.sendMultiOPC1(deoj, INF, 0x80, edt);

  // 機器状態
  digitalWrite(lightPin, HIGH);
}

int packetSize = 0;     // 受信データ量
byte *pdcedt = nullptr; // テンポラリ

////////////////////////////////////////////////////////////////////////////////
// main
void loop()
{
  // パケット貰ったらやる
  packetSize = 0;
  pdcedt = nullptr;

  if (packetSize = echo.read())
  { // 受け取った内容読み取り，あったら中へ

    // -----------------------------------
    // ESVがSETとかGETとかで動作をかえる
    switch (echo._rBuffer[ESV])
    {

    // -----------------------------------
    // 動作状態の変更 Set対応
    case SETI:
    case SETC:
      switch (echo._rBuffer[EPC])
      {
      case 0x80: // 電源
        if (echo._rBuffer[EDT] == 0x30)
        { // ON
          digitalWrite(lightPin, HIGH);
          pdcedt = new byte[2]{0x01, 0x30};        // ECHONET Liteの状態を変更（ライブラリに教えておく）
          echo.update(echo._rBuffer[EPC], pdcedt); // ECHONET Liteの状態を変更
        }
        else if (echo._rBuffer[EDT] == 0x31)
        { // OFF
          digitalWrite(lightPin, LOW);
          pdcedt = new byte[2]{0x01, 0x31};        // ECHONET Liteの状態を変更
          echo.update(echo._rBuffer[EPC], pdcedt); // ECHONET Liteの状態を変更
        }
        break;

      default: // 不明なEPC
        Serial.print("??? packet esv, epc, edt is : ");
        // set
        // ESV, EPC, EDT
        Serial.print(echo._rBuffer[ESV], HEX);
        Serial.print(" ");
        Serial.print(echo._rBuffer[EPC], HEX);
        Serial.print(" ");
        Serial.println(echo._rBuffer[EDT], HEX);
        break;
      }

      // pdcedtを使ったらクリア
      if (pdcedt != nullptr)
      {
        delete[] pdcedt;
        pdcedt = nullptr;
      }

      if (echo._rBuffer[ESV] == SETC)
      { // SETCなら返信必要
        echo.returner();
      }
      break; // SETI, SETCここまで

    // -----------------------------------
    // Get,INF_REQ対応
    // SETの時にきちんとupdate関数でECHONET Liteの状態変更をライブラリに教えておけばここは簡素になる
    case GET:
    case INF_REQ:
      // update関数でdetailsに状態が登録されていれば自動で返信する
      echo.returner();
      break; // GetとINF_REQここまで

    case INF:
      break;

    default: // 解釈不可能なESV
      Serial.print("error? ESV = ");
      Serial.println(echo._rBuffer[ESV]);
      break;
    }
  }
  // EL処理ここまで
  // -----------------------------------
  // パケットなかったとき、ふつうは何もしなくてよい
  // else {
  //}

  delay(200);
}
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
