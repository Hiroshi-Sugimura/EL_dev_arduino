//////////////////////////////////////////////////////////////////////
/// @file EL.cpp
/// @brief ECHONET Lite protocol for Arduino
/// @author SUGIMURA Hiroshi
/// @date 2013.09.27
/// @details https://github.com/Hiroshi-Sugimura/EL_dev_arduino
//////////////////////////////////////////////////////////////////////
#include "ELOBJ.h"
#include "EL.h"

////////////////////////////////////////////////////
/// @brief オブジェクトを一つだけサポートする場合のコンストラクタ
/// @param WiFiUDP&
/// @param byte eoj0: class group code
/// @param byte eoj1: class code
/// @param byte eoj2: instance code
/// @return none
/// @note eoj0, eoj1, eoj2で一つのオブジェクト
/// 一般照明の例
/// ex. EL(udp, 0x02, 0x90, 0x01);
EL::EL(WiFiUDP &udp, byte classGroupCode, byte classCode, byte instanceNumber)
{
	byte eoj[1][3] = {{ classGroupCode, classCode, instanceNumber }};
	commonConstructor(udp, eoj, 1);
}

////////////////////////////////////////////////////
/// @brief オブジェクトを複数サポートする場合のコンストラクタ
/// @param WiFiUDP&
/// @param byte eoj[][3]
/// @param int count
/// @return none
/// @note
EL::EL(WiFiUDP &udp, byte eojs[][3], int count)
{
	commonConstructor(udp, eojs, count);
}


////////////////////////////////////////////////////
/// @brief コンストラクタ共通処理
/// @param udp WiFiUDP&
/// @param eojs byte[][3]
/// @param count int
/// @return none
/// @note
void EL::commonConstructor(WiFiUDP &udp, byte eojs[][3], int count)
{
	_udp = &udp;

	deviceCount = count;
	_eojs = new byte[deviceCount * 3];  // [count][3], 3 = classGroupCode, classCode, instanceNumber
	devices = new ELOBJ[deviceCount];

	for (int i = 0; i < deviceCount; i++)
	{
		_eojs[i * 3 + 0] = eojs[i][0];
		_eojs[i * 3 + 1] = eojs[i][1];
		_eojs[i * 3 + 2] = eojs[i][2];
	}

	_sendPacketSize = 0;
	memset(_sBuffer, 0, EL_BUFFER_SIZE);
	memset(_rBuffer, 0, EL_BUFFER_SIZE);

	_multi = IPAddress(224, 0, 23, 0);

	// 仮
	_broad = IPAddress(192, 168, 1, 255);

	_tid[0] = 0;
	_tid[1] = 0;

	// profile object
	profile[0x80] = new byte[2]{0x01, 0x30};																								  // power
	profile[0x81] = new byte[2]{0x01, 0x00};																								  // osition
	profile[0x82] = new byte[5]{0x04, 0x01, 0x0a, 0x01, 0x00};																				  // Ver 1.10 (type 1)
	profile[0x83] = new byte[19]{0x12, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // identification number
	profile[0x88] = new byte[4]{0x01, 0x42};																								  // error status
	profile[0x8a] = new byte[4]{0x03, 0x00, 0x00, 0x77};																					  // maker KAIT
	profile[0x9d] = new byte[3]{0x02, 0x01, 0x80};																							  // inf property map
	profile[0x9e] = new byte[3]{0x02, 0x01, 0x80};																							  // set property map
	profile[0x9f] = new byte[16]{0x0f, 0x0e, 0x80, 0x81, 0x82, 0x83, 0x88, 0x8a, 0x9d, 0x9e, 0x9f, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7};			  // get property map
	profile[0xd3] = new byte[4]{0x03, 0x00, 0x00, byte(deviceCount)};																		  // total instance number
	profile[0xd4] = new byte[3]{0x02, 0x00, byte(deviceCount + 1)};																			  // total class number
	profile[0xd5] = new byte[2 + deviceCount * sizeof(byte[3])]{byte(1 + deviceCount * 3), byte(deviceCount)};								  // obj list
	profile[0xd6] = new byte[2 + deviceCount * sizeof(byte[3])]{byte(1 + deviceCount * 3), byte(deviceCount)};								  // obj list
	profile[0xd7] = new byte[2 + deviceCount * sizeof(byte[2])]{byte(1 + deviceCount * 2), byte(deviceCount)};								  // class list
	for (int i = 0; i < deviceCount; i++)
	{
		memcpy(&profile[0xd5][2 + i * sizeof(byte[3])], &_eojs[i * sizeof(byte[3])], sizeof(byte[3]));
		memcpy(&profile[0xd6][2 + i * sizeof(byte[3])], &_eojs[i * sizeof(byte[3])], sizeof(byte[3]));
		memcpy(&profile[0xd7][2 + i * sizeof(byte[2])], &_eojs[i * sizeof(byte[3])], sizeof(byte[2]));
	}

	// device object
	for (int i = 0; i < deviceCount; i++)
	{
		devices[i][0x80] = new byte[2]{0x01, 0x30};																								  // power
		devices[i][0x81] = new byte[2]{0x01, 0x00};																								  // position
		devices[i][0x82] = new byte[5]{0x04, 0x00, 0x00, 0x4b, 0x00};																				  // release K
		devices[i][0x83] = new byte[19]{0x12, 0xfe, _eojs[i * 3 + 0], _eojs[i * 3 + 1], _eojs[i * 3 + 2], 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // identification number
		devices[i][0x88] = new byte[4]{0x01, 0x42};																								  // error status
		devices[i][0x8a] = new byte[4]{0x03, 0x00, 0x00, 0x77};																					  // maker KAIT
		devices[i][0x9d] = new byte[4]{0x03, 0x02, 0x80, 0xd6};																					  // inf property map
		devices[i][0x9e] = new byte[3]{0x02, 0x01, 0xe0};																							  // set property map
		devices[i][0x9f] = new byte[11]{0x0a, 0x09, 0x80, 0x81, 0x82, 0x83, 0x88, 0x8a, 0x9d, 0x9e, 0x9f};											  // get property map
		devices[i].printAll();
	}
}


////////////////////////////////////////////////////
/// @brief TIDの自動インクリメント、オーバーフロー対策
/// @param none
/// @return none
/// @note
void EL::tidAutoIncrement(void)
{
	if( _tid[0] == 0xff && _tid[1] == 0xff ) {
		_tid[0] = 0; _tid[1] = 0;
	}else if( _tid[1] == 0xff ) {
		_tid[0] += 1; _tid[1] = 0;
	}else{
		_tid[1] += 1;
	}
}


////////////////////////////////////////////////////
/// @brief 通信の開始、受信開始
/// @param none
/// @return none
/// @note
void EL::begin(void)
{
	Serial.println("=========== EL.begin");
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

	if (_udp->beginMulticast(_multi, EL_PORT))
	{
		Serial.println("EL.udp.beginMulticast successful.");
	}
	else
	{
		Serial.println("Reseiver EL.udp.beginMulticast failed."); // localPort
	}

	// 接続ネットワークのブロードキャストアドレスに更新
	_broad = IPAddress( ip[0], ip[1], ip[2], 255);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief EPCの値を変更する, eojが1個の場合（複数の場合は0番に相当）
/// @param epc const byte
/// @param pdcedt byte[]
/// @return none
/// @note pdcedtなので、pdcは自分で計算することに注意
void EL::update(const byte epc, byte pdcedt[])
{
	devices[0].SetPDCEDT(epc, pdcedt); // power
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief EPCの値を取得する, eojが1個の場合（複数の場合は0番に相当）
/// @param epc const byte
/// @return byte*
byte* EL::at(const byte epc)
{
	return devices[0].GetPDCEDT(epc);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief EPCの値を変更する, 複数の場合
/// @param devId const int, コンストラクタで渡した順番に相当
/// @param epc const byte
/// @param byte pdcedt[]
/// @return none
void EL::update(const int devId, const byte epc, byte pdcedt[])
{
	if (devId < deviceCount)
		devices[devId].SetPDCEDT(epc, pdcedt); // power
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief EPCの値を取得する, 複数の場合
/// @param devId const int, コンストラクタで渡した順番に相当
/// @param epc const byte
/// @return none
byte* EL::at(const int devId, const byte epc)
{
	if (devId < deviceCount)
		return devices[devId].GetPDCEDT(epc);
	else
		return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// sender
////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief ブロードキャストによる送信(default: 192.168.1.255)
/// @param byte sBuffer[]
/// @param int size
/// @return none
/// @note !!deprecated!! 非推奨機能なので、本番環境では利用しないように
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

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief マルチキャストによる送信(default: 192.168.1.255)
/// @param byte sBuffer[]
/// @param int size
/// @return none
/// バージョンによってはブロードキャストによる送信の場合がある。
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

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief OPC一個用のマルチキャスト送信、TID指定有り
/// @param tid const byte*
/// @param seoj const byte*
/// @param deoj const byte*
/// @param esv const byte
/// @param epc const byte
/// @param pdcedt const byte*
/// @return none
void EL::sendMultiOPC1(const byte* tid, const byte* seoj, const byte* deoj, const byte esv, const byte epc, const byte* pdcedt)
{
	_sBuffer[EL_EHD1] = 0x10;
	_sBuffer[EL_EHD2] = 0x81;
	_sBuffer[EL_TID] = tid[0];
	_sBuffer[EL_TID + 1] = tid[1];
	_sBuffer[EL_SEOJ] = seoj[0];
	_sBuffer[EL_SEOJ + 1] = seoj[1];
	_sBuffer[EL_SEOJ + 2] = seoj[2];
	_sBuffer[EL_DEOJ] = deoj[0];
	_sBuffer[EL_DEOJ + 1] = deoj[1];
	_sBuffer[EL_DEOJ + 2] = deoj[2];
	_sBuffer[EL_ESV] = esv;
	_sBuffer[EL_OPC] = 0x01;
	_sBuffer[EL_EPC] = epc;

	if (pdcedt != nullptr)
	{
		memcpy(&_sBuffer[EL_PDC], pdcedt, pdcedt[0] + 1); // size = pcd + edt
		_sendPacketSize = EL_EDT + pdcedt[0];
	}
	else
	{
		_sBuffer[EL_PDC] = 0x00;
		_sendPacketSize = EL_PDC + 1;
	}
	sendMulti(_sBuffer, _sendPacketSize);

#ifdef EL_DEBUG
	Serial.print("sendMultiOPC1 packet: ");
	for (int i = 0; i < _sendPacketSize; i += 1)
	{
		Serial.print(_sBuffer[i], HEX);
		Serial.print(" ");
	}
	Serial.println(".");
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief OPC一個用のマルチキャスト送信、TID自動
/// @param seoj const byte*
/// @param deoj const byte*
/// @param esv const byte
/// @param epc const byte
/// @param pdcedt const byte *
/// @return none
void EL::sendMultiOPC1(const byte* seoj, const byte* deoj, const byte esv, const byte epc, const byte* pdcedt)
{
	sendMultiOPC1(_tid, seoj, deoj, esv, epc, pdcedt);
	tidAutoIncrement();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief OPC一個用のマルチキャスト送信、seoj省略（0番）、TID自動
/// @param deoj const byte*
/// @param esv const byte
/// @param epc const byte
/// @param pdcedt const byte*
/// @return none
void EL::sendMultiOPC1(const byte* deoj, const byte esv, const byte epc, const byte* pdcedt)
{
	byte eoj[3] = {_eojs[0], _eojs[1], _eojs[2]};
	sendMultiOPC1(_tid, eoj, deoj, esv, epc, pdcedt);
	tidAutoIncrement();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief OPC一個用のマルチキャスト送信、seojの代わりにIDで指定、TID自動
/// @param devId const byte int
/// @param deoj const byte*
/// @param esvconst byte
/// @param epc const byte
/// @param pdcedt const byte*
/// @return none
/// @note recommendation 推奨
void EL::sendMultiOPC1(const int devId, const byte* deoj, const byte esv, const byte epc, const byte* pdcedt)
{
	if (devId < deviceCount)
	{
		byte eoj[3] = {_eojs[devId * 3 + 0], _eojs[devId * 3 + 1], _eojs[devId * 3 + 2]};
		sendMultiOPC1( _tid, eoj, deoj, esv, epc, pdcedt);
		tidAutoIncrement();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief IP指定による送信
/// @param toip IPAddress
/// @param sBuffer byte[]
/// @param size int size of sBuffer
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

////////////////////////////////////////////////////
/// @brief
/// @param
/// @return none
/// @note
// OPC1指定による送信(SEOJも指定する，ほぼ内部関数)
void EL::sendOPC1(const IPAddress toip, const byte tid[], const byte *seoj, const byte *deoj, const byte esv, const byte epc, const byte *pdcedt)
{
	_sBuffer[EL_EHD1] = 0x10;
	_sBuffer[EL_EHD2] = 0x81;
	_sBuffer[EL_TID] = tid[0];
	_sBuffer[EL_TID + 1] = tid[1];
	_sBuffer[EL_SEOJ] = seoj[0];
	_sBuffer[EL_SEOJ + 1] = seoj[1];
	_sBuffer[EL_SEOJ + 2] = seoj[2];
	_sBuffer[EL_DEOJ] = deoj[0];
	_sBuffer[EL_DEOJ + 1] = deoj[1];
	_sBuffer[EL_DEOJ + 2] = deoj[2];
	_sBuffer[EL_ESV] = esv;
	_sBuffer[EL_OPC] = 0x01;
	_sBuffer[EL_EPC] = epc;

	if (pdcedt != nullptr)
	{
		memcpy(&_sBuffer[EL_PDC], pdcedt, pdcedt[0] + 1); // size = pcd + edt
		_sendPacketSize = EL_EDT + pdcedt[0];
	}
	else
	{
		_sBuffer[EL_PDC] = 0x00;
		_sendPacketSize = EL_PDC + 1;
	}
	send(toip, _sBuffer, _sendPacketSize);

#ifdef EL_DEBUG
	Serial.print("sendOPC1 packet: ");
	for (int i = 0; i < _sendPacketSize; i += 1)
	{
		Serial.print(_sBuffer[i], HEX);
		Serial.print(" ");
	}
	Serial.println(".");
#endif
}

////////////////////////////////////////////////////
/// @brief
/// @param
/// @return none
/// @note
// OPC1指定による送信(SEOJも指定する，ほぼ内部関数)
void EL::sendOPC1(const IPAddress toip, const byte seoj[], const byte deoj[], const byte esv, const byte epc, const byte pdcedt[])
{
	sendOPC1(toip, _tid, seoj, deoj, esv, epc, pdcedt);
	tidAutoIncrement();
}


////////////////////////////////////////////////////
/// @brief
/// @param
/// @return none
/// @note
// OPC1指定による送信(SEOJは初期化時に指定したものを使う)
void EL::sendOPC1(const IPAddress toip, const byte deoj[], const byte esv, const byte epc, const byte pdcedt[])
{
	byte eoj[3] = {_eojs[0], _eojs[1], _eojs[2]};
	sendOPC1(toip, _tid, eoj, deoj, esv, epc, pdcedt);
	tidAutoIncrement();
}

////////////////////////////////////////////////////
/// @brief
/// @param
/// @return none
/// @note
void EL::sendOPC1(const IPAddress toip, const int devId, const byte deoj[], const byte esv, const byte epc, const byte pdcedt[])
{
	byte eoj[3] = {_eojs[devId * 3 + 0], _eojs[devId * 3 + 1], _eojs[devId * 3 + 2]};
	sendOPC1(toip, _tid, eoj, deoj, esv, epc, pdcedt);
	tidAutoIncrement();
}


////////////////////////////////////////////////////
/// @brief 複数のEPCで送信する場合はこれを使う
/// @param toip const IPAddress:送信先
/// @param tid const tid:TID
/// @param seoj const byte[3]
/// @param deoj const byte[3]
/// @param esv const byte
/// @param epc const byte
/// @param detail const byte[N]: {EPC, PDC, EDT[x]}[y]
/// @param detailSize const byte: size of detail N
/// @return none
/// @note
void EL::sendDetails(const IPAddress toip, const byte tid[], const byte seoj[], const byte deoj[], const byte esv, const byte opc, const byte detail[], const byte detailSize)
{
	_sBuffer[EL_EHD1]   = 0x10;
	_sBuffer[EL_EHD2]   = 0x81;
	_sBuffer[EL_TID]    = tid[0];
	_sBuffer[EL_TID+1]  = tid[1];
	_sBuffer[EL_SEOJ]   = seoj[0];
	_sBuffer[EL_SEOJ+1] = seoj[1];
	_sBuffer[EL_SEOJ+2] = seoj[2];
	_sBuffer[EL_DEOJ]   = deoj[0];
	_sBuffer[EL_DEOJ+1] = deoj[1];
	_sBuffer[EL_DEOJ+2] = deoj[2];
	_sBuffer[EL_ESV] = esv;
	_sBuffer[EL_OPC] = opc;

	if (detail != nullptr)
	{
		memcpy( &_sBuffer[EL_EPC], detail, detailSize); // size = pcd + edt
		_sendPacketSize = EL_EPC + detailSize;
		send(toip, _sBuffer, _sendPacketSize);
	}
	else
	{
		Serial.println("Error: EL::sendDetails, detail is nullptr.");
	}
}


// ELの返信用、典型的なOPC一個でやる．TIDを併せて返信しないといけないため
// void EL::replyOPC1(const IPAddress toip, const unsigned short tid, const byte* seoj, const byte* deoj, const byte esv, const byte epc, const byte* edt) {}

////////////////////////////////////////////////////
/// @brief Getに対して複数OPCにも対応して返答する内部関数
/// @param toip const IPAddress
/// @return void
/// @note
void EL::replyGetDetail(const IPAddress toip)
{
	byte tid[]  = {_rBuffer[EL_TID], _rBuffer[EL_TID + 1]};
	byte seoj[] = {_rBuffer[EL_DEOJ], _rBuffer[EL_DEOJ + 1], _rBuffer[EL_DEOJ + 2]};   // DEOJがreplyではSEOJになる
	byte deoj[] = {_rBuffer[EL_SEOJ], _rBuffer[EL_SEOJ + 1], _rBuffer[EL_SEOJ + 2]};   // SEOJがreplyではDEOJになる
	byte esv = _rBuffer[EL_ESV];
	byte opc = _rBuffer[EL_OPC];

	// 送信用
	boolean success = true;
	byte detail[1500];  // EPC,PDC,EDT[n]
	byte detailSize = 0;    // data size

	// rBuffer走査用
	byte* p_rEPC = &_rBuffer[EL_EPC];  // 初期EPCポインタ
	// この関数を呼ばれるのはGETの場合なので、 PDC:0x00, EDTなしに決まっている。
	// 従って、EPCだけを焦点とする

	// temp
	byte* pdcedt = nullptr;         // pdc edt
	byte devId;

	for( byte i=0; i<opc; i+=1, p_rEPC += 2 )  // OPC個数のEPCに回答する
	{
		Serial.printf("i:%d EPC:%X\n", i, *p_rEPC);
		boolean exist = replyGetDetail_sub(seoj, *p_rEPC, devId);
		if( exist )
		{
			// ある
			Serial.printf("devId: %x\n", devId);
			if( devId == 0xff )  // devId = 0xff is profile
			{
				pdcedt = profile[*p_rEPC];  // EPCに対応するPDCEDT確保
				// Serial.printf("node prof: pdcedt: %x %x %x\n", pdcedt[0], pdcedt[1], pdcedt[2]);
				detail[detailSize] = *p_rEPC;  // EPCに対して
				detailSize += 1;
				// PDCとEDTを設定
				memcpy( &detail[detailSize], pdcedt, pdcedt[0] + 1 ); // size = pcd + edt
				detailSize += pdcedt[0] + 1;
			}else{
				pdcedt = devices[devId][*p_rEPC];  // EPCに対応するPDCEDT確保
				// Serial.printf("dev obj: pdcedt: %x %x %x\n", pdcedt[0], pdcedt[1], pdcedt[2]);
				detail[detailSize] = *p_rEPC;  // EPCに対して
				detailSize += 1;
				// PDCとEDT確保
				memcpy( &detail[detailSize], pdcedt, pdcedt[0] + 1 ); // size = pcd + edt
				detailSize += pdcedt[0] + 1;
			}
		}
		else
		{
			// ない
			if( devId == 0xfe ) { // そもそもDEOJが自分のオブジェクトでない場合は無視（@@ 追加）
				return;
			}else{  // DEOJはあるが、EPCがない
				// Serial.println("nothing");
				memcpy( &detail[detailSize], pdcedt, pdcedt[0] + 1 ); // size = pcd + edt
				detail[detailSize] = *p_rEPC;
				detailSize += 1;
				detail[detailSize] = 0x00;
				detailSize += 1;
				success = false;
			}
		}
	}

	// Serial.printf("detailSize: %d\n", detailSize);

	esv = success? EL_GET_RES: EL_GET_SNA;  // 一つでも失敗したらGET_SNA、全部OKならGET_RES
	sendDetails(  toip,  tid,  seoj, deoj,  esv, opc, detail,  detailSize);
}


////////////////////////////////////////////////////
/// @brief EOJとEPCを指定したとき、そのプロパティ（EDT）があるかチェックする内部関数
/// @param eoj const byte[]
/// @param epc const byte
/// @param devId[out] byte&: -1:profile, x:devId
/// @return true:無し、false:あり
/// @note replyGetDetailのサブルーチン、GetPropertyMapを参照しなくても、基本的に持っているPeopertyはGet可能なのでMapチェックしなくてよい
boolean EL::replyGetDetail_sub( const byte eoj[], const byte epc, byte& devId )
{
	devId = 0xfe;  // 0xfe はOJB無しとする（） // @@@ 実際は別の方法でOBJ無しとしないとバグ
	if( eoj[0] == 0x0e && eoj[1] == 0xf0 && eoj[2] == 0x01 )  // profile object
	{
		devId = 0xff;  // devId = -1 は node profileとする。（この方式は後で変更）
		byte* pdcedt = profile[epc];

		if( pdcedt == nullptr ) return false;  // epcがない
		return true;
	}

	// device object
	for (int i = 0; i < deviceCount; i++)  // deojとマッチするdevIdを調べる
	{
		if (eoj[0] == _eojs[i * 3 + 0] && eoj[1] == _eojs[i * 3 + 1])
		{
			devId = i;
			byte* pdcedt = devices[devId][epc];

			if( pdcedt == nullptr ) return false;  // epcがない
			return true;
		}
	}
	return false;  // no eoj, no epc
}


////////////////////////////////////////////////////
/// @brief Setに対して複数OPCにも対応して返答する内部関数
/// @param toip const IPAddress
/// @return void
/// @note EPC毎の設定値に関して基本はノーチェックなので注意すべし
/// EPC毎の設定値チェックや、INF処理に関しては下記の replySetDetail_sub にて実施
/// SET_RESはEDT入ってない
void EL::replySetDetail(const IPAddress toip)
{
	byte tid[]  = {_rBuffer[EL_TID], _rBuffer[EL_TID + 1]};
	byte seoj[] = {_rBuffer[EL_DEOJ], _rBuffer[EL_DEOJ + 1], _rBuffer[EL_DEOJ + 2]};   // DEOJがreplyではSEOJになる
	byte deoj[] = {_rBuffer[EL_SEOJ], _rBuffer[EL_SEOJ + 1], _rBuffer[EL_SEOJ + 2]};   // SEOJがreplyではDEOJになる
	byte esv = _rBuffer[EL_ESV];
	byte opc = _rBuffer[EL_OPC];

	// 送信用
	boolean success = true;
	byte detail[1500];  // EPC,PDC,EDT[n]
	byte detailSize = 0;    // data size

	// rBuffer走査用
	byte* p_rEPC = &_rBuffer[EL_EPC];  // 初期EPCポインタ
	// この関数を呼ばれるのはSETの場合であるので、 PDCを見ながらEDT分をスキップしていく
	byte* p_rPDC = &_rBuffer[EL_EPC+1]; // 初期PDCポインタ

	// temp
	byte* pdcedt = nullptr;         // pdc edt
	byte devId;

	for( byte i=0; i<opc; i+=1 )  // OPC個数のEPCに回答する
	{
		Serial.printf("i:%d EPC:%X\n", i, *p_rEPC);
		boolean exist = replySetDetail_sub(seoj, *p_rEPC, devId);
		if( exist )
		{
			// ある
			// Serial.print("devId ");
			// Serial.print(devId);
			if( devId == 0xff )  // devId = -1 is profile
			{
				// 成功
				// pdcedt = profile[*p_rEPC];  // EPC確保
				detail[detailSize] = *p_rEPC;
				detailSize += 1;
				Serial.printf("node prof: EPC: %x\n", *p_rEPC);
				detail[detailSize] = 0x00;  // 成功したら0x00を返却
				detailSize += 1;
			}else{
				// pdcedt = devices[devId][*p_rEPC];  // EPC確保
				detail[detailSize] = *p_rEPC;
				detailSize += 1;
				Serial.printf("dev obj: EPC: %x\n", *p_rEPC);
				detail[detailSize] = 0x00;  // 成功したら0x00を返却
				detailSize += 1;
			}
		}
		else
		{
			// 失敗したら、受信したデータをそのまま返却
			// Serial.println("nothing");
			detail[detailSize] = *p_rEPC;
			detailSize += 1;
			Serial.printf("no epc: pdcedt: %x\n", *p_rEPC);
			memcpy( &detail[detailSize], p_rPDC, p_rPDC[0] + 1 ); // size = pcd + edt
			detailSize += p_rPDC[0] + 1;
			success = false;  // 失敗フラグを付けておく
		}

		// EPCとPDCを次のステップへ
		p_rEPC += p_rPDC[0] + 2;  // EPC 1Byte とPDC 1Byteと EDT(PDC) Byte分移動
		p_rPDC += p_rPDC[0] + 2;
	}

	// Serial.printf("detailSize: %d\n", detailSize);

	if( esv == EL_SETI ) { return; }  // SetIなら返却なし
	// DEOJが自分のオブジェクトでない場合は破棄（@@ 追加）

	esv = success? EL_SET_RES: EL_SETC_SNA;  // 一つでも失敗したらSETC_SNA、全部OKならSET_RES
	sendDetails(  toip,  tid,  seoj, deoj,  esv, opc, detail,  detailSize);
}


////////////////////////////////////////////////////
/// @brief EOJとEPCを指定したとき、そのプロパティ（EDT）があるかチェックする内部関数
/// @param eoj const byte[]
/// @param epc const byte
/// @param devId[out] byte&: -1:profile, x:devId
/// @return true:無し、false:あり
/// @note replySetDetail_subのサブルーチン、本来はSetPropertyMap[0x9E]の確認をすべきだが、やってない
boolean EL::replySetDetail_sub(const byte eoj[], const byte epc, byte& devId )
{
	// profile
	if( eoj[0] == 0x0e && eoj[1] == 0xf0 && eoj[2] == 0x01 )  // profile object
	{
		byte* pdcedt = profile[epc];
		devId = 0xff;

		if( pdcedt == nullptr ) return false;  // epcがない
		return true;
	}

	// device object
	for (int i = 0; i < deviceCount; i++)  // deojとマッチするdevIdを調べる
	{
		if (eoj[0] == _eojs[i * 3 + 0] && eoj[1] == _eojs[i * 3 + 1])
		{
			devId = i;
			byte* pdcedt = devices[devId][epc];

			if( pdcedt == nullptr ) return false;  // epcがない
			return true;
		}
	}
	return false;  // no eoj, no epc
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// reseiver

////////////////////////////////////////////////////
/// @brief 受信データを読む
/// @param void
/// @return int
/// @note
int EL::parsePacket(void)
{
	return _udp->parsePacket();
}

////////////////////////////////////////////////////
/// @brief 受信データの送信元を取得する
/// @param void
/// @return 受信データの送信元IPアドレス : IPAddress
/// @note
IPAddress EL::remoteIP(void)
{
	return _udp->remoteIP();
}


////////////////////////////////////////////////////
/// @brief 受信データを受け取る
/// @param void
/// @return 受信データサイズ : int
/// @note
int EL::read(void)
{
	_readPacketSize = parsePacket();

	if (_readPacketSize)
	{
		_udp->read(_rBuffer, EL_BUFFER_SIZE); // 受け取った内容読み取り
	}
	return _readPacketSize;
}

////////////////////////////////////////////////////
/// @brief
/// @param
/// @return none
/// @note
// details
void EL::returner(void)
{
	///////////////////////////////////////////////////////////////////
	// 受信パケット解析
	IPAddress remIP = remoteIP();
	byte tid[]  = {_rBuffer[EL_TID], _rBuffer[EL_TID + 1]};
	byte seoj[] = {_rBuffer[EL_SEOJ], _rBuffer[EL_SEOJ + 1], _rBuffer[EL_SEOJ + 2]};
	byte deoj[] = {_rBuffer[EL_DEOJ], _rBuffer[EL_DEOJ + 1], _rBuffer[EL_DEOJ + 2]};
	const byte esv = _rBuffer[EL_ESV];
	const byte epc = _rBuffer[EL_EPC];
	byte* pdcedt = nullptr;
	int devId = 0;

	// 要求されたオブジェクトについて調べる
	if (deoj[0] == 0x0e && deoj[1] == 0xf0)
	{					// 0e f0 xx ならprofile object
		deoj[2] = 0x01; // search等，インスタンス番号が0で送信される時があるので
		pdcedt = profile[epc];
	}
	else
	{
		boolean noDevice = true;
		for (int i = 0; i < deviceCount; i++)  // deojとマッチするdevIdを調べる
		{
			if (deoj[0] == _eojs[i * 3 + 0] && deoj[1] == _eojs[i * 3 + 1])
			{
				devId = i;
				pdcedt = devices[devId][epc];
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
		Serial.println("### SETC ###");
		replySetDetail( remIP );
		break;

	case EL_GET:
		Serial.println("### GET ###");
		replyGetDetail( remIP );
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
			sendOPC1(remIP, tid, deoj, seoj, (esv - 0x10), epc, nullptr);
		}
		break;
		//  INF_REQここまで

	default: // 解釈不可能なESV
		break;
	}
}
// EL処理ここまで
////////////////////////////////////////////////////


////////////////////////////////////////////////////
/// @brief インスタンスの情報を表示
/// @param none
/// @return none
/// @note
void EL::printAll(void)
{
	Serial.println("===================");
	Serial.println("Node profile object");
	profile.printAll();

	Serial.println("--------------");
	Serial.print("Device object (deviceCount: ");
	Serial.print(deviceCount);
	Serial.println(")");

	for( int i = 0; i < deviceCount; i += 1 ) {
		Serial.print("-- devId: ");
		Serial.print(i);
		Serial.printf(" (%02X %02X %02X)\n",  _eojs[i * 3], _eojs[i * 3 + 1], _eojs[i * 3 + 2]);
		devices[i].printAll();
	}
	Serial.println("===================");
}



////////////////////////////////////////////////////
/// @brief byte[] を安全にdeleteするinline関数
/// @param ptr byte[]
/// @return none
/// @note
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
