//////////////////////////////////////////////////////////////////////
/// @file ELOBJ.cpp
/// @brief Subclasses for ECHONET Lite protocol
/// @author SUGIMURA Hiroshi
/// @date 2013.09.27
/// @details https://github.com/Hiroshi-Sugimura/EL_dev_arduino
//////////////////////////////////////////////////////////////////////
#include "ELOBJ.h"

//////////////////////////////////////////////////////////////////////
/// @brief コンストラクタ
/// @param none
/// @return none
/// @note ex. PDCEDT p; p = {0x30, 0x31};
PDCEDT::PDCEDT()
{
	m_pdcedt = nullptr;
	length = 0;
}

////////////////////////////////////////////////////
/// @brief コピーコンストラクタ
/// @param val const PDCEDT& コピー元
/// @return none
/// @note ex. PDCEDT pb = pa;
PDCEDT::PDCEDT(const PDCEDT &val) // copy constractor
{
#ifdef DEBUG
	cout << "- PDCEDT::deep copy constractor PDCEDT" << endl;
#endif
	length = val.getLength();
	m_pdcedt = new byte[length];
	memcpy(m_pdcedt, val, length);
}

////////////////////////////////////////////////////
/// @brief 初期化コンストラクタ
/// @param val const byte*
/// @return none
/// @note ex. PDCEDT pb((byte[]){0x01, 0x8b});
PDCEDT::PDCEDT(const byte *val)
{
#ifdef DEBUG
	cout << "- PDCEDT::deep copy constractor byte*" << endl;
#endif
	length = val[0] + 1;
	m_pdcedt = new byte[length];
	memcpy(m_pdcedt, val, length);
}

////////////////////////////////////////////////////
/// @brief 初期化コンストラクタ
/// @param il iterator
/// @return none
/// @note  PDCEDT p = {0x30, 0x31}; のように実装可能なコンストラクタ
PDCEDT::PDCEDT(std::initializer_list<byte> il)
{
#ifdef DEBUG
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
/// @param none
/// @return none
/// @note
PDCEDT::~PDCEDT()
{
#ifdef DEBUG
	cout << "- PDCEDT::destructor" << endl;
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
#ifdef DEBUG
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
/// @note
const byte *PDCEDT::operator=(const byte *val)
{
#ifdef DEBUG
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
#ifdef DEBUG
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
/// @param none
/// @return byte*
/// @note
PDCEDT::operator byte *() const
{
#ifdef DEBUG
	cout << "- PDCEDT::cast to byte*" << endl;
#endif
	return m_pdcedt;
}

////////////////////////////////////////////////////
/// @brief EDT setter
/// @param il list<byte>
/// @return EDT byte*
/// @note 可変長引数のような実装、PDCは自動計算
/// ex. pf.setEDT( {0x81, 0x82, 0x83} );
const byte *PDCEDT::setEDT(std::initializer_list<byte> il)
{
#ifdef DEBUG
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
/// @brief PDCEDT Length getter
/// @param none
/// @return Length byte (= PDC + 1 Byte)
/// @note
const byte PDCEDT::getLength() const
{
#ifdef DEBUG
	cout << "- PDCEDT::getLength()" << endl;
#endif
	return length;
}

////////////////////////////////////////////////////
/// @brief PDC getter
/// @param none
/// @return PDC byte (length of EDT)
/// @note
const byte PDCEDT::getPDC()
{
#ifdef DEBUG
	cout << "- PDCEDT::getPDC()" << endl;
#endif
	return m_pdcedt[0];
}

////////////////////////////////////////////////////
/// @brief EDT getter
/// @param none
/// @return EDT byte*
/// @note
const byte *PDCEDT::getEDT()
{
#ifdef DEBUG
	cout << "- PDCEDT::getEDT()" << endl;
#endif
	return &m_pdcedt[1];
}

////////////////////////////////////////////////////
/// @brief 設定されているかどうか
/// @param void
/// @return boolean
/// @note
const bool PDCEDT::isEmpty() const
{
	if (nullptr == m_pdcedt)
	{
		return true;
	}
	return false;
}

////////////////////////////////////////////////////
/// @brief デバグ用の標準出力
/// @param void
/// @return void
/// @note
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
/// @return none
/// @note
ELOBJ::ELOBJ()
{
#ifdef DEBUG
	cout << "- ELOBJ::new empty" << endl;
#endif
}

////////////////////////////////////////////////////
/// @brief デストラクタ
/// @return none
/// @note
ELOBJ::~ELOBJ()
{
#ifdef DEBUG
	cout << "- ELOBJ::destractor" << endl;
#endif
}

////////////////////////////////////////////////////
/// @brief キー文字列からデータ取得
/// @param epc const byte
/// @return none
/// @note
const PDCEDT ELOBJ::GetPDCEDT(const byte epc) const
{
	int key = epc - 0x80;
	return m_pdcedt[key];
}

////////////////////////////////////////////////////
/// @brief EPCに対して、PDCEDTのを結びつける（セットと更新）
/// @param epc
/// @param pdcedt
/// @return none
/// @note pdcedtがPDCEDT型のときに呼ばれる
const PDCEDT ELOBJ::SetPDCEDT(const byte epc, const PDCEDT pdcedt)
{
#ifdef DEBUG
	cout << "- ELOBJ::SetPDCEDT PDCEDT" << endl;
#endif
	int key = epc - 0x80;
	m_pdcedt[key] = pdcedt;
	return (m_pdcedt[key]);
}

////////////////////////////////////////////////////
/// @brief EPCに対して、PDCEDTのを結びつける（セットと更新）
/// @param epc
/// @param pdcedt
/// @return none
/// @note pdcedtがconst byte*型のときに呼ばれる
const PDCEDT ELOBJ::SetPDCEDT(const byte epc, const byte *&&pdcedt)
{
#ifdef DEBUG
	cout << "- ELOBJ::SetPDCEDT byte *&&" << endl;
#endif
	int key = epc - 0x80;
	m_pdcedt[key] = pdcedt;
	return (m_pdcedt[key]);
}

////////////////////////////////////////////////////
/// @brief 配列らしいインターフェイス，const this
/// @param epc const byte
/// @return none
/// @note
const PDCEDT ELOBJ::operator[](const byte epc) const
{
#ifdef DEBUG
	cout << "- ELOBJ::SetPDCEDT const : operator[]" << endl;
#endif
	int key = epc - 0x80;
	return ((const PDCEDT)m_pdcedt[key]);
}

////////////////////////////////////////////////////
/// @brief 配列らしいインターフェイス，not const this
/// @param epc const byte
/// @return none
/// @note
PDCEDT &ELOBJ::operator[](const byte epc)
{
#ifdef DEBUG
	cout << "- ELOBJ::SetPDCEDT & : operator" << endl;
#endif
	int key = epc - 0x80;
	return ((PDCEDT &)m_pdcedt[key]);
}

////////////////////////////////////////////////////
/// @brief null以外のEPCを全部出力
/// @return none
/// @note
void ELOBJ::printAll() const
{
	char s[2];
	for (int i = 0; i < PDC_MAX; i += 1)
	{
		if ( !m_pdcedt[i].isEmpty() )
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
