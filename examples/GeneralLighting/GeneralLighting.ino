////////////////////////////////////////////////////////////////////////////////
// Arduino ECHONET Lite Sample
// Copyright (C) Hiroshi SUGIMURA 2013.09.27
////////////////////////////////////////////////////////////////////////////////
#include <SPI.h>
#include <Wire.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include "LCD.h" // AQM0802A-RN-GBW
#include <ELOBJ.h>
#include <EL.h>

EL echo(0x02, 0x90, 0x01); // General Lighting object code = 0290 01
LCD lcd;                   // IP address

// !!! please change the MAC address for your device !!!!!!!!!!!!!!!!!!!!!!!!!!!
byte mac[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// settings for lighting
// int lightPin = 13;   // generally LED pin
int lightPin = 9; // LED pin for Netduino

////////////////////////////////////////////////////////////////////////////////
// initialize
void setup()
{
  lcd.begin();
  pinMode(lightPin, OUTPUT); // lighting

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

  echo.printNetData(&lcd); // print status
  echo.begin();            // ECHONET Lite start

  // initial INF packet
  const byte deoj[] = {0x05, 0xff, 0x01};
  const byte edt[] = {0x01, 0x30};
  echo.sendMultiOPC1(deoj, INF, 0x80, edt);

  // device status, light on
  digitalWrite(lightPin, HIGH);
}

int packetSize = 0;     // receive data size
byte *pdcedt = nullptr; // temp

////////////////////////////////////////////////////////////////////////////////
// main
void loop()
{
  // receive data
  packetSize = 0;
  pdcedt = nullptr;

  if (packetSize = echo.read())
  {
    // -----------------------------------
    // ESV is SET, GET, etc ?
    switch (echo._rBuffer[ESV])
    {

      // -----------------------------------
      // change status, Set
      case SETI:
      case SETC:
        switch (echo._rBuffer[EPC])
        {
          case 0x80: // power
            if (echo._rBuffer[EDT] == 0x30)
            { // ON
              digitalWrite(lightPin, HIGH);
              pdcedt = new byte[2] {0x01, 0x30};       // change status
              echo.update(echo._rBuffer[EPC], pdcedt); // and status update
            }
            else if (echo._rBuffer[EDT] == 0x31)
            { // OFF
              digitalWrite(lightPin, LOW);
              pdcedt = new byte[2] {0x01, 0x31};       // change status
              echo.update(echo._rBuffer[EPC], pdcedt); // and status update
            }
            break;

          default: // unknown EPC
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

        // clear if use pdcedt
        if (pdcedt != nullptr)
        {
          delete[] pdcedt;
          pdcedt = nullptr;
        }

        if (echo._rBuffer[ESV] == SETC)
        { // SETC is no responce
          echo.returner();
        }
        break; // SETI, SETC

      // -----------------------------------
      // Get,INF_REQ
      case GET:
      case INF_REQ:
        // auto reply for device status if change status by using update function
        echo.returner();
        break;

      case INF:
        break;

      default: // unknown ESV
        Serial.print("error? ESV = ");
        Serial.println(echo._rBuffer[ESV]);
        break;
    }
  }
  // end EL process
  // -----------------------------------
  // no receiving packet, no operations.
  // else {
  // }

  delay(200);
}
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
