//////////////////////////////////////////////////////////////////////
/// @file EL.cpp
/// @brief ECHONET Lite protocol for Arduino
/// @author SUGIMURA Hiroshi
/// @date 2013.09.27
/// @details https://github.com/Hiroshi-Sugimura/EL_dev_arduino
//////////////////////////////////////////////////////////////////////
#ifndef GPP
#include <ELOBJ.h>
#include <EL.h>
#else
#include "ELOBJ.h"
#include "EL.h"
#endif

// #define __EL_DEBUG__ 1

////////////////////////////////////////////////////
/// @brief オブジェクトを一つだけサポートする場合のコンストラクタ
/// @param udp WiFiUDP&
/// @param classGroupCode byte class group code
/// @param classCode byte class code
/// @param instanceNumber byte instance number
/// @note eoj0, eoj1, eoj2で一つのオブジェクト
/// 一般照明の例
/// ex. EL(udp, 0x02, 0x90, 0x01);
EL::EL(WiFiUDP &udp, byte classGroupCode, byte classCode, byte instanceNumber)
{
	byte eoj[1][3] = {{classGroupCode, classCode, instanceNumber}};
	commonConstructor(udp, eoj, 1);
}

////////////////////////////////////////////////////
/// @brief オブジェクトを複数サポートする場合のコンストラクタ
/// @param udp WiFiUDP&
/// @param eojs byte [][3]
/// @param count int
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
void EL::commonConstructor(WiFiUDP &udp, byte eojs[][3], int count)
{
	_udp = &udp;

	deviceCount = count;
	_eojs = new byte[deviceCount * 3]; // [count][3], 3 = classGroupCode, classCode, instanceNumber
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

	// 仮
	_mac[0] = 0x00;
	_mac[1] = 0x00;
	_mac[2] = 0x77;
	_mac[3] = 0x00;
	_mac[4] = 0x00;
	_mac[5] = 0x77;

	_tid[0] = 0;
	_tid[1] = 0;

	// profile object
	profile[0x80].setEDT({0x30});																								  // power
	profile[0x82].setEDT({0x01, 0x0a, 0x01, 0x00});																				  // Ver 1.10 (type 1)
	profile[0x83].setEDT({0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}); // identification number
#ifdef ESP32
	String macraw = WiFi.macAddress(); // stringを返すタイプしかない。 M5Stackで WiFi.macAddress(byte) はダメ
	String mac0str = macraw.substring(0, 2);
	String mac1str = macraw.substring(3, 5);
	String mac2str = macraw.substring(6, 8);
	String mac3str = macraw.substring(9, 11);
	String mac4str = macraw.substring(12, 14);
	String mac5str = macraw.substring(15, 17);

	_mac[0] = strtoul(mac0str.c_str(), 0, 16);
	_mac[1] = strtoul(mac1str.c_str(), 0, 16);
	_mac[2] = strtoul(mac2str.c_str(), 0, 16);
	_mac[3] = strtoul(mac3str.c_str(), 0, 16);
	_mac[4] = strtoul(mac4str.c_str(), 0, 16);
	_mac[5] = strtoul(mac5str.c_str(), 0, 16);
#endif
	profile[0x83].setEDT({0xfe, _mac[0], _mac[1], _mac[2], _mac[3], _mac[4], _mac[5], 0x0e, 0xf0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}); // identification number

	profile[0x88].setEDT({0x42});			  // error status
	profile[0x8a].setEDT({0x00, 0x00, 0x77}); // maker KAIT

	profile.SetMyPropertyMap(0x9d, {0x80, 0xd5});																	// inf property map
	profile.SetMyPropertyMap(0x9e, {0x80});																			// set property map
	profile.SetMyPropertyMap(0x9f, {0x80, 0x82, 0x83, 0x88, 0x8a, 0x9d, 0x9e, 0x9f, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7}); // get property map

	// int deviceCount;				   // インスタンス数, d3用
	int classNum = 0;				   // クラス数, d4用
	byte devObjs[deviceCount * 3 + 1]; // maxで確保しておく, d5, d6用, edt
	byte classes[deviceCount * 2 + 1]; // maxで確保しておく, d7用, edt

	int devPosition = 0;
	devObjs[devPosition] = deviceCount;
	devPosition += 0;

	int classPosition = 0;
	for (int di = 0; di < deviceCount; di++)
	{
		// デバイスのD6登録
		devObjs[devPosition * 3 + 1] = eojs[di][0];
		devObjs[devPosition * 3 + 2] = eojs[di][1];
		devObjs[devPosition * 3 + 3] = eojs[di][2];
		devPosition += 1;

		// classのD7登録
		bool exist = false;
		for (int ci = 0; ci < classNum; ci += 1)
		{
			if (classes[ci * 2 + 1] == eojs[di][0] && classes[ci * 2 + 2] == eojs[di][1])
			{
				exist = true;
			}
		}
		if (!exist)
		{
			classes[classNum * 2 + 1] = eojs[di][0];
			classes[classNum * 2 + 2] = eojs[di][1];
			classNum += 1;
		}
	}
	classes[0] = classNum;

	profile[0xd3].setEDT({0x00, 0x00, byte(deviceCount)}); // total instance number、デバイスオブジェクトの数
	profile[0xd4].setEDT({0x00, byte(classNum + 1)});	   // total class number、デバイスクラス＋ノードプロファイルクラス

	profile[0xd5].setEDT(devObjs, deviceCount * 3 + 1); // obj list
	profile[0xd6].setEDT(devObjs, deviceCount * 3 + 1); // obj list
	profile[0xd7].setEDT(classes, classNum * 2 + 1);	// class list

	// device object
	for (unsigned char i = 0; i < deviceCount; i++)
	{
		devices[i][0x80].setEDT({0x30});				   // power
		devices[i][0x81].setEDT({0x00});				   // position
		devices[i][0x82].setEDT({0x00, 0x00, 0x4b, 0x00}); // release K

		devices[i][0x83].setEDT({0xfe, _mac[0], _mac[1], _mac[2], _mac[3], _mac[4], _mac[5], _eojs[i * 3], _eojs[i * 3 + 1], _eojs[i * 3 + 2], 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, static_cast<unsigned char>(i + 1)}); // identification number

		devices[i][0x88].setEDT({0x42});			 // error status
		devices[i][0x8a].setEDT({0x00, 0x00, 0x77}); // maker KAIT

		devices[i].SetMyPropertyMap(0x9d, {0x80, 0xd6});										   // inf property map
		devices[i].SetMyPropertyMap(0x9e, {0xe0});												   // set property map
		devices[i].SetMyPropertyMap(0x9f, {0x80, 0x81, 0x82, 0x83, 0x88, 0x8a, 0x9d, 0x9e, 0x9f}); // get property map
#ifdef __EL_DEBUG__
		devices[i].printAll();
#endif
	}
}

////////////////////////////////////////////////////
/// @brief TIDの自動インクリメント、オーバーフロー対策
void EL::tidAutoIncrement(void)
{
	if (_tid[0] == 0xff && _tid[1] == 0xff)
	{
		_tid[0] = 0;
		_tid[1] = 0;
	}
	else if (_tid[1] == 0xff)
	{
		_tid[0] += 1;
		_tid[1] = 0;
	}
	else
	{
		_tid[1] += 1;
	}
}

////////////////////////////////////////////////////
/// @brief 通信の開始、受信開始
void EL::begin(void)
{
#ifdef __EL_DEBUG__
	Serial.println("=========== EL.begin");
	Serial.printf("deviceCount: %d", deviceCount);
	Serial.println();

	for (int i = 0; i < deviceCount; i++)
	{
		Serial.printf("eojs[%d] = %02x%02x%02x", i, *(_eojs + i * 3 + 0), *(_eojs + i * 3 + 1), *(_eojs + i * 3 + 2));
		Serial.println();
	}
#endif

	// udp
	if (_udp->begin(EL_PORT))
	{
#ifdef __EL_DEBUG__
		Serial.println("EL.udp.begin successful.");
#endif
	}
	else
	{
#ifdef __EL_DEBUG__
		Serial.println("Reseiver udp.begin failed."); // localPort
#endif
	}

	if (_udp->beginMulticast(_multi, EL_PORT))
	{
#ifdef __EL_DEBUG__
		Serial.println("EL.udp.beginMulticast successful.");
#endif
	}
	else
	{
#ifdef __EL_DEBUG__
		Serial.println("Reseiver EL.udp.beginMulticast failed."); // localPort
#endif
	}

	// 接続ネットワークのブロードキャストアドレスに更新
	_broad = IPAddress(ip[0], ip[1], ip[2], 255);

	userfunc = null; // ユーザ処理のコールバックなし
}

////////////////////////////////////////////////////
/// @brief 通信の開始、受信開始、ユーザ関数受付
void EL::begin(ELCallback cb)
{
#ifdef __EL_DEBUG__
	Serial.println("=========== EL.begin");
	Serial.printf("deviceCount: %d", deviceCount);
	Serial.println();

	for (int i = 0; i < deviceCount; i++)
	{
		Serial.printf("eojs[%d] = %02x%02x%02x", i, *(_eojs + i * 3 + 0), *(_eojs + i * 3 + 1), *(_eojs + i * 3 + 2));
		Serial.println();
	}
#endif

	// udp
	if (_udp->begin(EL_PORT))
	{
#ifdef __EL_DEBUG__
		Serial.println("EL.udp.begin successful.");
#endif
	}
	else
	{
#ifdef __EL_DEBUG__
		Serial.println("Reseiver udp.begin failed."); // localPort
#endif
	}

	if (_udp->beginMulticast(_multi, EL_PORT))
	{
#ifdef __EL_DEBUG__
		Serial.println("EL.udp.beginMulticast successful.");
#endif
	}
	else
	{
#ifdef __EL_DEBUG__
		Serial.println("Reseiver EL.udp.beginMulticast failed."); // localPort
#endif
	}

	// 接続ネットワークのブロードキャストアドレスに更新
	_broad = IPAddress(ip[0], ip[1], ip[2], 255);

	// ユーザ処理のコールバック登録
	userfunc = cb;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief EPCの値を変更する, eojが1個の場合（複数の場合は0番に相当）
/// @param epc const byte
/// @param pdcedt byte[]
/// @note pdcedtなので、pdcは自分で計算することに注意
void EL::update(const byte epc, byte pdcedt[])
{
	devices[0].SetPDCEDT(epc, pdcedt); // power
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief EPCの値を取得する, eojが1個の場合（複数の場合は0番に相当）
/// @param epc const byte
/// @return edt byte*
byte *EL::at(const byte epc)
{
	return devices[0].GetPDCEDT(epc);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief EPCの値を変更する, 複数の場合
/// @param devId const int, コンストラクタで渡した順番に相当
/// @param epc const byte
/// @param pdcedt byte []
void EL::update(const int devId, const byte epc, byte pdcedt[])
{
	if (devId < deviceCount)
		devices[devId].SetPDCEDT(epc, pdcedt); // power
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief EPCの値を取得する, 複数の場合
/// @param devId const int, コンストラクタで渡した順番に相当
/// @param epc const byte
/// @return edt or nullptr
byte *EL::at(const int devId, const byte epc)
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
/// @param sBuffer byte[]
/// @param size int
/// @note !!deprecated!! 非推奨機能なので、本番環境では利用しないように
void EL::sendBroad(byte sBuffer[], int size)
{
	if (_udp->beginPacket(_broad, EL_PORT))
	{
#ifdef __EL_DEBUG__
		// Serial.println("UDP beginPacket(B) Successful.");
#endif
		_udp->write(sBuffer, size);
	}

	if (_udp->endPacket())
	{
#ifdef __EL_DEBUG__
		// Serial.println("UDP endPacket(B) Successful.");
#endif
	}
	else
	{
#ifdef __EL_DEBUG__
		Serial.println("UDP endPacket(B) failed.");
#endif
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief マルチキャストによる送信(default: 192.168.1.255)
/// @param sBuffer byte []
/// @param size int
/// バージョンによってはブロードキャストによる送信の場合がある。
void EL::sendMulti(byte sBuffer[], int size)
{
	// sendBroad(sBuffer, size);

	if (_udp->beginMulticastPacket())
	{
#ifdef __EL_DEBUG__
		// Serial.println("UDP beginPacket(B) Successful.");
#endif
		_udp->write(sBuffer, size);
	}

	if (_udp->endPacket())
	{
#ifdef __EL_DEBUG__
		// Serial.println("UDP endPacket(B) Successful.");
#endif
	}
	else
	{
#ifdef __EL_DEBUG__
		Serial.println("UDP endPacket(M) failed.");
#endif
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief OPC一個用のマルチキャスト送信、TID指定有り
/// @param tid const byte[]
/// @param seoj const byte[]
/// @param deoj const byte[]
/// @param esv const byte
/// @param epc const byte
/// @param pdcedt const byte[]
void EL::sendMultiOPC1(const byte tid[], const byte seoj[], const byte deoj[], const byte esv, const byte epc, const byte pdcedt[])
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

#ifdef __EL_DEBUG__
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
/// @param seoj const byte[]
/// @param deoj const byte[]
/// @param esv const byte
/// @param epc const byte
/// @param pdcedt const byte[]
void EL::sendMultiOPC1(const byte seoj[], const byte deoj[], const byte esv, const byte epc, const byte pdcedt[])
{
	sendMultiOPC1(_tid, seoj, deoj, esv, epc, pdcedt);
	tidAutoIncrement();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief OPC一個用のマルチキャスト送信、seoj省略（0番）、TID自動
/// @param deoj const byte[]
/// @param esv const byte
/// @param epc const byte
/// @param pdcedt const byte[]
void EL::sendMultiOPC1(const byte deoj[], const byte esv, const byte epc, const byte pdcedt[])
{
	byte eoj[3] = {_eojs[0], _eojs[1], _eojs[2]};
	sendMultiOPC1(_tid, eoj, deoj, esv, epc, pdcedt);
	tidAutoIncrement();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief OPC一個用のマルチキャスト送信、seojの代わりにIDで指定、TID自動
/// @param devId const byte int
/// @param deoj const byte[]
/// @param esv const byte
/// @param epc const byte
/// @param pdcedt const byte[]
/// @note recommendation 推奨
void EL::sendMultiOPC1ID(const int devId, const byte deoj[], const byte esv, const byte epc, const byte pdcedt[])
{
	if (devId < deviceCount)
	{
		byte eoj[3] = {_eojs[devId * 3 + 0], _eojs[devId * 3 + 1], _eojs[devId * 3 + 2]};
		sendMultiOPC1(_tid, eoj, deoj, esv, epc, pdcedt);
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
#ifdef __EL_DEBUG__
		// Serial.println("UDP beginPacket Successful.");
#endif
		_udp->write(sBuffer, size);
	}
	else
	{
#ifdef __EL_DEBUG__
		Serial.println("UDP beginPacket failed.");
#endif
	}

	if (_udp->endPacket())
	{
#ifdef __EL_DEBUG__
		// Serial.println("UDP endPacket Successful.");
#endif
	}
	else
	{
#ifdef __EL_DEBUG__
		Serial.println("UDP endPacket failed.");
#endif
	}
}

////////////////////////////////////////////////////
/// @brief OPC1指定による送信(SEOJも指定する，ほぼ内部関数)
/// @param toip const IPAddress
/// @param tid const byte[]
/// @param seoj const byte[]
/// @param deoj const byte[]
/// @param esv const byte
/// @param epc const byte
/// @param pdcedt const byte[]
void EL::sendOPC1(const IPAddress toip, const byte tid[], const byte seoj[], const byte deoj[], const byte esv, const byte epc, const byte pdcedt[])
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

#ifdef __EL_DEBUG__
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
/// @brief OPC1指定による送信(SEOJも指定する，ほぼ内部関数)
/// @param toip const IPAddress
/// @param seoj const byte[]
/// @param deoj const byte[]
/// @param esv const byte
/// @param epc const byte
/// @param pdcedt const byte[]
void EL::sendOPC1(const IPAddress toip, const byte seoj[], const byte deoj[], const byte esv, const byte epc, const byte pdcedt[])
{
	sendOPC1(toip, _tid, seoj, deoj, esv, epc, pdcedt);
	tidAutoIncrement();
}

////////////////////////////////////////////////////
/// @brief OPC1指定による送信(SEOJは初期化時に指定したものを使う)
/// @param toip const IPAddress
/// @param deoj const byte[]
/// @param esv const byte
/// @param epc const byte
/// @param pdcedt const byte[]
void EL::sendOPC1(const IPAddress toip, const byte deoj[], const byte esv, const byte epc, const byte pdcedt[])
{
	byte eoj[3] = {_eojs[0], _eojs[1], _eojs[2]};
	sendOPC1(toip, _tid, eoj, deoj, esv, epc, pdcedt);
	tidAutoIncrement();
}

////////////////////////////////////////////////////
/// @brief
/// @param toip const IPAddress
/// @param devId const int
/// @param deoj const byte[]
/// @param esv const byte
/// @param epc const byte
/// @param pdcedt const byte[]
/// @note recommended 推奨
void EL::sendOPC1ID(const IPAddress toip, const int devId, const byte deoj[], const byte esv, const byte epc, const byte pdcedt[])
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
/// @param opc const byte
/// @param detail const byte[N]: {EPC, PDC, EDT[x]}[y]
/// @param detailSize const byte: size of detail N
void EL::sendDetails(const IPAddress toip, const byte tid[], const byte seoj[], const byte deoj[], const byte esv, const byte opc, const byte detail[], const byte detailSize)
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
	_sBuffer[EL_OPC] = opc;

	if (detail != nullptr)
	{
		memcpy(&_sBuffer[EL_EPC], detail, detailSize); // size = pcd + edt
		_sendPacketSize = EL_EPC + detailSize;
		send(toip, _sBuffer, _sendPacketSize);
	}
	else
	{
#ifdef __EL_DEBUG__
		Serial.println("Error: EL::sendDetails, detail is nullptr.");
#endif
	}
}

// ELの返信用、典型的なOPC一個でやる．TIDを併せて返信しないといけないため
// void EL::replyOPC1(const IPAddress toip, const unsigned short tid, const byte* seoj, const byte* deoj, const byte esv, const byte epc, const byte* edt) {}

////////////////////////////////////////////////////
/// @brief Getに対して複数OPCにも対応して返答する内部関数
/// @param toip const IPAddress
void EL::replyGetDetail(const IPAddress toip)
{
	byte tid[] = {_rBuffer[EL_TID], _rBuffer[EL_TID + 1]};
	byte seoj[] = {_rBuffer[EL_DEOJ], _rBuffer[EL_DEOJ + 1], _rBuffer[EL_DEOJ + 2]}; // DEOJがreplyではSEOJになる
	byte deoj[] = {_rBuffer[EL_SEOJ], _rBuffer[EL_SEOJ + 1], _rBuffer[EL_SEOJ + 2]}; // SEOJがreplyではDEOJになる
	byte esv = _rBuffer[EL_ESV];
	byte opc = _rBuffer[EL_OPC];

	// 送信用
	boolean success = true;
	byte detail[1500];	 // EPC,PDC,EDT[n]
	byte detailSize = 0; // data size

	// rBuffer走査用
	byte *p_rEPC = &_rBuffer[EL_EPC]; // 初期EPCポインタ
	// この関数を呼ばれるのはGETの場合なので、 PDC:0x00, EDTなしに決まっている。
	// 従って、EPCだけを焦点とする

	// temp
	byte *pdcedt = nullptr; // pdc edt
	byte devId;

	for (byte i = 0; i < opc; i += 1, p_rEPC += 2) // OPC個数のEPCに回答する
	{
#ifdef __EL_DEBUG__
		Serial.printf("i:%d EPC:%X\n", i, *p_rEPC);
#endif
		boolean exist = replyGetDetail_sub(seoj, *p_rEPC, devId);
		if (exist)
		{
			// ある
#ifdef __EL_DEBUG__
			Serial.printf("devId: %x\n", devId);
#endif
			if (devId == 0xff) // devId = 0xff is profile
			{
				pdcedt = profile[*p_rEPC]; // EPCに対応するPDCEDT確保
				// Serial.printf("node prof: pdcedt: %x %x %x\n", pdcedt[0], pdcedt[1], pdcedt[2]);
				detail[detailSize] = *p_rEPC; // EPCに対して
				detailSize += 1;
				// PDCとEDTを設定
				memcpy(&detail[detailSize], pdcedt, pdcedt[0] + 1); // size = pcd + edt
				detailSize += pdcedt[0] + 1;
			}
			else
			{
				pdcedt = devices[devId][*p_rEPC]; // EPCに対応するPDCEDT確保
				// Serial.printf("dev obj: pdcedt: %x %x %x\n", pdcedt[0], pdcedt[1], pdcedt[2]);
				detail[detailSize] = *p_rEPC; // EPCに対して
				detailSize += 1;
				// PDCとEDT確保
				memcpy(&detail[detailSize], pdcedt, pdcedt[0] + 1); // size = pcd + edt
				detailSize += pdcedt[0] + 1;
			}
		}
		else
		{
			// ない
			if (devId == 0xfe)
			{ // そもそもDEOJが自分のオブジェクトでない場合は無視（@@ 追加）
				return;
			}
			else
			{ // DEOJはあるが、EPCがない
				// Serial.println("nothing");
				memcpy(&detail[detailSize], pdcedt, pdcedt[0] + 1); // size = pcd + edt
				detail[detailSize] = *p_rEPC;
				detailSize += 1;
				detail[detailSize] = 0x00;
				detailSize += 1;
				success = false;
			}
		}
	}

	// Serial.printf("detailSize: %d\n", detailSize);

	esv = success ? EL_GET_RES : EL_GET_SNA; // 一つでも失敗したらGET_SNA、全部OKならGET_RES
	sendDetails(toip, tid, seoj, deoj, esv, opc, detail, detailSize);
}

////////////////////////////////////////////////////
/// @brief EOJとEPCを指定したとき、そのプロパティ（EDT）があるかチェックする内部関数
/// @param eoj const byte[]
/// @param epc const byte
/// @param devId[out] byte&: -1:profile, x:devId
/// @return true:無し、false:あり
/// @note replyGetDetailのサブルーチン、GetPropertyMapを参照しなくても、基本的に持っているPeopertyはGet可能なのでMapチェックしなくてよい
boolean EL::replyGetDetail_sub(const byte eoj[], const byte epc, byte &devId)
{
	devId = 0xfe;											// 0xfe はOJB無しとする（） // @@@ 実際は別の方法でOBJ無しとしないとバグ
	if (eoj[0] == 0x0e && eoj[1] == 0xf0 && eoj[2] == 0x01) // profile object
	{
		devId = 0xff; // devId = -1 は node profileとする。（この方式は後で変更）
		byte *pdcedt = profile[epc];

		if (pdcedt == nullptr)
			return false; // epcがない
		return true;
	}

	// device object
	for (int i = 0; i < deviceCount; i++) // deojとマッチするdevIdを調べる
	{
		if (eoj[0] == _eojs[i * 3 + 0] && eoj[1] == _eojs[i * 3 + 1])
		{
			devId = i;
			byte *pdcedt = devices[devId][epc];

			if (pdcedt == nullptr)
				return false; // epcがない
			return true;
		}
	}
	return false; // no eoj, no epc
}

////////////////////////////////////////////////////
/// @brief Setに対して複数OPCにも対応して返答する内部関数
/// @param toip const IPAddress
/// @note EPC毎の設定値に関して基本はノーチェックなので注意すべし
/// EPC毎の設定値チェックや、INF処理に関しては下記の replySetDetail_sub にて実施
/// SET_RESはEDT入ってない
void EL::replySetDetail(const IPAddress toip)
{
	byte tid[] = {_rBuffer[EL_TID], _rBuffer[EL_TID + 1]};
	byte seoj[] = {_rBuffer[EL_DEOJ], _rBuffer[EL_DEOJ + 1], _rBuffer[EL_DEOJ + 2]}; // DEOJがreplyではSEOJになる
	byte deoj[] = {_rBuffer[EL_SEOJ], _rBuffer[EL_SEOJ + 1], _rBuffer[EL_SEOJ + 2]}; // SEOJがreplyではDEOJになる
	byte esv = _rBuffer[EL_ESV];
	byte opc = _rBuffer[EL_OPC];

	// 送信用
	boolean success = true;
	byte detail[1500];	 // EPC,PDC,EDT[n]
	byte detailSize = 0; // data size

	// rBuffer走査用
	byte *p_rEPC = &_rBuffer[EL_EPC]; // 初期EPCポインタ
	// この関数を呼ばれるのはSETの場合であるので、 PDCを見ながらEDT分をスキップしていく
	byte *p_rPDC = &_rBuffer[EL_EPC + 1]; // 初期PDCポインタ

	// temp
	byte *pdcedt = nullptr; // pdc edt
	byte devId;

	for (byte i = 0; i < opc; i += 1) // OPC個数のEPCに回答する
	{
#ifdef __EL_DEBUG__
		Serial.printf("i:%d EPC:%X\n", i, *p_rEPC);
#endif
		boolean exist = replySetDetail_sub(seoj, *p_rEPC, devId);
		if (exist)
		{
			// ある
			// Serial.print("devId ");
			// Serial.print(devId);
			if (devId == 0xff) // devId = -1 is profile
			{
				// 成功
				// pdcedt = profile[*p_rEPC];  // EPC確保
				detail[detailSize] = *p_rEPC;
				detailSize += 1;
#ifdef __EL_DEBUG__
				Serial.printf("node prof: EPC: %x\n", *p_rEPC);
#endif
				detail[detailSize] = 0x00; // 成功したら0x00を返却
				detailSize += 1;
			}
			else
			{
				// pdcedt = devices[devId][*p_rEPC];  // EPC確保
				detail[detailSize] = *p_rEPC;
				detailSize += 1;
#ifdef __EL_DEBUG__
				Serial.printf("dev obj: EPC: %x\n", *p_rEPC);
#endif
				detail[detailSize] = 0x00; // 成功したら0x00を返却
				detailSize += 1;
			}
		}
		else
		{
			// 失敗したら、受信したデータをそのまま返却
			// Serial.println("nothing");
			detail[detailSize] = *p_rEPC;
			detailSize += 1;
#ifdef __EL_DEBUG__
			Serial.printf("no epc: pdcedt: %x\n", *p_rEPC);
#endif
			memcpy(&detail[detailSize], p_rPDC, p_rPDC[0] + 1); // size = pcd + edt
			detailSize += p_rPDC[0] + 1;
			success = false; // 失敗フラグを付けておく
		}

		// EPCとPDCを次のステップへ
		p_rEPC += p_rPDC[0] + 2; // EPC 1Byte とPDC 1Byteと EDT(PDC) Byte分移動
		p_rPDC += p_rPDC[0] + 2;
	}

	// Serial.printf("detailSize: %d\n", detailSize);

	if (esv == EL_SETI)
	{
		return;
	} // SetIなら返却なし
	// DEOJが自分のオブジェクトでない場合は破棄（@@ 追加）

	esv = success ? EL_SET_RES : EL_SETC_SNA; // 一つでも失敗したらSETC_SNA、全部OKならSET_RES
	sendDetails(toip, tid, seoj, deoj, esv, opc, detail, detailSize);
}

////////////////////////////////////////////////////
/// @brief EOJとEPCを指定したとき、そのプロパティ（EDT）があるかチェックする内部関数
/// @param eoj const byte[]
/// @param epc const byte
/// @param devId[out] byte&: -1:profile, x:devId
/// @return true:無し、false:あり
/// @note replySetDetail_subのサブルーチン、本来はSetPropertyMap[0x9E]の確認をすべきだが、やってない
boolean EL::replySetDetail_sub(const byte eoj[], const byte epc, byte &devId)
{
	// profile
	if (eoj[0] == 0x0e && eoj[1] == 0xf0 && eoj[2] == 0x01) // profile object
	{
		byte *pdcedt = profile[epc];
		devId = 0xff;

		if (pdcedt == nullptr)
			return false; // epcがない
		return true;
	}

	// device object
	for (int i = 0; i < deviceCount; i++) // deojとマッチするdevIdを調べる
	{
		if (eoj[0] == _eojs[i * 3 + 0] && eoj[1] == _eojs[i * 3 + 1])
		{
			devId = i;
			byte *pdcedt = devices[devId][epc];

			if (pdcedt == nullptr)
				return false; // epcがない
			return true;
		}
	}
	return false; // no eoj, no epc
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// reseiver

////////////////////////////////////////////////////
/// @brief 受信データを読む
/// @return int
int EL::parsePacket(void)
{
	return _udp->parsePacket();
}

////////////////////////////////////////////////////
/// @brief 受信データの送信元を取得する
/// @return 受信データの送信元IPアドレス : IPAddress
IPAddress EL::remoteIP(void)
{
	return _udp->remoteIP();
}

////////////////////////////////////////////////////
/// @brief 受信データを受け取る
/// @return 受信データサイズ : int
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
/// @brief 受信データを処理する。EL処理でupdateしたら呼ぶ
void EL::returner(void)
{
	///////////////////////////////////////////////////////////////////
	// 受信パケット解析
	IPAddress remIP = remoteIP();
	byte tid[] = {_rBuffer[EL_TID], _rBuffer[EL_TID + 1]};
	byte seoj[] = {_rBuffer[EL_SEOJ], _rBuffer[EL_SEOJ + 1], _rBuffer[EL_SEOJ + 2]};
	byte deoj[] = {_rBuffer[EL_DEOJ], _rBuffer[EL_DEOJ + 1], _rBuffer[EL_DEOJ + 2]};
	const byte esv = _rBuffer[EL_ESV];
	const byte epc = _rBuffer[EL_EPC];
	byte *pdcedt = nullptr;
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
		for (int i = 0; i < deviceCount; i++) // deojとマッチするdevIdを調べる
		{
			if (deoj[0] == _eojs[i * 3 + 0] && deoj[1] == _eojs[i * 3 + 1])
			{
				devId = i;
				pdcedt = devices[devId][epc];
				noDevice = false;
#ifdef __EL_DEBUG__
				Serial.printf("pdcedt: %p", pdcedt);
				Serial.println();
#endif
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
#ifdef __EL_DEBUG__
		Serial.println("### SETC ###");
#endif
		replySetDetail(remIP);
		break;

	case EL_GET:
#ifdef __EL_DEBUG__
		Serial.println("### GET ###");
#endif
		replyGetDetail(remIP);
		break;

		// ユニキャストへの返信ここまで，INFはマルチキャスト
	case EL_INF_REQ:
#ifdef __EL_DEBUG__
		Serial.print("INF_REQ: ");
		Serial.println(epc, HEX);
#endif
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

void EL::recvProcess(void)
{
	// パケット貰ったらやる
	int packetSize = 0; // 受信データ量

	// -----------------------------------
	// ESVがSETとかGETとかで動作をかえる
	if (0 != (packetSize = echo.read())) // 0!=はなくてもよいが，Warning出るのでつけとく
	{									 // 受け取った内容読み取り，あったら中へ
		///////////////////////////////////////////////////////////////////
		// 受信パケット解析
		IPAddress remIP = remoteIP();

		// 受信データをまずは意味づけしておくとらくかも
		byte tid[]  = {_rBuffer[EL_TID],  _rBuffer[EL_TID + 1]};
		byte seoj[] = {_rBuffer[EL_SEOJ], _rBuffer[EL_SEOJ + 1], _rBuffer[EL_SEOJ + 2]};
		byte deoj[] = {_rBuffer[EL_DEOJ], _rBuffer[EL_DEOJ + 1], _rBuffer[EL_DEOJ + 2]};
		byte esv = echo._rBuffer[EL_ESV];
		byte opc = echo._rBuffer[EL_OPC];

		byte epc = echo._rBuffer[EL_EPC];
		byte pdc = echo._rBuffer[EL_PDC];

		byte *edt = &echo._rBuffer[EL_EDT];
		PDCEDT pdcedt = &echo._rBuffer[EL_PDC];

		// 返却用のpdcedt
		byte ret_details[1024]; // 1024で良いかはあとで考える
		byte ret_now = 0;

		// OPCで処理
		boolean success = true;
		for (int o = 0; o < opc; o += 1)
		{
			if (userfunc(tid, seoj, deoj, esv, opc, epc, pdcedt)) // 成功
			{
				switch (esv)
				{
				case EL_SET: // SET成功したらedtは0
					*(ret_details + ret_now) = epc;
					*(ret_details + ret_now + 1) = 0;
					break;
				case EL_GET: // GET成功したらedtはその数値
					*(ret_details + ret_now) = epc;
					*(ret_details + ret_now + 1) = pdc[0];
					memcpy( (ret_details + ret_now + 2), &pdc[1], pdc[0]);
					break;
				}
			}
			else // 失敗
			{
				// どこかで失敗したら、失敗を返却
				success = false;
				switch (esv)
				{
				case EL_SET: // SET失敗
					break;
				case EL_GET: // GET失敗
					break;
				}
			}

			// 次のEPC,PDC,EDTへ
			switch ()
			{
			case EL_SET: // SET, epc,pdc,edt数 進める
				edt += 2 + pdcedt[0];
				pdcedt += 2 + pdcedt[0];
				break;
			case EL_GET: // GET, epc,pdcの２byte進める
				edt += 2;
				pdcedt += 2;
				break;
			}
		}

		// OPCでまとめて返信
		switch (esv)
		{
		case: // SET返却
			break;
		case: // GET返却
			break;
		}
	}
}

// EL処理ここまで
////////////////////////////////////////////////////

////////////////////////////////////////////////////
/// @brief インスタンスの情報を表示
void EL::printAll(void)
{
#ifdef Arduino_h
	Serial.println("===================");
	Serial.println("Node profile object");
#else
	cout << "=================== Node profile object" << endl;
#endif
	profile.printAll();

#ifdef Arduino_h
	Serial.println("--------------");
	Serial.print("Device object (deviceCount: ");
	Serial.print(deviceCount);
	Serial.println(")");
#else
	cout << "-------------- Device object" << endl;
#endif

	for (int i = 0; i < deviceCount; i += 1)
	{
#ifdef Arduino_h
		Serial.print("-- devId: ");
		Serial.print(i);
		Serial.printf(" (%02X %02X %02X)\n", _eojs[i * 3], _eojs[i * 3 + 1], _eojs[i * 3 + 2]);
#else
		cout << "-------------- DevId: " << i << endl;
#endif
		devices[i].printAll();
	}

#ifdef Arduino_h
	Serial.println("===================");
#endif
}

////////////////////////////////////////////////////
/// @brief byte[] を安全にdeleteするinline関数
/// @param ptr byte[]
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
