# Overview (EL\_dev\_arduino)

このモジュールはArduinoの**ECHONET Liteプロトコル**をサポートします．
ECHONET Liteプロトコルはスマートハウス機器の通信プロトコルです．
M5Stackでテストしています。


This module provides **ECHONET Lite protocol** for Arduino.
The ECHONET Lite protocol is a communication protocol for smart home devices.
It is comfirmed with M5stack.


## Install

ArduinoIDEのライブラリマネージャからモジュールをインストールできます．
ECHONETでフィルタしてください。

```
1. ArduinoIDEを起動する
2. メニューのスケッチ
3. ライブラリをインクルード
4. ライブラリを管理
5. ECHONETでフィルタ
6. EL_dev_arduinoをインストール
```

You can install the module by using Library Manager of ArduinoIDE as following.

```
1. Start ArduinoIDE
2. Menu > Sketch
3. Include Library
4. Library Manager
5. Filter [ECHONET]
6. Install EL_dev_arduino
```


# How to use (for Mac)

1. Install the Arduino
2. Download or clone this (https://github.com/Hiroshi-Sugimura/EL_dev_arduino)
3. put EL_dev_arduino into ~/Documents/Arduino/libraries
4. Open example GeneralLighting by arduino IDE
5. Attach the M5stack
6. Run


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


# Classes

## EL (for user)

ELクラスはWiFiのUDP（受信は3610，送信は適当）とECHONET Liteプロトコルをサポートします。ECHONET Liteネットワークにデバイスを参加させるためには専門のオブジェクトコードが必要です。オブジェクトコードはECHONET規格書に書かれているものを参照してください（https://echonet.jp/spec_g/#standard-01）。サンプルとして一般照明（0x029001）をM5Stackで実装した例を示しています。

EL class manages WiFiUDP (recv is 3610 port, sned is any port) and ECHONET Lite protocol.
To join ECHONET Lite network, the device needs ECHONET Lite object code.
ECHONET Lite object code is defined in the specifications. (see https://echonet.jp/spec-en/#standard-01)
In this example, we deals with M5stack as a general lighting (0x029001).


```C++
EL echo(elUDP, 0x02, 0x90, 0x01 );
```

## ELOBJ (inner class)



## APIs of EL class



### Initialize（初期化）

- EL( WiFiUDP& udp, byte eoj0, byte eoj1, byte eoj2);

constructor

- void begin(void);

beggining ECHONET Lite protocol.

```
EL echo(elUDP, 0x02, 0x90, 0x01 );

void setup() {
  echo.begin();
}

```

### Sender（送信）

- void send(IPAddress toip, byte sBuffer[], int size);
- void sendOPC1(const IPAddress toip, const byte *deoj, const byte esv, const byte epc, const byte *edt);
- void sendOPC1(const IPAddress toip, const byte *tid, const byte *seoj, const byte *deoj, const byte esv, const byte epc, const byte *edt);
- void sendBroad(byte sBuffer[], int size);
- void sendMulti(byte sBuffer[], int size);
- void sendMultiOPC1(const byte *deoj, const byte esv, const byte epc, const byte *edt);
- void sendMultiOPC1(const byte *tid, const byte *seoj, const byte *deoj, const byte esv, const byte epc, const byte *edt);


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


```
void loop() {
  if (0 != echo.read()) )
  {
    switch (echo._rBuffer[EL_ESV])
    {
      // -----------------------------------
      // 動作状態の変更 Set対応
      case EL_SETI:
      case EL_SETC:
        break; // SETI, SETCここまで

      // -----------------------------------
      // Get,INF_REQ
      case EL_GET:
      case EL_INF_REQ:
        echo.returner();
        break;

      case EL_INF:
        break;
      default: // 解釈不可能なESV
    }
  }
}
```

### Update EDT（EDT:機器データの更新）

- void update(const byte epc, byte pdcedt[]);


### Confirm EDT（機器データの確認）

- byte *at(const byte epc);


### Device Data（機器データ）

- ELOBJ profile; // node profile object (PDC and EDT list)
- ELOBJ details; // device object (PDC and EDT list)


# Defines

```C++:EL.h
#define EL_PORT		3610
#define EL_EHD1		0
#define EL_EHD2		1
#define EL_TID		2
#define EL_SEOJ		4
#define EL_DEOJ		7
#define EL_ESV		10
#define EL_OPC		11
#define EL_EPC		12
#define EL_PDC		13
#define EL_EDT		14
#define EL_SETI_SNA	0x50
#define EL_SETC_SNA	0x51
#define EL_GET_SNA	0x52
#define EL_INF_SNA	0x53
#define EL_SETGET_SNA 0x5e
#define EL_SETI		0x60
#define EL_SETC		0x61
#define EL_GET		0x62
#define EL_INF_REQ	0x63
#define EL_SETGET	0x6e
#define EL_SET_RES	0x71
#define EL_GET_RES	0x72
#define EL_INF		0x73
#define EL_INFC		0x74
#define EL_INFC_RES	0x7a
#define EL_SETGET_RES 0x7e
#define EL_BUFFER_SIZE 256


// Device Object
// センサ関連機器
#define	EL_GasLeakSensor	0x00, 0x01	//	ガス漏れセンサ
#define	EL_CrimePreventionSensor	0x00, 0x02	//	防犯センサ
#define	EL_EmergencyButton	0x00, 0x03	//	非常ボタン
#define	EL_FirstAidSensor	0x00, 0x04	//	救急用センサ
#define	EL_EarthquakeSensor	0x00, 0x05	//	地震センサ
#define	EL_ElectricLeakSensor	0x00, 0x06	//	漏電センサ
#define	EL_HumanDetectionSensor	0x00, 0x07	//	人体検知センサ
#define	EL_VisitorSensor	0x00, 0x08	//	来客センサ
#define	EL_CallSensor	0x00, 0x09	//	呼び出しセンサ
#define	EL_CondensationSensor	0x00, 0x0A	//	結露センサ
#define	EL_AirPollutionSensor	0x00, 0x0B	//	空気汚染センサ
#define	EL_OxygenSensor	0x00, 0x0C	//	酸素センサ
#define	EL_IlluminanceSensor	0x00, 0x0D	//	照度センサ
#define	EL_SoundSensor	0x00, 0x0E	//	音センサ
#define	EL_MailingSensor	0x00, 0x0F	//	投函センサ
#define	EL_WeightSensor	0x00, 0x10	//	重荷センサ
#define	EL_TemperatureSensor	0x00, 0x11	//	温度センサ
#define	EL_HumiditySensor	0x00, 0x12	//	湿度センサ
#define	EL_RainSensor	0x00, 0x13	//	雨センサ
#define	EL_WaterLevelSensor	0x00, 0x14	//	水位センサ
#define	EL_BathWaterLevelSensor	0x00, 0x15	//	風呂水位センサ
#define	EL_BathHeatingStatusSensor	0x00, 0x16	//	風呂沸き上がりセンサ
#define	EL_WaterLeakSensor	0x00, 0x17	//	水漏れセンサ
#define	EL_WaterOverflowSensor	0x00, 0x18	//	水あふれセンサ
#define	EL_FireSensor	0x00, 0x19	//	火災センサ
#define	EL_CigaretteSmokeSensor	0x00, 0x1A	//	タバコ煙センサ
#define	EL_CO2Sensor	0x00, 0x1B	//	CO2センサ
#define	EL_GasSensor	0x00, 0x1C	//	ガスセンサ
#define	EL_VOCSensor	0x00, 0x1D	//	VOCセンサ
#define	EL_DifferentialPressureSensor	0x00, 0x1E	//	差圧センサ
#define	EL_AirSpeedSensor	0x00, 0x1F	//	風速センサ
#define	EL_OdorSensor	0x00, 0x20	//	臭いセンサ
#define	EL_FlameSensor	0x00, 0x21	//	炎センサ
#define	EL_ElectricEnergySensor	0x00, 0x22	//	電力量センサ
#define	EL_CurrentValueSensor	0x00, 0x23	//	電流値センサ
#define	EL_WaterFlowRateSensor	0x00, 0x25	//	水流量センサ
#define	EL_MicromotionSensor	0x00, 0x26	//	微動センサ
#define	EL_PassageSensor	0x00, 0x27	//	通過センサ
#define	EL_BedPresenceSensor	0x00, 0x28	//	在床センサ
#define	EL_OpenCloseSensor	0x00, 0x29	//	開閉センサ
#define	EL_ActivityAmountSensor	0x00, 0x2A	//	活動量センサ
#define	EL_HumanBodyLocationSensor	0x00, 0x2B	//	人体位置センサ
#define	EL_SnowSensor	0x00, 0x2C	//	雪センサ
// 空調関連機器
#define	EL_HomeAirConditioner	0x01, 0x30	//	家庭用エアコン
#define	EL_VentilationFan	0x01, 0x32	//	換気扇
#define	EL_AirConditionerVentilationFan	0x01, 0x34	//	空調換気扇
#define	EL_AirCleaner	0x01, 0x35	//	空気清浄器
#define	EL_Humidifier	0x01, 0x39	//	加湿器
#define	EL_ElectricHeater	0x01, 0x42	//	電気暖房機
#define	EL_FanHeater	0x01, 0x43	//	ファンヒータ
#define	EL_PackageTypeCommercialAirConditionerIndoorUnit	0x01, 0x56	//	業務用パッケージエアコン室内機
#define	EL_PackageTypeCommercialAirConditionerOutdoorUnit	0x01, 0x57	//	業務用パッケージエアコン室外機
// 住宅・設備関連機器
#define	EL_ElectricallyOperatedShade	0x02, 0x60	//	電動ブラインド・日よけ
#define	EL_ElectricShutter	0x02, 0x61	//	電動シャッター
#define	EL_ElectricStormWindow	0x02, 0x63	//	電動雨戸・シャッター
#define	EL_Sprinkler	0x02, 0x67	//	散水器(庭用)
#define	EL_ElectricWaterHeater	0x02, 0x6B	//	電気温水器
#define	EL_ElectricToiletSeat	0x02, 0x6E	//	電気便座(温水洗浄便座・暖房便座など)
#define	EL_ElectricLock	0x02, 0x6F	//	電気錠
#define	EL_InstantaneousWaterHeater	0x02, 0x72	//	瞬間式給湯機
#define	EL_BathroomHeaterAndDryer	0x02, 0x73	//	浴室暖房乾燥機
#define	EL_HouseholdSolarPowerGeneration	0x02, 0x79	//	住宅用太陽光発電
#define	EL_ColdOrHotWaterHeatSourceEquipment	0x02, 0x7A	//	冷温水熱源機
#define	EL_FloorHeater	0x02, 0x7B	//	床暖房
#define	EL_FuelCell	0x02, 0x7C	//	燃料電池
#define	EL_Battery	0x02, 0x7D	//	蓄電池
#define	EL_ElectricVehicle	0x02, 0x7E	//	電気自動車充放電器
#define	EL_EngineCogeneration	0x02, 0x7F	//	エンジンコージェネレーション
#define	EL_WattHourMeter	0x02, 0x80	//	電力量メータ
#define	EL_WaterFlowmeter	0x02, 0x81	//	水流量メータ
#define	EL_GasMeter	0x02, 0x82	//	ガスメータ
#define	EL_LPGasMeter	0x02, 0x83	//	LPガスメータ
#define	EL_PowerDistributionBoardMetering	0x02, 0x87	//	分電盤メータリング
#define	EL_SmartElectricEnergyMeter	0x02, 0x88	//	スマート電力量メータ
#define	EL_SmartGasMeter	0x02, 0x89	//	スマートガスメータ
#define	EL_GeneralLighting	0x02, 0x90	//	一般照明
#define	EL_Buzzer	0x02, 0xA0	//	ブザー
// 調理・家事関連機器
#define	EL_ElectricHotWaterPot	0x03, 0xB2	//	電気ポット
#define	EL_Refrigerator	0x03, 0xB7	//	冷凍冷蔵庫
#define	EL_CombinationMicrowaveOven	0x03, 0xB8	//	オーブンレンジ
#define	EL_CookingHeater	0x03, 0xB9	//	クッキングヒータ
#define	EL_RiceCooker	0x03, 0xBB	//	炊飯器
#define	EL_WashingMachine	0x03, 0xC5	//	洗濯機
#define	EL_ClothesDryer	0x03, 0xC6	//	衣類乾燥機
#define	EL_WasherAndDryer	0x03, 0xD3	//	洗濯乾燥機
// 健康関連機器
#define	EL_Weighing	0x04, 0x01	//	体重計
// 管理・操作関連機器
#define	EL_Switch	0x05, 0xFD	//	スイッチ(JEM-A/HA端子対応)
#define	EL_Controller	0x05, 0xFF	//	コントローラ
// AV関連機器
#define	EL_Display	0x06, 0x01	//	ディスプレー
#define	EL_Television	0x06, 0x02	//	テレビ
```

# Files

- examples/GeneralLighting/GeneralLighting.ino: sample program
- EL.h, EL.cpp: ECHONET Lite protocol class
- ELOBJ.h, ELOBJ.cpp: ECHONET Lite object manager class. It is required by EL.h


# Limitations

本モジュールの制限として気がついた点を下記に示します。

- 機器オブジェクトは1つだけしか持てない。


Limitations of this module is following.

- This module can have only a device object.


# Know-how


- もしESVがSETCのパケットを受信したときに再起動するなら，EL.returner関数でエラーが発生していると思われる。
 - 対応するEDTを下記のように設定してください。

```
 details[ 0xE0 ] = (byte[]){0x01, soundNumber}; // power
```


- If occuring reboot when receiving a packet (ESV is SETC), it seems that EL.returner function occurs an error probably.
 - Please set details (EDT) as following.

```
 details[ 0xE0 ] = (byte[]){0x01, soundNumber}; // power
```


# Version

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

SUGIMURA, Hiroshi
