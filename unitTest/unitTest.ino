#include <Arduino.h>
#include <WiFi.h>
#include "EL.h"


void setup() {
  Serial.begin(115200);
  Serial.println("start");

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
}

void loop() {
	Serial.println('end');
  delay(5000);
}
