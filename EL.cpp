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

    deviceCount = 1;
    _eojs = new byte[deviceCount * sizeof(byte[3])];
    _eojs[0] = eoj0;
    _eojs[1] = eoj1;
    _eojs[2] = eoj2;
}

// 複数のオブジェクトをサポートする。
EL::EL(WiFiUDP &udp, byte eoj[][3], int count)
{
    _udp = &udp;

    deviceCount = count;
    _eojs = new byte[deviceCount * sizeof(byte[3])];
    devices = new ELOBJ[deviceCount];
    for (int i = 0; i < deviceCount; i++) {
        _eojs[i * 3 + 0] = eoj[i][0];
        _eojs[i * 3 + 1] = eoj[i][1];
        _eojs[i * 3 + 2] = eoj[i][2];
    }

    _sendPacketSize = 0;
    memset(_sBuffer, 0, EL_BUFFER_SIZE);
    memset(_rBuffer, 0, EL_BUFFER_SIZE);

    _multi = IPAddress(224, 0, 23, 0);

    _broad[0] = 192;
    _broad[1] = 168;
    _broad[2] = 1;
    _broad[3] = 255;
}

void EL::begin(void)
{
    Serial.printf("deviceCount: %d", deviceCount);
    Serial.println();
    for (int i = 0; i < deviceCount; i++)
    {
        Serial.printf("eojs[%d] = %02x%02x%02x", i, *(_eojs + i * 3 + 0), *(_eojs + i * 3 + 1), *(_eojs + i * 3 + 2));
        Serial.println();
    }

	// udp
	if (_udp->begin(EL_PORT))
	{
		Serial.println("EL.udp.begin successful.");
	}
	else
	{
		Serial.println("Reseiver udp.begin failed."); // localPort
	}


	if (_udp->beginMulticast( _multi, EL_PORT) )
	{
		Serial.println("EL.udp.beginMulticast successful.");
	}
	else
	{
		Serial.println("Reseiver EL.udp.beginMulticast failed."); // localPort
	}

	// profile object
	profile[ 0x80 ] = new byte[2] {0x01, 0x30}; // power
	profile[ 0x81 ] = new byte[2] {0x01, 0x00}; // position
	profile[ 0x82 ] = new byte[5] {0x04, 0x01, 0x0a, 0x01, 0x00}; // Ver 1.10 (type 1)
	profile[ 0x83 ] = new byte[19]{0x12, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // identification number
	profile[ 0x88 ] = new byte[4] {0x01, 0x42}; // error status
	profile[ 0x8a ] = new byte[4] {0x03, 0x00, 0x00, 0x77}; // maker KAIT
	profile[ 0x9d ] = new byte[3] {0x02, 0x01, 0x80}; // inf property map
	profile[ 0x9e ] = new byte[3] {0x02, 0x01, 0x80}; // set property map
	profile[ 0x9f ] = new byte[16]{0x0f, 0x0e, 0x80, 0x81, 0x82, 0x83, 0x88, 0x8a, 0x9d, 0x9e, 0x9f, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7}; // get property map
    profile[ 0xd3 ] = new byte[4]{0x03, 0x00, 0x00, byte(deviceCount)};                                                                    // total instance number
    profile[ 0xd4 ] = new byte[3]{0x02, 0x00, byte(deviceCount + 1)};                                                                            // total class number
    profile[ 0xd5 ] = new byte[2 + deviceCount * sizeof(byte[3])]{byte(1 + deviceCount * 3), byte(deviceCount)}; // obj list
    profile[ 0xd6 ] = new byte[2 + deviceCount * sizeof(byte[3])]{byte(1 + deviceCount * 3), byte(deviceCount)}; // obj list
    profile[ 0xd7 ] = new byte[2 + deviceCount * sizeof(byte[2])]{byte(1 + deviceCount * 2), byte(deviceCount)}; // class list
    for (int i = 0; i < deviceCount; i++){
        memcpy(&profile[0xd5][2 + i * sizeof(byte[3])], &_eojs[i * sizeof(byte[3])], sizeof(byte[3]));
        memcpy(&profile[0xd6][2 + i * sizeof(byte[3])], &_eojs[i * sizeof(byte[3])], sizeof(byte[3]));
        memcpy(&profile[0xd7][2 + i * sizeof(byte[2])], &_eojs[i * sizeof(byte[3])], sizeof(byte[2]));
    }

	// device object
	details[ 0x80 ] = new byte[2] {0x01, 0x30}; // power
	details[ 0x81 ] = new byte[2] {0x01, 0x00}; // position
	details[ 0x82 ] = new byte[5] {0x04, 0x00, 0x00, 0x4b, 0x00}; // release K
	details[ 0x83 ] = new byte[19]{0x12, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // identification number
	details[ 0x88 ] = new byte[4] {0x01, 0x42}; // error status
	details[ 0x8a ] = new byte[4] {0x03, 0x00, 0x00, 0x77}; // maker KAIT
	details[ 0x9d ] = new byte[4] {0x03, 0x02, 0x80, 0xd6}; // inf property map
	details[ 0x9e ] = new byte[3] {0x02, 0x01, 0xe0}; // set property map
	details[ 0x9f ] = new byte[11]{0x0a, 0x09, 0x80, 0x81, 0x82, 0x83, 0x88, 0x8a, 0x9d, 0x9e, 0x9f}; // get property map

	details.printAll();
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

void EL::update(const int devId, const byte epc, byte pdcedt[])
{
    if (devId < deviceCount)
    	devices[devId].SetPDCEDT(epc, pdcedt); // power
}

byte *EL::at(const int devId, const byte epc)
{
    if (devId < deviceCount)
    	return devices[devId].GetPDCEDT(epc);
    else
        return nullptr;
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
void EL::sendMultiOPC1(const byte *tid, const byte *seoj, const byte *deoj, const byte esv, const byte epc, const byte *pdcedt)
{
	_sBuffer[EL_EHD1]		= 0x10;
	_sBuffer[EL_EHD2]		= 0x81;
	_sBuffer[EL_TID]		= tid[0];
	_sBuffer[EL_TID + 1]	= tid[1];
	_sBuffer[EL_SEOJ]		= seoj[0];
	_sBuffer[EL_SEOJ + 1]	= seoj[1];
	_sBuffer[EL_SEOJ + 2]	= seoj[2];
	_sBuffer[EL_DEOJ]		= deoj[0];
	_sBuffer[EL_DEOJ + 1]	= deoj[1];
	_sBuffer[EL_DEOJ + 2]	= deoj[2];
	_sBuffer[EL_ESV]		= esv;
	_sBuffer[EL_OPC]		= 0x01;
	_sBuffer[EL_EPC]		= epc;

	if( pdcedt != nullptr ) {
		memcpy(&_sBuffer[EL_PDC], pdcedt, pdcedt[0] + 1); // size = pcd + edt
		_sendPacketSize = EL_EDT + pdcedt[0];
	}else{
		_sBuffer[EL_PDC] = 0x00;
		_sendPacketSize = EL_PDC +1;
	}
	sendMulti(_sBuffer, _sendPacketSize);

#ifdef EL_DEBUG
	Serial.print("sendMultiOPC1 packet: ");
	for( int i=0; i<_sendPacketSize; i+=1) {
		Serial.print(_sBuffer[i], HEX );
		Serial.print( " " );
	}
	Serial.println( "." );
#endif
}

// OPC一個用のマルチキャスト関数（変なミスが少なくなるはず）
void EL::sendMultiOPC1(const byte *seoj, const byte *deoj, const byte esv, const byte epc, const byte *pdcedt)
{
	const byte tid[] = {0x00, 0x00};
	sendMultiOPC1(tid, seoj, deoj, esv, epc, pdcedt);
}

// OPC一個用のマルチキャスト関数（変なミスが少なくなるはず）
void EL::sendMultiOPC1(const byte *deoj, const byte esv, const byte epc, const byte *pdcedt)
{
	const byte tid[] = {0x00, 0x00};
	sendMultiOPC1(tid, _eoj, deoj, esv, epc, pdcedt);
}
void EL::sendMultiOPC1(const int devId, const byte *deoj, const byte esv, const byte epc, const byte *pdcedt)
{
    if (devId < deviceCount) {
        const byte tid[] = {0x00, 0x00};
        const byte seoj[] = {
            _eojs[devId * 3 + 0],
            _eojs[devId * 3 + 1],
            _eojs[devId * 3 + 2],
        };
        sendMultiOPC1(tid, seoj, deoj, esv, epc, pdcedt);
    }
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

// OPC1指定による送信(SEOJも指定する，ほぼ内部関数)
void EL::sendOPC1(const IPAddress toip, const byte *tid, const byte *seoj, const byte *deoj, const byte esv, const byte epc, const byte *pdcedt)
{
	_sBuffer[EL_EHD1]		= 0x10;
	_sBuffer[EL_EHD2]		= 0x81;
	_sBuffer[EL_TID]		= tid[0];
	_sBuffer[EL_TID + 1]	= tid[1];
	_sBuffer[EL_SEOJ]		= seoj[0];
	_sBuffer[EL_SEOJ + 1]	= seoj[1];
	_sBuffer[EL_SEOJ + 2]	= seoj[2];
	_sBuffer[EL_DEOJ]		= deoj[0];
	_sBuffer[EL_DEOJ + 1]	= deoj[1];
	_sBuffer[EL_DEOJ + 2]	= deoj[2];
	_sBuffer[EL_ESV]		= esv;
	_sBuffer[EL_OPC]		= 0x01;
	_sBuffer[EL_EPC]		= epc;

	if( pdcedt != nullptr ) {
		memcpy(&_sBuffer[EL_PDC], pdcedt, pdcedt[0] + 1); // size = pcd + edt
		_sendPacketSize = EL_EDT + pdcedt[0];
	}else{
		_sBuffer[EL_PDC] = 0x00;
		_sendPacketSize = EL_PDC +1;
	}
	send(toip, _sBuffer, _sendPacketSize);

#ifdef EL_DEBUG
	Serial.print("sendOPC1 packet: ");
	for( int i=0; i<_sendPacketSize; i+=1) {
		Serial.print(_sBuffer[i], HEX );
		Serial.print( " " );
	}
	Serial.println( "." );
#endif
}

// OPC1指定による送信(SEOJも指定する，ほぼ内部関数)
void EL::sendOPC1(const IPAddress toip, const byte *seoj, const byte *deoj, const byte esv, const byte epc, const byte *pdcedt)
{
	const byte tid[] = {0x00, 0x00};
	sendOPC1(toip, tid, seoj, deoj, esv, epc, pdcedt);
}


// OPC1指定による送信(SEOJは初期化時に指定したものを使う)
void EL::sendOPC1(const IPAddress toip, const byte *deoj, const byte esv, const byte epc, const byte *pdcedt)
{
	const byte tid[] = {0x00, 0x00};
	sendOPC1(toip, tid, _eoj, deoj, esv, epc, pdcedt);
}
void EL::sendOPC1(const IPAddress toip, const int devId, const byte *deoj, const byte esv, const byte epc, const byte *pdcedt)
{
    const byte tid[] = {0x00, 0x00};
    const byte seoj[] = {
        _eojs[devId * 3 + 0],
        _eojs[devId * 3 + 1],
        _eojs[devId * 3 + 2],
    };
    sendOPC1(toip, tid, seoj, deoj, esv, epc, pdcedt);
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
	byte tid[] 	   = {_rBuffer[EL_TID], _rBuffer[EL_TID + 1]};
	byte seoj[]    = {_rBuffer[EL_SEOJ], _rBuffer[EL_SEOJ + 1], _rBuffer[EL_SEOJ + 2]};
	byte deoj[]    = {_rBuffer[EL_DEOJ], _rBuffer[EL_DEOJ + 1], _rBuffer[EL_DEOJ + 2]};
	const byte esv = _rBuffer[EL_ESV];
	const byte epc = _rBuffer[EL_EPC];
	byte *pdcedt = nullptr;

	// 要求されたオブジェクトについて調べる
	if( deoj[0] == 0x0e && deoj[1] == 0xf0 ) { // 0ef0xx ならprofile object
		deoj[2] = 0x01;  // search等，インスタンス番号が0で送信される時があるので
		pdcedt = profile[epc];
	}else if( deoj[0] == _eoj[0] && deoj[1] == _eoj[1] ) { // 持っていないdevice objectは無視
		pdcedt = details[epc]; // その他はdevice object
	} else {
		// pdcedt = details[0][epc]; // その他はdevice object
        boolean noDevice = true;
        for (int i = 0; i < deviceCount; i++)
        {
            const byte eoj0 = _eojs[i * sizeof(byte[3]) + 0];
            const byte eoj1 = _eojs[i * sizeof(byte[3]) + 1];
            Serial.printf("%d, esv:%02x, epc:%02x, eoj0:%02x == %02x, eoj1:%02x == %02x", i, esv, epc, eoj0, deoj[0], eoj1, deoj[1]);
            Serial.println();
            if (deoj[0] == eoj0 && deoj[1] == eoj1)
            {
                pdcedt = devices[i][epc];
                noDevice = false;
                Serial.printf("pdcedt: %p", pdcedt);
                Serial.println();
                break;
            }
        }
        if (noDevice)
            return;
    }

    // esvの要求にこたえる
	switch (esv)
	{
	case EL_SETI:
		break; // SetIは返信しない
		///////////////////////////////////////////////////////////////////
		// SETC, Get, INF_REQ は返信処理がある
	case EL_SETC:
	case EL_GET:
		Serial.print("SETC or GET: ");
		Serial.println(epc, HEX);

		if (pdcedt)
		{ // そのEPCがある場合
			// Serial.println("There is pdcedt.");
			sendOPC1(remIP, tid, deoj, seoj, (esv + 0x10), epc, pdcedt);
		}
		else
		{
			// Serial.println("No pdcedt.");
			sendOPC1(remIP, tid, deoj, seoj, (esv - 0x10), epc, nullptr );
		}
		break;

		// ユニキャストへの返信ここまで，INFはマルチキャスト
	case EL_INF_REQ:
		Serial.print("INF_REQ: ");
		Serial.println(epc, HEX);
		if (pdcedt)
		{ // そのEPCがある場合、マルチキャスト
			sendMultiOPC1(tid, deoj, seoj, (esv + 0x10), epc, pdcedt);
		}
		else
		{ // ない場合はエラーなのでユニキャストで返信
			pdcedt[0] = 0x00;
			sendOPC1(remIP, tid, deoj, seoj, (esv - 0x10), epc, nullptr );
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
