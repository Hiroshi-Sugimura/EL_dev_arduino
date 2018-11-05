//////////////////////////////////////////////////////////////////////
// ECHONET Lite protocol
//	Copyright (C) Hiroshi SUGIMURA 2013.09.27
//////////////////////////////////////////////////////////////////////
#include "ELOBJ.h"
#include "EL.h"

// オブジェクトを一つだけサポートする。
EL::EL(WiFiUDP& udp, byte eoj0, byte eoj1, byte eoj2)
{
	_udp = &udp;
	_eoj[0] = eoj0;
	_eoj[1] = eoj1;
	_eoj[2] = eoj2;

	_sendPacketSize = 0;
	memset(_sBuffer, 0, EL_BUFFER_SIZE);
	memset(_rBuffer, 0, EL_BUFFER_SIZE);

	_multi = IPAddress(224,0,23,0);

	_broad[0] = 192;
	_broad[1] = 168;
	_broad[2] = 1;
	_broad[3] = 255;
}

void EL::begin(void)
{
	// udp
	if (_udp->begin(EL_PORT))
	{
		Serial.println("EL.udp.begin successful.");
	}
	else
	{
		Serial.println("Reseiver udp.begin failed."); // localPort
	}

	// IPAddress _multi(224,0,23,0);


	if (_udp->beginMulticast( _multi, EL_PORT) )
	{
		Serial.println("EL.udp.beginMulticast successful.");
	}
	else
	{
		Serial.println("Reseiver EL.udp.beginMulticast failed."); // localPort
	}

	details[ 0x80 ] = new byte[2] {0x01, 0x30}; // power
	details[ 0x82 ] = new byte[5] {0x04, 0x01, 0x0a, 0x01, 0x00}; // version 1.01
	details[ 0x83 ] = new byte[19] {0x12, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; //
	details[ 0x8a ] = new byte[4] {0x03, 0x00, 0x00, 0x77}; // maker KAIT
	details[ 0x9d ] = new byte[4] {0x03, 0x02, 0x80, 0xd6}; // inf p map
	details[ 0x9e ] = new byte[2] {0x01, 0x80}; // set p map
	details[ 0x9f ] = new byte[11] {0x0a, 0x09, 0x80, 0x82, 0x83, 0x8a, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7}; // get p map
	details[ 0xd3 ] = new byte[4] {0x03, 0x00, 0x00, 0x01}; //
	details[ 0xd4 ] = new byte[3] {0x02, 0x00, 0x02}; //
	details[ 0xd5 ] = new byte[4] {0x01, _eoj[0], _eoj[1], _eoj[2]}; // obj list
	details[ 0xd6 ] = new byte[4] {0x01, _eoj[0], _eoj[1], _eoj[2]}; // obj list
	details[ 0xd7 ] = new byte[3] {0x01, _eoj[0], _eoj[1]}; // class list

	details.printAll();

	// 繋がった宣言
	// 立ち上がったINFを飛ばす，まずはプロファイル
	byte s[] = {
		0x10, 0x81, 0x00, 0x00, 0x0e, 0xf0, 0x01, 0x05, 0xff, 0x01, EL_INF, 0x01, 0x80, 0x01, 0x30
		};
	sendMulti(s, sizeof(s) / sizeof(s[0]));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// details change
void EL::update(const byte epc, byte pdcedt[])
{
	details.SetPDCEDT(epc, pdcedt); // power
}

byte *EL::at(const byte epc)
{
	return details.GetPDCEDT(epc);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// sender

// ブロードキャストによる送信
void EL::sendBroad(byte sBuffer[], int size)
{
	if (_udp->beginPacket(_broad, EL_PORT))
	{
		// Serial.println("UDP beginPacket(B) Successful.");
		_udp->write(sBuffer, size);
	}

	if (_udp->endPacket())
	{
		// Serial.println("UDP endPacket(B) Successful.");
	}
	else
	{
		Serial.println("UDP endPacket(B) failed.");
	}
}

// マルチと見せかけてブロードキャストによる送信
// このようにしておくとArduinoがマルチ対応したときに互換性を保てるハズ
void EL::sendMulti(byte sBuffer[], int size)
{
	// sendBroad(sBuffer, size);

	if (_udp->beginMulticastPacket())
	{
		// Serial.println("UDP beginPacket(B) Successful.");
		_udp->write(sBuffer, size);
	}

	if (_udp->endPacket())
	{
		// Serial.println("UDP endPacket(B) Successful.");
	}
	else
	{
		Serial.println("UDP endPacket(M) failed.");
	}

}

// OPC一個用のマルチキャスト関数（変なミスが少なくなるはず）
void EL::sendMultiOPC1(const byte *deoj, const byte esv, const byte epc, const byte *pdcedt)
{
	_sBuffer[EL_EHD1] = 0x10;
	_sBuffer[EL_EHD2] = 0x81;
	_sBuffer[EL_TID] = 0x00;
	_sBuffer[EL_TID + 1] = 0x00;
	_sBuffer[EL_SEOJ] = _eoj[0]; // node profile
	_sBuffer[EL_SEOJ + 1] = _eoj[1];
	_sBuffer[EL_SEOJ + 2] = _eoj[2];
	_sBuffer[EL_DEOJ] = deoj[0]; // node profile
	_sBuffer[EL_DEOJ + 1] = deoj[1];
	_sBuffer[EL_DEOJ + 2] = deoj[2];
	_sBuffer[EL_ESV] = esv;
	_sBuffer[EL_OPC] = 0x01;
	_sBuffer[EL_EPC] = epc;
	memcpy(&_sBuffer[EL_PDC], pdcedt, pdcedt[0] + 1); // size = pcd + edt
	_sendPacketSize = EL_EDT + pdcedt[0];
	sendMulti(_sBuffer, _sendPacketSize);
}

// IP指定による送信
void EL::send(IPAddress toip, byte sBuffer[], int size)
{

	if (_udp->beginPacket(toip, EL_PORT))
	{
		// Serial.println("UDP beginPacket Successful.");
		_udp->write(sBuffer, size);
	}
	else
	{
		Serial.println("UDP beginPacket failed.");
	}

	if (_udp->endPacket())
	{
		// Serial.println("UDP endPacket Successful.");
	}
	else
	{
		Serial.println("UDP endPacket failed.");
	}
}

// OPC1指定による送信(SEOJは初期化時に指定したものを使う)
void EL::sendOPC1(const IPAddress toip, const byte *deoj, const byte esv, const byte epc, const byte *pdcedt)
{
	_sBuffer[EL_EHD1] = 0x10;
	_sBuffer[EL_EHD2] = 0x81;
	_sBuffer[EL_TID] = 0x00;
	_sBuffer[EL_TID + 1] = 0x00;
	_sBuffer[EL_SEOJ] = _eoj[0]; // node profile
	_sBuffer[EL_SEOJ + 1] = _eoj[1];
	_sBuffer[EL_SEOJ + 2] = _eoj[2];
	_sBuffer[EL_DEOJ] = deoj[0]; // node profile
	_sBuffer[EL_DEOJ + 1] = deoj[1];
	_sBuffer[EL_DEOJ + 2] = deoj[2];
	_sBuffer[EL_ESV] = esv;
	_sBuffer[EL_OPC] = 0x01;
	_sBuffer[EL_EPC] = epc;
	memcpy(&_sBuffer[EL_PDC], pdcedt, pdcedt[0] + 1); // size = pcd + edt
	_sendPacketSize = EL_EDT + pdcedt[0];
	send(toip, _sBuffer, _sendPacketSize);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// reseiver
int EL::parsePacket(void)
{
	return _udp->parsePacket();
}

IPAddress EL::remoteIP(void)
{
	return _udp->remoteIP();
}

int EL::read(void)
{
	_readPacketSize = parsePacket();

	if (_readPacketSize)
	{
		_udp->read(_rBuffer, EL_BUFFER_SIZE); // 受け取った内容読み取り
	}
	return _readPacketSize;
}

// details
void EL::returner(void)
{
	///////////////////////////////////////////////////////////////////
	// 動作状態の変更
	IPAddress remIP = remoteIP();
	byte deoj[] = {_rBuffer[EL_DEOJ], _rBuffer[EL_DEOJ + 1], _rBuffer[EL_DEOJ + 2]};
	const byte esv = _rBuffer[EL_ESV];
	const byte epc = _rBuffer[EL_EPC];
	byte *pdcedt = details[epc];

	switch (esv)
	{
	case EL_SETI:
		break; // Setは返信しない
		///////////////////////////////////////////////////////////////////
		// SETC, Get, INF_REQ は返信処理がある
	case EL_SETC:
	case EL_GET:
		Serial.print("get: ");
		Serial.print(epc, HEX);
		Serial.print(", ");
		Serial.print(pdcedt[0], HEX);
		Serial.print(", ");
		Serial.println(pdcedt[1], HEX);

		if (pdcedt)
		{ // そのEPCがある場合
			sendOPC1(remIP, deoj, (esv + 0x10), epc, pdcedt);
		}
		else
		{
			pdcedt[0] = 0x00;
			sendOPC1(remIP, deoj, (esv - 0x10), epc, pdcedt);
		}
		break;

		// ユニキャストへの返信ここまで，INFはマルチキャスト
	case EL_INF_REQ:
		if (pdcedt)
		{ // そのEPCがある場合、マルチキャスト
			sendMultiOPC1(deoj, (esv + 0x10), epc, pdcedt);
		}
		else
		{ // ない場合はエラーなのでユニキャストで返信
			pdcedt[0] = 0x00;
			sendOPC1(remIP, deoj, (esv - 0x10), epc, pdcedt);
		}
		break;
		//  INF_REQここまで

	default: // 解釈不可能なESV
		break;
	}
}
// EL処理ここまで


// byte[] を安全にdeleteする
inline void EL::delPtr(byte ptr[])
{
	if (ptr != nullptr)
	{
		delete[] ptr;
		ptr = nullptr;
	}
}

//////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////
