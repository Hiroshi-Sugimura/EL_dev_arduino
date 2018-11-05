//////////////////////////////////////////////////////////////////////
// ECHONET Lite protocol
//	Copyright (C) Hiroshi SUGIMURA 2013.09.27
//////////////////////////////////////////////////////////////////////
#ifndef __EL_H__
#define __EL_H__
#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUDP.h>
#include "ELOBJ.h"

// defined
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

class EL
{
private:
	IPAddress ip;
	IPAddress _multi;
	byte _broad[4];
	byte _eoj[3];
	int _sendPacketSize = 0;
	int _readPacketSize = 0;
	byte _sBuffer[EL_BUFFER_SIZE]; // send buffer
	WiFiUDP* _udp;

public:
	ELOBJ details;
	byte _rBuffer[EL_BUFFER_SIZE]; // receive buffer

	EL( WiFiUDP& udp, byte eoj0, byte eoj1, byte eoj2);
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
	void printNetData(); // lcdがあるときはそこにIP表示

	// byte[] を安全にdeleteする
	void delPtr(byte ptr[]);
};

#endif
//////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////
