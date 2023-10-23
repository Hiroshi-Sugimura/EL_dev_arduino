//////////////////////////////////////////////////////////////////////
/// @file EL.h
/// @brief ECHONET Lite protocol for Arduino
/// @author SUGIMURA Hiroshi
/// @date 2013.09.27
/// @details https://github.com/Hiroshi-Sugimura/EL_dev_arduino
//////////////////////////////////////////////////////////////////////
#ifndef __EL_H__
#define __EL_H__
#pragma once

// auto config
#ifndef GPP
// arduino
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUDP.h>
#include <ELOBJ.h>

#else
// g++
typedef unsigned char byte;
typedef bool boolean;
#include <iostream>
#include "WiFiUDP.h"
#include "ELOBJ.h"
using std::cout;
using std::dec;
using std::endl;
using std::hex;
using std::move;
#endif

// defined
#define EL_PORT 3610		///< ECHONET Lite port
#define EL_EHD1 0			///< EHD1
#define EL_EHD2 1			///< EHD2
#define EL_TID 2			///< TID 2 byte
#define EL_SEOJ 4			///< SEOJ 3 byte
#define EL_DEOJ 7			///< DEOJ 3 byte
#define EL_ESV 10			///< ESV
#define EL_OPC 11			///< OPC
#define EL_EPC 12			///< EPC
#define EL_PDC 13			///< PDC
#define EL_EDT 14			///< EDT n byte
#define EL_SETI_SNA 0x50	///< SETI_SNA
#define EL_SETC_SNA 0x51	///< SETC_SNA
#define EL_GET_SNA 0x52		///< GET_SNA
#define EL_INF_SNA 0x53		///< INF_SNA
#define EL_SETGET_SNA 0x5e	///< SETGET_SNA
#define EL_SETI 0x60		///< SETI
#define EL_SETC 0x61		///< SETC
#define EL_GET 0x62			///< GET
#define EL_INF_REQ 0x63		///< INF_REQ
#define EL_SETGET 0x6e		///< SETGET
#define EL_SET_RES 0x71		///< SET_RES
#define EL_GET_RES 0x72		///< GET_RES
#define EL_INF 0x73			///< INF
#define EL_INFC 0x74		///< INFC
#define EL_INFC_RES 0x7a	///< INFC_RES
#define EL_SETGET_RES 0x7e	///< SETGET_RES
#define EL_BUFFER_SIZE 1500 ///< BUFFER_SIZE
#define EL_MINIMUM_FRAME 13 ///< MINIMUM_FRAME

// Device Object
// センサ関連機器
#define EL_GasLeakSensor 0x00, 0x01				 ///< ガス漏れセンサ
#define EL_CrimePreventionSensor 0x00, 0x02		 ///< 防犯センサ
#define EL_EmergencyButton 0x00, 0x03			 ///< 非常ボタン
#define EL_FirstAidSensor 0x00, 0x04			 ///< 救急用センサ
#define EL_EarthquakeSensor 0x00, 0x05			 ///< 地震センサ
#define EL_ElectricLeakSensor 0x00, 0x06		 ///< 漏電センサ
#define EL_HumanDetectionSensor 0x00, 0x07		 ///< 人体検知センサ
#define EL_VisitorSensor 0x00, 0x08				 ///< 来客センサ
#define EL_CallSensor 0x00, 0x09				 ///< 呼び出しセンサ
#define EL_CondensationSensor 0x00, 0x0A		 ///< 結露センサ
#define EL_AirPollutionSensor 0x00, 0x0B		 ///< 空気汚染センサ
#define EL_OxygenSensor 0x00, 0x0C				 ///< 酸素センサ
#define EL_IlluminanceSensor 0x00, 0x0D			 ///< 照度センサ
#define EL_SoundSensor 0x00, 0x0E				 ///< 音センサ
#define EL_MailingSensor 0x00, 0x0F				 ///< 投函センサ
#define EL_WeightSensor 0x00, 0x10				 ///< 重荷センサ
#define EL_TemperatureSensor 0x00, 0x11			 ///< 温度センサ
#define EL_HumiditySensor 0x00, 0x12			 ///< 湿度センサ
#define EL_RainSensor 0x00, 0x13				 ///< 雨センサ
#define EL_WaterLevelSensor 0x00, 0x14			 ///< 水位センサ
#define EL_BathWaterLevelSensor 0x00, 0x15		 ///< 風呂水位センサ
#define EL_BathHeatingStatusSensor 0x00, 0x16	 ///< 風呂沸き上がりセンサ
#define EL_WaterLeakSensor 0x00, 0x17			 ///< 水漏れセンサ
#define EL_WaterOverflowSensor 0x00, 0x18		 ///< 水あふれセンサ
#define EL_FireSensor 0x00, 0x19				 ///< 火災センサ
#define EL_CigaretteSmokeSensor 0x00, 0x1A		 ///< タバコ煙センサ
#define EL_CO2Sensor 0x00, 0x1B					 ///< CO2センサ
#define EL_GasSensor 0x00, 0x1C					 ///< ガスセンサ
#define EL_VOCSensor 0x00, 0x1D					 ///< VOCセンサ
#define EL_DifferentialPressureSensor 0x00, 0x1E ///< 差圧センサ
#define EL_AirSpeedSensor 0x00, 0x1F			 ///< 風速センサ
#define EL_OdorSensor 0x00, 0x20				 ///< 臭いセンサ
#define EL_FlameSensor 0x00, 0x21				 ///< 炎センサ
#define EL_ElectricEnergySensor 0x00, 0x22		 ///< 電力量センサ
#define EL_CurrentValueSensor 0x00, 0x23		 ///< 電流値センサ
#define EL_WaterFlowRateSensor 0x00, 0x25		 ///< 水流量センサ
#define EL_MicromotionSensor 0x00, 0x26			 ///< 微動センサ
#define EL_PassageSensor 0x00, 0x27				 ///< 通過センサ
#define EL_BedPresenceSensor 0x00, 0x28			 ///< 在床センサ
#define EL_OpenCloseSensor 0x00, 0x29			 ///< 開閉センサ
#define EL_ActivityAmountSensor 0x00, 0x2A		 ///< 活動量センサ
#define EL_HumanBodyLocationSensor 0x00, 0x2B	 ///< 人体位置センサ
#define EL_SnowSensor 0x00, 0x2C				 ///< 雪センサ
// 空調関連機器
#define EL_HomeAirConditioner 0x01, 0x30							 ///< 家庭用エアコン
#define EL_VentilationFan 0x01, 0x32								 ///< 換気扇
#define EL_AirConditionerVentilationFan 0x01, 0x34					 ///< 空調換気扇
#define EL_AirCleaner 0x01, 0x35									 ///< 空気清浄器
#define EL_Humidifier 0x01, 0x39									 ///< 加湿器
#define EL_ElectricHeater 0x01, 0x42								 ///< 電気暖房機
#define EL_FanHeater 0x01, 0x43										 ///< ファンヒータ
#define EL_PackageTypeCommercialAirConditionerIndoorUnit 0x01, 0x56	 ///< 業務用パッケージエアコン室内機
#define EL_PackageTypeCommercialAirConditionerOutdoorUnit 0x01, 0x57 ///< 業務用パッケージエアコン室外機
// 住宅・設備関連機器
#define EL_ElectricallyOperatedShade 0x02, 0x60			///< 電動ブラインド・日よけ
#define EL_ElectricShutter 0x02, 0x61					///< 電動シャッター
#define EL_ElectricStormWindow 0x02, 0x63				///< 電動雨戸・シャッター
#define EL_Sprinkler 0x02, 0x67							///< 散水器(庭用)
#define EL_ElectricWaterHeater 0x02, 0x6B				///< 電気温水器
#define EL_ElectricToiletSeat 0x02, 0x6E				///< 電気便座(温水洗浄便座・暖房便座など)
#define EL_ElectricLock 0x02, 0x6F						///< 電気錠
#define EL_InstantaneousWaterHeater 0x02, 0x72			///< 瞬間式給湯機
#define EL_BathroomHeaterAndDryer 0x02, 0x73			///< 浴室暖房乾燥機
#define EL_HouseholdSolarPowerGeneration 0x02, 0x79		///< 住宅用太陽光発電
#define EL_ColdOrHotWaterHeatSourceEquipment 0x02, 0x7A ///< 冷温水熱源機
#define EL_FloorHeater 0x02, 0x7B						///< 床暖房
#define EL_FuelCell 0x02, 0x7C							///< 燃料電池
#define EL_Battery 0x02, 0x7D							///< 蓄電池
#define EL_ElectricVehicle 0x02, 0x7E					///< 電気自動車充放電器
#define EL_EngineCogeneration 0x02, 0x7F				///< エンジンコージェネレーション
#define EL_WattHourMeter 0x02, 0x80						///< 電力量メータ
#define EL_WaterFlowmeter 0x02, 0x81					///< 水流量メータ
#define EL_GasMeter 0x02, 0x82							///< ガスメータ
#define EL_LPGasMeter 0x02, 0x83						///< LPガスメータ
#define EL_PowerDistributionBoardMetering 0x02, 0x87	///< 分電盤メータリング
#define EL_SmartElectricEnergyMeter 0x02, 0x88			///< スマート電力量メータ
#define EL_SmartGasMeter 0x02, 0x89						///< スマートガスメータ
#define EL_GeneralLighting 0x02, 0x90					///< 一般照明
#define EL_Buzzer 0x02, 0xA0							///< ブザー
// 調理・家事関連機器
#define EL_ElectricHotWaterPot 0x03, 0xB2	   ///< 電気ポット
#define EL_Refrigerator 0x03, 0xB7			   ///< 冷凍冷蔵庫
#define EL_CombinationMicrowaveOven 0x03, 0xB8 ///< オーブンレンジ
#define EL_CookingHeater 0x03, 0xB9			   ///< クッキングヒータ
#define EL_RiceCooker 0x03, 0xBB			   ///< 炊飯器
#define EL_WashingMachine 0x03, 0xC5		   ///< 洗濯機
#define EL_ClothesDryer 0x03, 0xC6			   ///< 衣類乾燥機
#define EL_WasherAndDryer 0x03, 0xD3		   ///< 洗濯乾燥機
// 健康関連機器
#define EL_Weighing 0x04, 0x01 ///< 体重計
// 管理・操作関連機器
#define EL_Switch 0x05, 0xFD	 ///< スイッチ(JEM-A/HA端子対応)
#define EL_Controller 0x05, 0xFF ///< コントローラ
// AV関連機器
#define EL_Display 0x06, 0x01	 ///< ディスプレー
#define EL_Television 0x06, 0x02 ///< テレビ
// Node profile
#define EL_NodeProfile 0x0e, 0xf0 ///< ノードプロファイル

// 内部利用
#define EL_DEVID_NODEPROFILE -1 ///< ノードプロファイルのDevID
#define EL_DEVID_NOTHING -2		///< DevIDが見つからなかった
#define EL_DEVID_MULTI -3		///< インスタンス0なので複数マッチする可能性あり

// V.4
//      bool (*ELCallback) (  tid,   seoj,   deoj,  esv,  opc,  epc, pdc,  edt);
typedef bool (*ELCallback)(byte[], byte[], byte[], byte, byte, byte, byte, byte[]);

//////////////////////////////////////////////////////////////////////
/// @class EL
/// @brief Main class for EL
//////////////////////////////////////////////////////////////////////
class EL
{
private:
	IPAddress ip;				   ///< my ipaddress: 自分のIPアドレス
	IPAddress _multi;			   ///< multicast address:
	IPAddress _broad;			   ///< broadcast address:
	byte _mac[6];				   ///< mac address
	byte *_eojs;				   ///< EOJ array: = _eojs[][3]の構造で管理
	int deviceCount;			   ///< Number of EOJ: _eojs[deviceCount][3]に相当
	int _sendPacketSize = 0;	   ///< recentry sended packet size: 直近の送信パケットサイズ
	int _readPacketSize = 0;	   ///< recentry readed packet size: 直近の受信・読込パケットサイズ
	byte _sBuffer[EL_BUFFER_SIZE]; ///< send buffer: 直近の送信パケットデータ
	WiFiUDP *_udp;				   ///< WiFiのUDPソケット
	ELCallback userfunc;		   ///< ユーザの受信処理, V.4

protected:
	int parsePacket(void);											 // 受信データを読む
	void commonConstructor(WiFiUDP &udp, byte eojs[][3], int count); // コンストラクタで共通にコールされる
	void tidAutoIncrement(void);									 // データ送信時にTIDを自動的にインクリメントの再計算する(シンプルに+1するとオーバーフローするのでこれ使う)

public:
	ELOBJ profile;				   ///< profile object (for specialist)
	ELOBJ *devices;				   ///< device objects (for multi eoj)
	byte _rBuffer[EL_BUFFER_SIZE]; ///< receive buffer
	byte _tid[2];				   ///< TID (semi-auto incremented)

	////////////////////////////////////////////////////
	EL(WiFiUDP &udp, byte classGroupCode, byte classCode, byte instanceNumber); // for single dev (devid=0)
	EL(WiFiUDP &udp, byte eojs[][3], int count);								// for multi dev
	void begin(void);
	void begin(ELCallback cb); // V.4

	// details change
	void update(const byte epc, PDCEDT pdcedt);										  // for single dev (devid=0)
	void update(const int devId, const byte epc, PDCEDT pdcedt);					  // for multi dev
	void update(const byte epc, std::initializer_list<byte> edt);				  // for single dev (devid=0)
	void update(const int devId, const byte epc, std::initializer_list<byte> edt); // for multi dev

	// details
	byte *at(const byte epc);														  // for single dev (devid=0)
	byte *at(const int devId, const byte epc);										  // for multi dev

	// sender
	void send(IPAddress toip, byte sBuffer[], int size);
	void sendOPC1(const IPAddress toip, const byte tid[], const byte seoj[], const byte deoj[], const byte esv, const byte epc, const byte pdcedt[]);
	void sendOPC1(const IPAddress toip, const byte seoj[], const byte deoj[], const byte esv, const byte epc, const byte pdcedt[]);
	void sendOPC1(const IPAddress toip, const byte deoj[], const byte esv, const byte epc, const byte pdcedt[]);
	void sendOPC1ID(const IPAddress toip, const int devId, const byte deoj[], const byte esv, const byte epc, const byte pdcedt[]);
	void sendBroad(byte sBuffer[], int size);
	void sendMulti(byte sBuffer[], int size);
	void sendMultiOPC1(const byte tid[], const byte seoj[], const byte deoj[], const byte esv, const byte epc, const byte pdcedt[]);
	void sendMultiOPC1(const byte seoj[], const byte deoj[], const byte esv, const byte epc, const byte pdcedt[]);
	void sendMultiOPC1(const byte deoj[], const byte esv, const byte epc, const byte pdcedt[]);
	void sendMultiOPC1ID(const int devId, const byte deoj[], const byte esv, const byte epc, const byte pdcedt[]);
	// multi opc
	void sendDetails(const IPAddress toip, const byte tid[], const byte seoj[], const byte deoj[], const byte esv, const byte opc, const byte detail[], const byte detailSize);
	// return
	// void replyOPC1(const IPAddress toip, const unsigned short tid, const byte *seoj, const byte* deoj, const byte esv, const byte epc, const byte* edt);
	void replyGetDetail(const IPAddress toip, const byte seoj[]);
	boolean replyGetDetail_sub(const byte eoj[], const byte epc, int &devId);
	void replySetDetail(const IPAddress toip, const byte seoj[]);
	boolean replySetDetail_sub(const byte eoj[], const byte epc, int &devId);
	// INFプロパティ
	void checkInfAndSend(int devId, const byte epc);
	void checkInfAndSend(const byte eoj[], const byte epc);

	// reseiver
	int read();
	IPAddress remoteIP(void);
	void returner(void);
	void recvProcess(void); // 受信処理 V4, begin(callback)と一緒に使う

	// display, debug
	void printAll(void);

	////////////////////////////////////////////////////////////////////
	// inline function

	// オブジェクトを持っているかどうか判定して、持っているならdevIdを返す
	int getDevId(const byte obj[]);

	// byte[] を安全にdeleteする
	void delPtr(byte ptr[]);
};

#endif
//////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////
