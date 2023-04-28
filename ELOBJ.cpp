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
/// @note e.g. PDCEDT p; p = {0x30, 0x31};
PDCEDT::PDCEDT()
{
	m_pdcedt = nullptr;
	length = 0;
}

////////////////////////////////////////////////////
/// @brief コピーコンストラクタ
/// @param val const PDCEDT& コピー元
/// @return none
/// @note e.g. PDCEDT pb = pa;
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
/// @note e.g. PDCEDT pb((byte[]){0x01, 0x8b});
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
/// e.g. pf.setEDT( {0x81, 0x82, 0x83} );
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
const byte PDCEDT::getPDC() const
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
const byte *PDCEDT::getEDT() const
{
#ifdef DEBUG
	cout << "- PDCEDT::getEDT()" << endl;
#endif
	return &m_pdcedt[1];
}

////////////////////////////////////////////////////
/// @brief 設定されているかどうか
/// @param void
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
/// @param void
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
/// @brief Profile(0x9d, 0x9e, 0x9f)を計算してPDCとEDTを設定する
/// @param epcs std::initializer_list<byte> : epc list
/// @return none
/// @note e.g. obj.SetProfile( {0x80, 0x81, 0x88} )
/// プロパティ数16以上（PDC含めると17Byte以上）のとき、Format 2
const PDCEDT ELOBJ::SetProfile(const byte epc, std::initializer_list<byte> epcs)
{
#ifdef DEBUG
	cout << "- ELOBJ::SetProfile()" << endl;
#endif
	int key = epc - 0x80;
	byte n = (byte)epcs.size();

	if (n < 16)
	{ // format 1
		PDCEDT t;
		m_pdcedt[key] = t.setEDT(epcs);
	}
	else
	{								// format 2
		byte temp_pdcedt[17] = {0}; // 確保するメモリは17Byte固定(PDC + EDT[16])となる
		temp_pdcedt[0] = n;			// PDCはプロパティ数

		for (auto it = epcs.begin(); it != epcs.end(); it += 1)
		{
			int i = (*it & 0x0f) + 1;			  // バイト目
			byte flag = 0x01 << ((*it >> 4) - 8); // フラグ位置
#ifdef DEBUG
			// cout << "- ELOBJ::SetProfile() [" << dec << i << "]" << hex << (int)flag << endl;
#endif
			temp_pdcedt[i] += flag;
		}

		m_pdcedt[key] = temp_pdcedt;
	}
	return (m_pdcedt[key]);
}

////////////////////////////////////////////////////
/// @brief Profile(0x9d, 0x9e, 0x9f)を計算してPDC[1] + EDT[PDC]の形で返す
/// @param epc const byte
/// @return epcs PDCEDT
/// @note e.g. obj.GetProfile( 0x9d );
/// Format 2を解析するところがミソ
const PDCEDT ELOBJ::GetProfile(const byte epc) const
{
#ifdef DEBUG
	cout << "- ELOBJ::GetProfile()" << endl;
#endif
	int key = epc - 0x80;
	byte *pdcedt = m_pdcedt[key];
	byte pdc = pdcedt[0];
#ifdef DEBUG
	cout << "- ELOBJ::GetProfile() PDC:" << dec << (int)pdc << endl;
#endif
	if (pdc < 16)
	{ // format 1ならそのまま
		return m_pdcedt[key];
	}
	else
	{								   // format 2
		byte *ret = new byte[pdc + 1]; // 確保するメモリは17Byte固定(PDC + EDT[16])となる
		ret[0] = pdc;

		int count = 1;
		for (int bit = 0; bit < 8; bit += 1)
		{
			for (int i = 1; i < 17; i += 1)
			{
				byte exist = ((pdcedt[i] >> bit) & 0x01);
				if (exist)
				{
					// 上位 (bit + 7) << 4
					// 下位 i-1
					byte epc = ((bit + 8) << 4) + (i - 1);
#ifdef DEBUG
					// DEBUG時でもうるさいので必要な時だけ有効にする
					// cout << "- ELOBJ::GetProfile()" << dec << i << ":" << hex << (int)bit << endl;
#endif
					ret[count] = epc;
					count += 1;
				}
			}
		}
		return PDCEDT(ret);
	}
}

////////////////////////////////////////////////////
/// @brief 指定のEPCがGet可能かどうか
/// @param void
/// @return boolean true:available, false: no EPC
/// @note isEmptyの逆と思っていい。プロパティ持っているかだけで判定する、EPC:0x9fは確認しない
const bool ELOBJ::hasGetProfile(const byte epc) const
{
	int key = epc - 0x80;
	return (!m_pdcedt[key].isNull());
}


////////////////////////////////////////////////////
/// @brief 指定のEPCがSet可能かどうか
/// @param epc const byte
/// @return boolean true:available, false: not available
/// @note EPC:0x9eで判定する
/// 毎回Profileを全部作って捜査するので少し遅い。
/// いくつもEPCを検索するなら、GetProfileをつかって自分で探すことをお勧めする。
/// No checkでEPCにセットすると、新規作成するのできちんとチェックしてからセットしたい
const bool ELOBJ::hasSetProfile(const byte epc) const
{
	int key = epc - 0x80;
	const PDCEDT setlist = GetProfile(0x9e);
	const byte pdc = setlist.getPDC();
	const byte* edtarray = setlist.getEDT();
	bool ret = false;
	for (int i = 1; i < pdc; i += 1)
	{
		if (edtarray[i] == epc)
		{
			ret = true;
		}
	}

	return ret;
}


////////////////////////////////////////////////////
/// @brief 指定のEPCがINF必須かどうか
/// @param epc const byte
/// @return boolean true:available, false: not available
/// @note EPC:0x9fで判定する
/// 毎回Profileを全部作って捜査するので少し遅い。
/// いくつもEPCを検索するなら、GetProfileをつかって自分で探すことをお勧めする。
const bool ELOBJ::hasInfProfile(const byte epc) const
{
	int key = epc - 0x80;
	const PDCEDT setlist = GetProfile(0x9f);
	const byte pdc = setlist.getPDC();
	const byte* edtarray = setlist.getEDT();
	bool ret = false;
	for (int i = 1; i < pdc; i += 1)
	{
		if (edtarray[i] == epc)
		{
			ret = true;
		}
	}

	return ret;
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
