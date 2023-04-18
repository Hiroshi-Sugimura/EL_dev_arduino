//////////////////////////////////////////////////////////////////////
/// @file ELOBJ.cpp
/// @brief Subclasses for ECHONET Lite protocol
/// @author SUGIMURA Hiroshi
/// @date 2013.09.27
/// @details https://github.com/Hiroshi-Sugimura/EL_dev_arduino
//////////////////////////////////////////////////////////////////////
#include <M5Core2.h>

#include "ELOBJ.h"


//////////////////////////////////////////////////////////////////////
/// @brief コンストラクタ
/// @return none
//////////////////////////////////////////////////////////////////////
PDCEDT::PDCEDT()
{
    m_pdcedt = nullptr;
    length = 0;
}

////////////////////////////////////////////////////
/// @brief コピーコンストラクタ
/// @param val const PDCEDT& コピー元
/// @return none
/// @note
PDCEDT::PDCEDT(const PDCEDT& val) // copy constractor
{
    length = val[0] + 1;
    m_pdcedt = new byte[length];
    memcpy(m_pdcedt, val, length);
}

////////////////////////////////////////////////////
/// @brief 初期化コンストラクタ
/// @param val const byte*
/// @return none
/// @note
PDCEDT::PDCEDT(const byte* val)
{
    length = val[0] + 1;
    m_pdcedt = new byte[length];
    memcpy(m_pdcedt, val, length);
}

////////////////////////////////////////////////////
/// @brief デストラクタ
/// @return none
/// @note
PDCEDT::~PDCEDT()
{
    if (nullptr != m_pdcedt)
    {
        delete[] m_pdcedt;
        m_pdcedt = nullptr;
        length = 0;
    }
}

////////////////////////////////////////////////////
/// @brief operator=
/// @param PDCEDT val
/// @return PDCEDT
/// @note PDCEDT型
const PDCEDT PDCEDT::operator=(const PDCEDT val)
{
    // cout << "ope = PDCEDT" << endl;
    if (nullptr != m_pdcedt)
    {
        // cout << "delete" << endl;
        delete[] m_pdcedt;
        m_pdcedt = nullptr;
    }
    // cout << "new" << endl;

    length = val[0] + 1;
    // cout << size << endl;
    m_pdcedt = new byte[length];
    memcpy(m_pdcedt, val, length);
    return *this;
}

////////////////////////////////////////////////////
/// @brief operator=
/// @param const byte* val
/// @return const byte*
/// @note
const byte* PDCEDT::operator=(const byte* val)
{
    // cout << "ope = byte" << endl;
    if (nullptr != m_pdcedt)
    {
        // cout << "delete" << endl;
        delete[] m_pdcedt;
        m_pdcedt = nullptr;
    }
    // cout << "new" << endl;

    length = val[0] + 1;
    // cout << size << endl;
    m_pdcedt = new byte[length];
    memcpy(m_pdcedt, val, length);

    return m_pdcedt;
}

////////////////////////////////////////////////////
/// @brief operator byte*
/// @param none
/// @return byte*
/// @note
PDCEDT::operator byte* () const
{
    return m_pdcedt;
}

////////////////////////////////////////////////////
/// @brief デバグ用の標準出力
/// @param void
/// @return void
/// @note
void PDCEDT::print(void) const
{

#ifdef Arduino_h
    if(length == 0) {
        Serial.println("error: PDCEDT len 0");
        return;
    }
    Serial.print("[");
    Serial.print(length);
    Serial.print("]: ");
    Serial.print((int)m_pdcedt[0]);
    for (byte i = 1; i < length; i += 1)
    {
        Serial.print(", ");
        Serial.print((int)m_pdcedt[i]);
    }
    Serial.print("\n");
#else
    if(length == 0) {
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
}

////////////////////////////////////////////////////
/// @brief デストラクタ
/// @return none
/// @note
ELOBJ::~ELOBJ() {}



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
    // cout << "Set PDCEDT" << endl;
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
const PDCEDT ELOBJ::SetPDCEDT(const byte epc, const byte*&& pdcedt)
{
    // cout << "Set byte*" << endl;
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
    int key = epc - 0x80;
    return ((PDCEDT &)m_pdcedt[key]);
}


////////////////////////////////////////////////////
/// @brief null以外のEPCを全部出力
/// @return none
/// @note
void ELOBJ::printAll() const
{
    for (int i = 0; i < PDC_MAX; i += 1)
    {
        if (nullptr != m_pdcedt[i])
        {
#ifdef Arduino_h
            Serial.print( (int)(i + 0x80) );
            Serial.print(": ");
#else
            cout << hex << i + 0x80 << ": ";
#endif
            m_pdcedt[i].print();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
