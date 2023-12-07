# Overview (EL\_dev\_arduino)

このモジュールはArduinoの**ECHONET Liteプロトコル**通信をサポートします．
(ECHONET Liteプロトコルはスマートハウス機器の通信プロトコルです．)
M5Stack Core2を中心にテストしています。
**本モジュールはECHONET Lite認証を受けたものではありませんので、商品化する際には開発者自らが認証を受ける必要があります。**


This module provides the communication method, **ECHONET Lite protocol**, for Arduino.
(ECHONET Lite protocol is a common communication specification for Smart home)
We are developping and debugging with M5stack Core2 mainly.

## SDK details

- ECHONET Lite version 1.13
- (based) Release R, Rev.1

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

# Examples
- [API Ver. 3 (General Lighting)](https://github.com/Hiroshi-Sugimura/EL_dev_arduino/tree/master/examples/GeneralLighting)
- [API Ver. 4 (General Lighting)](https://github.com/Hiroshi-Sugimura/EL_dev_arduino/tree/master/examples/GeneralLightingV4esp)

## Other samples
- [EL_pj_arduino (GitHub repo)](https://github.com/Hiroshi-Sugimura/EL_pj_arduino)


# Description for the example

## Constractor

ELクラスはWiFiのUDP（受信は3610，送信は適当）とECHONET Liteプロトコルをサポートします。ECHONET Liteネットワークにデバイスを参加させるためには専門のオブジェクトコードが必要です。オブジェクトコードはECHONET規格書に書かれているものを参照してください [ECHONET規格書]（https://echonet.jp/spec_g/#standard-01）。
サンプルとして一般照明（0x029001）をM5Stackで実装した例を示しています。

EL class manages WiFiUDP (recv is 3610 port, sned is any port) and ECHONET Lite protocol.
To join ECHONET Lite network, the device needs ECHONET Lite object code.
ECHONET Lite object code is defined in the specifications. (see https://echonet.jp/spec-en/#standard-01)
In this example, we deals with M5stack as a general lighting (0x029001).

- single object
```cpp
EL echo(elUDP, 0x02, 0x90, 0x01 );
```

- multi object
```cpp
byte devices[][3] = { {0x02, 0x90, 0x01}, {0x02, 0x90, 0x02}};
EL echo(elUDP, devices, 2 );
```


### Initialize and Start communication（初期化と通信開始）

begin()メソドを呼ぶと受信開始します。

To call 'begin()' method begins receiving ECHONET Lite protocol.

- void begin(void);

```cpp:Ver.4
EL echo(elUDP, {{0x02, 0x90, 0x01}});

bool callback(byte tid[], byte seoj[], byte deoj[], byte esv, byte opc, byte epc, byte pdc, byte edt[]) {
  // user received data process
}

void setup() {
  echo.begin(callback);
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


```cpp:Ver.4
void loop() {
  echo.recvProcess();
  delay(300);
}
```



# Version

- 4.1.4 位置情報バグ取り、サンプルの修正
- 4.1.3 SETGETに返信するようにした（現在はSNA固定）
- 4.1.2 INFCに返信するようにした
- 4.1.1 エラーケースに対応、内部バージョン1.13, Rel.R, Rev.1とした
- 4.1.0 updateでINFが出るように修正、インスタンスリスト通知
- 4.0.1 新API追加、コンストラクタ追加
- 4.0.0 新API対応、update仕様変更
- 3.0.1 スタブ対応と細かいキャスト
- 3.0.0 change method name! sendOPC1(type devId) to sendOPC1ID, also sendMultiOPC1(type devId) to sendMultiOPC1ID. workaround for overload issue. Set/GetProfile to Set/GetMyPropertyMap. additional Keyword.
- 2.9.1 calc and set automatically the identification number(0x83) (devices)
- 2.9.0 calc and set automatically the identification number(0x83) (profile)
- 2.8.2 bugfix SetProfile, GetProfile
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
