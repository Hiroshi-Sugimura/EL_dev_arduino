//////////////////////////////////////////////////////////////////////
// unit test
//////////////////////////////////////////////////////////////////////

// #include <M5Stack.h>
// #include <M5Core2.h>
#include <Arduino.h>
#include <WiFi.h>
#include <EL.h>

#define WIFI_SSID "ssid"        // !!!! change
#define WIFI_PASS "pass"  // !!!! change


//////////////////////////////////////////////////////////////////////
// proto type
void delPtr(byte ptr[]);


//////////////////////////////////////////////////////////////////////
// global
WiFiClient client;
WiFiUDP elUDP;

// single object
EL echo(elUDP, EL_GeneralLighting, 0x01);  // Controller = 0x05, 0xFF
// multi object
// byte objs={ {EL_GeneralLighting, 0x01}, {EL_GeneralLighting, 0x02} };
// EL echo(elUDP, objs, 2);  // light x 2


//////////////////////////////////////////////////////////////////////
// process
void setup() {
  // M5.begin();
  Serial.begin(115200);
  Serial.println("start");

  Serial.println("wifi connect start");

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("wifi connect ok");
  // M5.update();

  // print your WiFi IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address:");
  Serial.println(ip);

  //////////////////////////////////////////////////////////////////
  // unit test
  // ECHONET Lite initialize
  byte* pdcedt;
  pdcedt = new byte[2]{ 0x01, 0x42 };
  echo.update(0xB6, pdcedt);
  delPtr(pdcedt);

  pdcedt = new byte[2]{ 0x01, 0x30 };
  echo.update(0xB7, pdcedt);
  delPtr(pdcedt);

  echo.printAll();

  echo.begin();
  const byte deoj[] = { 0x05, 0xff, 0x01 };
  const byte edt[] = { 0x01, 0x30 };  // I am power on
  echo.sendMultiOPC1(deoj, EL_INF, 0x80, edt);
}

void loop() {
  // M5.update();
  delay(200);
}


void delPtr(byte ptr[]) {
  if (ptr != nullptr) {
    delete[] ptr;
    ptr = nullptr;
  }
}
