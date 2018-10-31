//////////////////////////////////////////////////////////////////////
// ECHONET Lite protocol
//	Copyright (C) Hiroshi SUGIMURA 2013.09.27
//////////////////////////////////////////////////////////////////////
#ifndef __EL_H__
#define __EL_H__
#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

/*
  known problems:
   multi -> broad
*/

// defined
#define ELPORT 3610

#define EHD1 0
#define EHD2 1
#define TID 2
#define SEOJ 4
#define DEOJ 7
#define ESV 10
#define OPC 11
#define EPC 12
#define PDC 13
#define EDT 14

#define SETI_SNA 0x50
#define SETC_SNA 0x51
#define GET_SNA 0x52
#define INF_SNA 0x53
#define SETGET_SNA 0x5e
#define SETI 0x60
#define SETC 0x61
#define GET 0x62
#define INF_REQ 0x63
#define SETGET 0x6e
#define SET_RES 0x71
#define GET_RES 0x72
#define INF 0x73
#define INFC 0x74
#define INFC_RES 0x7a
#define SETGET_RES 0x7e

#define EL_BUFFER_SIZE 256

class EL
{
private:
  IPAddress ip;
  byte _multi[4];
  byte _broad[4];
  byte _eoj[3];
  int _sendPacketSize = 0;
  int _readPacketSize = 0;
  byte _sBuffer[EL_BUFFER_SIZE]; // send buffer
  EthernetUDP udp;

public:
  ELOBJ details;
  byte _rBuffer[EL_BUFFER_SIZE]; // receive buffer

  EL(byte eoj0, byte eoj1, byte eoj2);
  void begin(void);

  // details change
  void update(const byte epc, byte pdcedt[]);
  byte *at(const byte epc);

  // sender
  void send(IPAddress toip, byte sBuffer[], int size);
  void sendOPC1(const IPAddress toip, const byte *deoj, const byte esv, const byte epc, const byte *edt);
  void sendBroad(byte sBuffer[], int size);
  void sendMulti(byte sBuffer[], int size);
  void sendMultiOPC1(const byte *deoj, const byte esv, const byte epc, const byte *edt);

  // reseiver
  int parsePacket(void);
  int read();
  IPAddress remoteIP(void);
  void returner(void);

  // debug
  void printNetData();

  // byte[] を安全にdeleteする
  void delPtr(byte ptr[]);
};

#endif
//////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////
