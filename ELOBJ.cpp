//////////////////////////////////////////////////////////////////////
/// @file ELOBJ.cpp
/// @brief Subclasses for ECHONET Lite protocol
/// @author SUGIMURA Hiroshi
/// @date 2013.09.27
/// @details https://github.com/Hiroshi-Sugimura/EL_dev_arduino
//////////////////////////////////////////////////////////////////////
#ifndef GPP
#include <ELOBJ.h>
#else
#include "ELOBJ.h"
#endif

// #define __ELOJB_DEBUG__ 1

//////////////////////////////////////////////////////////////////////
/// @brief コンストラクタ
/// @note e.g. PDCEDT p; p = {0x30, 0x31};
PDCEDT::PDCEDT()
{
	m_pdcedt = nullptr;
	length = 0;
}

////////////////////////////////////////////////////
/// @brief コピーコンストラクタ
/// @param val const PDCEDT& コピー元
/// @note e.g. PDCEDT pb = pa;
PDCEDT::PDCEDT(const PDCEDT &val) // copy constractor
{
#ifdef __ELOJB_DEBUG__
	cout << "- PDCEDT::deep copy constractor PDCEDT" << endl;
#endif
	length = val.getLength();
	m_pdcedt = new byte[length];
	memcpy(m_pdcedt, val, length);
}

////////////////////////////////////////////////////
/// @brief 初期化コンストラクタ
/// @param val const byte*
/// @note e.g. PDCEDT pb((byte[]){0x01, 0x8b});
PDCEDT::PDCEDT(const byte *val)
{
#ifdef __ELOJB_DEBUG__
	cout << "- PDCEDT::deep copy constractor byte*" << endl;
#endif
	length = val[0] + 1;
	m_pdcedt = new byte[length];
	memcpy(m_pdcedt, val, length);
}

////////////////////////////////////////////////////
/// @brief 初期化コンストラクタ
/// @param il iterator
/// @note  PDCEDT p = {0x30, 0x31}; のように実装可能なコンストラクタ
PDCEDT::PDCEDT(std::initializer_list<byte> il)
{
#ifdef __ELOJB_DEBUG__
	cout << "- PDCEDT::constractor iterator" << endl;
#endif
	byte n = (byte)il.size();
	length = n;
	m_pdcedt = new byte[length];

	int i = 0;
	for (auto it = il.begin(); it != il.end(); it += 1)
	{
		m_pdcedt[i] = *it;
		i += 1;
	}
}

////////////////////////////////////////////////////
/// @brief デストラクタ
PDCEDT::~PDCEDT()
{
#ifdef __ELOJB_DEBUG__
	// cout << "- PDCEDT::destructor" << endl;
#endif
	if (nullptr != m_pdcedt)
	{
		delete[] m_pdcedt;
		m_pdcedt = nullptr;
		length = 0;
	}
}

////////////////////////////////////////////////////
/// @brief operator=
/// @param val PDCEDT
/// @return PDCEDT
/// @note PDCEDT型
const PDCEDT PDCEDT::operator=(const PDCEDT val)
{
#ifdef __ELOJB_DEBUG__
	cout << "- PDCEDT::operator = PDCEDT" << endl;
#endif
	if (nullptr != m_pdcedt)
	{
		// cout << "delete" << endl;
		delete[] m_pdcedt;
		m_pdcedt = nullptr;
	}
	// cout << "new" << endl;

	length = val.getLength();
	// cout << size << endl;
	m_pdcedt = new byte[length];
	memcpy(m_pdcedt, val, length);
	return *this;
}

////////////////////////////////////////////////////
/// @brief operator=
/// @param val const byte*
/// @return const byte*
const byte *PDCEDT::operator=(const byte *val)
{
#ifdef __ELOJB_DEBUG__
	cout << "- PDCEDT::operator = byte*" << endl;
#endif
	if (nullptr != m_pdcedt)
	{
		// cout << "delete" << endl;
		delete[] m_pdcedt;
		m_pdcedt = nullptr;
	}

	length = val[0] + 1;
	m_pdcedt = new byte[length];
	memcpy(m_pdcedt, val, length);

	return m_pdcedt;
}

////////////////////////////////////////////////////
/// @brief operator= (iterator)
/// @param il list<byte>
/// @return const byte*
/// @note = {0x02, 0x31, 0x32}
const byte *PDCEDT::operator=(std::initializer_list<byte> il)
{
#ifdef __ELOJB_DEBUG__
	cout << "- PDCEDT::operator = iterator" << endl;
#endif
	byte n = (byte)il.size();
	length = n;
	m_pdcedt = new byte[length];

	int i = 0;
	for (auto it = il.begin(); it != il.end(); it += 1)
	{
		m_pdcedt[i] = *it;
		i += 1;
	}
	return m_pdcedt;
}

////////////////////////////////////////////////////
/// @brief operator byte*
/// @return pdcedt byte*
PDCEDT::operator byte *() const
{
#ifdef __ELOJB_DEBUG__
	cout << "- PDCEDT::cast to byte*" << endl;
#endif
	return m_pdcedt;
}

////////////////////////////////////////////////////
/// @brief EDT setter
/// @param il list<byte>
/// @return EDT byte*
/// @note 可変長引数のような実装、PDCは自動計算
/// e.g. pf.setEDT( {0x81, 0x82, 0x83} );
const byte *PDCEDT::setEDT(std::initializer_list<byte> il)
{
#ifdef __ELOJB_DEBUG__
	cout << "- PDCEDT::setEDT()" << endl;
#endif
	if (nullptr != m_pdcedt)
	{
		delete[] m_pdcedt;
		m_pdcedt = nullptr;
	}
	byte n = (byte)il.size();
	length = n + 1;
	m_pdcedt = new byte[length];
	m_pdcedt[0] = length - 1;

	int i = 1;
	for (auto it = il.begin(); it != il.end(); it += 1)
	{
		m_pdcedt[i] = *it;
		i += 1;
	}

	return m_pdcedt;
}

////////////////////////////////////////////////////
/// @brief EDT setter
/// @param edt byte[]
/// @param size int: size of edt
/// @return EDT byte*
/// @note PDCは自動計算だけどサイズがほしい
/// e.g. pf.setEDT( new[3]{0x81, 0x82, 0x83}, 3 );
const byte *PDCEDT::setEDT(const byte edt[], int size)
{
#ifdef __ELOJB_DEBUG__
	cout << "- PDCEDT::setEDT( const byte edt[], size)" << endl;
#endif
	if (nullptr != m_pdcedt)
	{
		delete[] m_pdcedt;
		m_pdcedt = nullptr;
	}
	length = (byte)size + 1;
	m_pdcedt = new byte[length];
	m_pdcedt[0] = size;

	int i = 1;
	for (int i = 1; i <= size; i += 1)
	{
		m_pdcedt[i] = edt[i - 1];
	}

	return m_pdcedt;
}

////////////////////////////////////////////////////
/// @brief PDCEDT Length getter
/// @return Length byte (= PDC + 1 Byte)
const byte PDCEDT::getLength() const
{
#ifdef __ELOJB_DEBUG__
	cout << "- PDCEDT::getLength()" << endl;
#endif
	return length;
}

////////////////////////////////////////////////////
/// @brief PDC getter
/// @return PDC byte (length of EDT)
const byte PDCEDT::getPDC() const
{
#ifdef __ELOJB_DEBUG__
	cout << "- PDCEDT::getPDC()" << endl;
#endif
	return m_pdcedt[0];
}

////////////////////////////////////////////////////
/// @brief EDT getter
/// @return EDT byte*
const byte *PDCEDT::getEDT() const
{
#ifdef __ELOJB_DEBUG__
	cout << "- PDCEDT::getEDT()" << endl;
#endif
	return &m_pdcedt[1];
}

////////////////////////////////////////////////////
/// @brief 設定されているかどうか
/// @return boolean true:empty, false: not empty
/// @note isNullと同じ
const bool PDCEDT::isEmpty() const
{
	if (nullptr == m_pdcedt)
	{
		return true;
	}
	return false;
}

////////////////////////////////////////////////////
/// @brief Nullかどうか
/// @return boolean true:Null, false: not Null
/// @note isEmptyと同じ
const bool PDCEDT::isNull() const
{
	if (nullptr == m_pdcedt)
	{
		return true;
	}
	return false;
}

////////////////////////////////////////////////////
/// @brief デバグ用の標準出力
void PDCEDT::print(void) const
{
	char s[2];
#ifdef Arduino_h
	if (length == 0)
	{
		Serial.println("error: PDCEDT len 0");
		return;
	}

	// length = pdcedt bytes
	Serial.print("size:");
	Serial.print(length);
	Serial.print(": ");

	// PDC
	sprintf(s, "%02X", (int)m_pdcedt[0]);
	Serial.print(s);

	// EDT
	for (byte i = 1; i < length; i += 1)
	{
		Serial.print(", ");
		sprintf(s, "%02X", (int)m_pdcedt[i]);
		Serial.print(s);
	}
	Serial.print("\n");

#else
	if (length == 0)
	{
		cout << "error: PDCEDT len 0" << endl;
		return;
	}
	cout << "[" << dec << (int)length << "]: ";
	cout << dec << (int)m_pdcedt[0];
	for (byte i = 1; i < length; i += 1)
	{
		cout << ", " << hex << (int)m_pdcedt[i];
	}
	cout << endl;
#endif
}

//////////////////////////////////////////////////////////////////////
//	EOOBJ
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////
/// @brief コンストラクタ
ELOBJ::ELOBJ()
{
#ifdef __ELOJB_DEBUG__
	cout << "- ELOBJ::new empty" << endl;
#endif
}

////////////////////////////////////////////////////
/// @brief デストラクタ
ELOBJ::~ELOBJ()
{
#ifdef __ELOJB_DEBUG__
	cout << "- ELOBJ::destractor" << endl;
#endif
}

////////////////////////////////////////////////////
/// @brief キー文字列からデータ取得
/// @param epc const byte
/// @return m_pdcedt[key]
const PDCEDT ELOBJ::GetPDCEDT(const byte epc) const
{
	int key = epc - 0x80;
	return m_pdcedt[key];
}

////////////////////////////////////////////////////
/// @brief EPCに対して、PDCEDTのを結びつける（セットと更新）
/// @param epc const byte
/// @param pdcedt const PDCEDT
/// @return m_pdcedt[key]
/// @note pdcedtがPDCEDT型のときに呼ばれる
const PDCEDT ELOBJ::SetPDCEDT(const byte epc, const PDCEDT pdcedt)
{
#ifdef __ELOJB_DEBUG__
	cout << "- ELOBJ::SetPDCEDT PDCEDT" << endl;
#endif
	int key = epc - 0x80;
	m_pdcedt[key] = pdcedt;
	return (m_pdcedt[key]);
}

////////////////////////////////////////////////////
/// @brief EPCに対して、PDCEDTのを結びつける（セットと更新）
/// @param epc const byte
/// @param pdcedt const byte*&&
/// @return m_pdcedt[key]
/// @note pdcedtがconst byte*型のときに呼ばれる
const PDCEDT ELOBJ::SetPDCEDT(const byte epc, const byte *&&pdcedt)
{
#ifdef __ELOJB_DEBUG__
	cout << "- ELOBJ::SetPDCEDT byte *&&" << endl;
#endif
	int key = epc - 0x80;
	m_pdcedt[key] = pdcedt;
	return (m_pdcedt[key]);
}

////////////////////////////////////////////////////
/// @brief EPCに対して、PDCEDTのを結びつける（セットと更新）
/// @param epc const byte
/// @param il std::initializer_list<byte>
/// @return m_pdcedt[key]
const PDCEDT ELOBJ::SetPDCEDT(const byte epc, std::initializer_list<byte> il)
{
#ifdef __ELOJB_DEBUG__
	cout << "- ELOBJ::SetPDCEDT std::initializer_list<byte>" << endl;
#endif
	int key = epc - 0x80;
	m_pdcedt[key] = PDCEDT(il);
	return (m_pdcedt[key]);
}

////////////////////////////////////////////////////
/// @brief PropertyMap(0x9d, 0x9e, 0x9f)を計算してPDCとEDTを設定する
/// @param epc const byte
/// @param epcs std::initializer_list<byte> : epc list
/// @return m_pdcedt[key]
/// @note e.g. obj.SetMyPropertyMap( {0x80, 0x81, 0x88} )
/// プロパティ数16以上（PDC含めると17Byte以上）のとき、Format 2
const PDCEDT ELOBJ::SetMyPropertyMap(const byte epc, std::initializer_list<byte> epcs)
{
#ifdef __ELOJB_DEBUG__
	cout << "- ELOBJ::SetMyPropertyMap()" << endl;
#endif
	int key = epc - 0x80;
	byte n = (byte)epcs.size();

	if (n < 16)
	{						  // format 1
		byte temp_edt[n + 1]; // EDT
		temp_edt[0] = n;	  // EDT[0]はプロパティの数
		int i = 1;
		for (auto it = epcs.begin(); it != epcs.end(); it += 1)
		{
			temp_edt[i] = *it;
			i += 1;
		}

		PDCEDT t;
		t.setEDT(temp_edt, n + 1);
		m_pdcedt[key] = t;
	}
	else
	{							 // format 2
		byte temp_edt[17] = {0}; // 確保するEDTのメモリは17Byte固定となる
		temp_edt[0] = n;		 // EDT[0]はプロパティ数

		for (auto it = epcs.begin(); it != epcs.end(); it += 1)
		{
			int i = (*it & 0x0f) + 1;			  // バイト目
			byte flag = 0x01 << ((*it >> 4) - 8); // フラグ位置
#ifdef __ELOJB_DEBUG__
			// cout << "- ELOBJ::SetProfile() [" << dec << i << "]" << hex << (int)flag << endl;
#endif
			temp_edt[i] += flag;
		}
		PDCEDT t;
		t.setEDT(temp_edt, 17);
		m_pdcedt[key] = t;
	}
	return (m_pdcedt[key]);
}

////////////////////////////////////////////////////
/// @brief PropertyMap(0x9d, 0x9e, 0x9f)を計算して 個数 + EPCsの形で返す（個数はPDCではないことに注意）
/// @param epc const byte
/// @return epcs Num + EPCs
/// @note e.g. obj.GetMyPropertyMap( 0x9d );
/// Format 2を解析するところがミソ
const byte *ELOBJ::GetMyPropertyMap(const byte epc) const
{
#ifdef __ELOJB_DEBUG__
	cout << "- ELOBJ::GetMyPropertyMap()" << endl;
#endif
	int key = epc - 0x80;
	byte *pdcedt = m_pdcedt[key];
	byte pdc = pdcedt[0];
	byte profNum = pdcedt[1];
	byte *epcs = &pdcedt[2];
#ifdef __ELOJB_DEBUG__
	// cout << "- ELOBJ::GetMyPropertyMap() PDC:" << dec << (int)pdc << endl;
	// cout << "- ELOBJ::GetMyPropertyMap() Num:" << dec << (int)profNum << endl;
#endif
	if (profNum < 16)
	{ // format 1ならそのままの形式、ただし 個数+epc listは pdcedt[1] に相当する
		return &pdcedt[1];
	}
	else
	{									   // format 2
		byte *ret = new byte[profNum + 1]; // 確保するメモリはprofileの数+1
		ret[0] = profNum;

		int count = 1;
		for (int bit = 0; bit < 8; bit += 1)
		{
			for (int i = 1; i < 17; i += 1)
			{
				byte exist = ((epcs[i] >> bit) & 0x01);
				if (exist)
				{
					// 上位 (bit + 7) << 4
					// 下位 i-1
					byte epc = ((bit + 8) << 4) + (i - 1);
#ifdef __ELOJB_DEBUG__
					// DEBUG時でもうるさいので必要な時だけ有効にする
					// cout << "- ELOBJ::GetProfile() : " << dec << count << ":" << hex << (int)epc << endl;
#endif
					ret[count] = epc;
					count += 1;
				}
			}
		}
		return ret;
	}
}

////////////////////////////////////////////////////
/// @brief 指定のEPCがINF必須かどうか
/// @param epc const byte
/// @return boolean true:available, false: not available
/// @note EPC:0x9dで判定する
/// 毎回Profileを全部作って捜査するので少し遅い。
/// いくつもEPCを検索するなら、GetMyPropertyMapをつかって自分で探すことをお勧めする。
const bool ELOBJ::hasInfProperty(const byte epc) const
{
	int key = epc - 0x80;
	const byte *setlist = GetMyPropertyMap(0x9d);
	const byte num = setlist[0];
	const byte *edtarray = &setlist[1];

	for (int i = 0; i < num; i += 1)
	{
#ifdef __ELOJB_DEBUG__
		// cout << "- ELOBJ::hasInfProperty : " << (int)i + 1 << "/" << (int)num << endl;
		// cout << "- ELOBJ::hasInfProperty : has:" << hex << (int)edtarray[i] << endl;
		// cout << "- ELOBJ::hasInfProperty : epc:" << hex << (int)epc << endl;
#endif
		if (edtarray[i] == epc)
		{
			return true;
		}
	}

	return false;
}

////////////////////////////////////////////////////
/// @brief 指定のEPCがSet可能かどうか
/// @param epc const byte
/// @return boolean true:available, false: not available
/// @note EPC:0x9eで判定する
/// 毎回Profileを全部作って捜査するので少し遅い。
/// いくつもEPCを検索するなら、GetMyPropertyMapをつかって自分で探すことをお勧めする。
/// No checkでEPCにセットすると、新規作成するのできちんとチェックしてからセットしたい
const bool ELOBJ::hasSetProperty(const byte epc) const
{
	int key = epc - 0x80;
	const byte *setlist = GetMyPropertyMap(0x9d);
	const byte num = setlist[0];
	const byte *edtarray = &setlist[1];

	for (int i = 0; i < num; i += 1)
	{
#ifdef __ELOJB_DEBUG__
		// cout << "- ELOBJ::hasSetProperty : " << (int)i + 1 << "/" << (int)num << endl;
		// cout << "- ELOBJ::hasSetProperty : has:" << hex << (int)edtarray[i] << endl;
		// cout << "- ELOBJ::hasSetProperty : epc:" << hex << (int)epc << endl;
#endif
		if (edtarray[i] == epc)
		{
			return true;
		}
	}

	return false;
}

////////////////////////////////////////////////////
/// @brief 指定のEPCがGet可能かどうか
/// @param epc const byte
/// @return boolean true:available, false: no EPC
/// @note isEmptyの逆と思っていい。プロパティ持っているかだけで判定する、EPC:0x9fは確認しない
const bool ELOBJ::hasGetProperty(const byte epc) const
{
	int key = epc - 0x80;
	return (!m_pdcedt[key].isNull());
}

////////////////////////////////////////////////////
/// @brief 配列らしいインターフェイス，const this
/// @param epc const byte
/// @return pdcedt const PDCEDT
const PDCEDT ELOBJ::operator[](const byte epc) const
{
#ifdef __ELOJB_DEBUG__
	cout << "- ELOBJ::SetPDCEDT const : operator[]" << endl;
#endif
	int key = epc - 0x80;
	return ((const PDCEDT)m_pdcedt[key]);
}

////////////////////////////////////////////////////
/// @brief 配列らしいインターフェイス，not const this
/// @param epc const byte
/// @return pdcedt PDCEDT&
PDCEDT &ELOBJ::operator[](const byte epc)
{
#ifdef __ELOJB_DEBUG__
	cout << "- ELOBJ::SetPDCEDT & : operator" << endl;
#endif
	int key = epc - 0x80;
	return ((PDCEDT &)m_pdcedt[key]);
}

////////////////////////////////////////////////////
/// @brief null以外のEPCを全部出力
void ELOBJ::printAll() const
{
	char s[2];
	for (int i = 0; i < PDC_MAX; i += 1)
	{
		if (!m_pdcedt[i].isEmpty())
		{
#ifdef Arduino_h
			sprintf(s, "%02X", (int)(i + 0x80)); // print EPC
			Serial.print(s);
			Serial.print(": ");
#else
			cout << hex << i + 0x80 << ": ";
#endif
			m_pdcedt[i].print(); // print pdc and edt
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
