////////////////////////////////////////////////////////////////////////////////
// ECHONET Lite protocol object
//  Copyright (C) Hiroshi SUGIMURA 2018.07.02
////////////////////////////////////////////////////////////////////////////////
// #include <iostream>
// #include <memory>


#include "ELOBJ.h"
/*
using std::cout;
using std::endl;
using std::hex;
using std::move;
 */


//////////////////////////////////////////////////////////////////////
//	PDCEDT
//////////////////////////////////////////////////////////////////////
PDCEDT::PDCEDT() {
	m_pdcedt = nullptr;
}
PDCEDT::PDCEDT(const PDCEDT& val) {
	m_pdcedt = new byte[ val[0]+1 ];
	memcpy( m_pdcedt, val, val[0]+1 );
}
PDCEDT::PDCEDT(const byte*& val) {
	m_pdcedt = new byte[ val[0]+1 ];
	memcpy( m_pdcedt, val, val[0]+1 );
}

PDCEDT::~PDCEDT() {
	if(nullptr != m_pdcedt) {delete [] m_pdcedt; m_pdcedt = nullptr;}
}

const PDCEDT PDCEDT::operator=(const PDCEDT val) {
	// cout << "ope = PDCEDT" << endl;
	if(nullptr != m_pdcedt) {
		// cout << "delete" << endl;
		delete [] m_pdcedt; m_pdcedt = nullptr;
	}
	// cout << "new" << endl;

	int size = val[0]+1;
	// cout << size << endl;
	m_pdcedt = new byte[ size ];
	memcpy( m_pdcedt, val, size );
	return *this;
}

const byte* PDCEDT::operator=(const byte* val) {
	// cout << "ope = byte" << endl;
	if(nullptr != m_pdcedt) {
		// cout << "delete" << endl;
		delete [] m_pdcedt; m_pdcedt = nullptr;
	}
	// cout << "new" << endl;

	int size = val[0]+1;
	// cout << size << endl;
	m_pdcedt = new byte[ size ];
	memcpy( m_pdcedt, val, size );

	return m_pdcedt;
}

PDCEDT::operator byte*() const {
	return m_pdcedt;
}






//////////////////////////////////////////////////////////////////////
//	EOOBJ
//////////////////////////////////////////////////////////////////////
ELOBJ::ELOBJ() {}

ELOBJ::~ELOBJ() {}


//////////////////////////////////////////////////////////////////////
//	キー文字列からデータ取得
const PDCEDT ELOBJ::GetPDCEDT(const byte epc) const
{
	int key = epc - 0x80;
	return m_pdcedt[key];
}

//	データセット, 更新
const PDCEDT ELOBJ::SetPDCEDT(const byte epc, const PDCEDT pdcedt)
{
	// cout << "Set PDCEDT" << endl;
	int key = epc - 0x80;
	m_pdcedt[key] = pdcedt;
	return (m_pdcedt[key]);
}

const PDCEDT ELOBJ::SetPDCEDT(const byte epc, const byte* pdcedt)
{
	// cout << "Set byte*" << endl;
	int key = epc - 0x80;
	m_pdcedt[key] = pdcedt;
	return (m_pdcedt[key]);
}

/*
//	配列らしいインターフェイス，右辺値として使う用
//  代入不可
const byte* ELOBJ::operator[](const byte epc) const
{
	// ELOBJ *&returnal = Search(epc);
	// return (returnal->m_pdcedt);
	cout << "右辺" << endl;
	return GetPDCEDT(epc);
}


//	配列らしいインターフェイス，左辺値として使う用
//	代入可
byte*&	ELOBJ::operator[]( byte epc )
{
	cout << "左辺" << endl;
	int key = epc - 0x80;

	if( nullptr != m_pdcedt[key] ) {
		delete [] m_pdcedt[key];
		m_pdcedt[key] = nullptr;
	}
	return ( (byte*&)m_pdcedt[key]);
}
 */

//	配列らしいインターフェイス，右辺値として使う用
//  代入不可
const PDCEDT ELOBJ::operator[](const byte epc) const // rvalue
{
	// ELOBJ *&returnal = Search(epc);
	// return (returnal->m_pdcedt);
	// cout << "右辺" << endl;
	int key = epc - 0x80;
	return ( (const PDCEDT)m_pdcedt[key]);
}


//	配列らしいインターフェイス，左辺値として使う用
//	代入可
PDCEDT&	ELOBJ::operator[]( const byte epc ) // lvalue
{
	// cout << "左辺1" << endl;
	int key = epc - 0x80;
	return ( (PDCEDT&)m_pdcedt[key]);
}

/*
const PDCEDT ELOBJ::operator[](const byte epc) && // rvalue
{
	// ELOBJ *&returnal = Search(epc);
	// return (returnal->m_pdcedt);
	cout << "右辺2" << endl;
	int key = epc - 0x80;
	return ( (const PDCEDT&)m_pdcedt[key]);
}
 */



// 状態表示系
void ELOBJ::pdcedt_print(const byte* edt)
{
	Serial.print((int)edt[0]);
	// cout << hex << (int)edt[0];

	for (byte i = 1; i <= edt[0]; i += 1)
	{
		Serial.print(", ");
		Serial.print((int)edt[i], HEX);
		// cout << ", " << hex << (int)edt[i];
	}
	Serial.print("\n");
	// cout << endl;
}

void ELOBJ::pdcedt_print(const PDCEDT edt)
{
	pdcedt_print( (byte*)(edt) );
}

// null以外全部出力
void ELOBJ::printAll() const
{
	for( int i=0; i<PDC_MAX; i+=1 ) {

		if( nullptr != m_pdcedt[i] ) {
			/// cout << hex << i + 0x80 << ": ";
			Serial.print("EPC: ");
			Serial.print( i+0x80, HEX);
			Serial.print(": ");
			ELOBJ::pdcedt_print( m_pdcedt[i] );
		}

	}
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
