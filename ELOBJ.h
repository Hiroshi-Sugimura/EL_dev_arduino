////////////////////////////////////////////////////////////////////////////////
// ECHONET Lite protocol object
//  Copyright (C) Hiroshi SUGIMURA 2013.09.27
////////////////////////////////////////////////////////////////////////////////
#ifndef __ELOBJ_H__
#define __ELOBJ_H__
#pragma once
#include <Arduino.h>

/*
#define byte unsigned char

#include <iostream>
using std::cout;
using std::endl;
using std::hex;
using std::move;
 */

//////////////////////////////////////////////////////////////////////
//	クラスの定義
class PDCEDT
{
protected:
	byte*	m_pdcedt;				//	データ

public:
	PDCEDT();
	PDCEDT(const PDCEDT& val);
	PDCEDT(const byte*& val);
	virtual ~PDCEDT();
	const PDCEDT operator=(const PDCEDT val);
	const byte* operator=(const byte* val);
	operator byte*() const;
};


class ELOBJ
{
	//	メンバ変数定義
public:
	const static byte PDC_MAX = 0x80;	// 0xFF - 0x79

protected:
	PDCEDT	m_pdcedt[PDC_MAX];				//	データ

	// member function
public:
	ELOBJ();	//	構築
	virtual	~ELOBJ();	//	消滅

	//	データ取得
	const	PDCEDT	GetPDCEDT( const byte epc ) const;
	const	PDCEDT	SetPDCEDT( const byte epc, const PDCEDT pdcedt);
	const	PDCEDT	SetPDCEDT( const byte epc, const byte*  pdcedt);

	//	配列らしいインターフェイス
	// const byte*  operator[]( const byte epc ) const;	// 読み取り, Rvalues
	// byte*& operator[]( const byte epc ); 		// 書き込み, Lvalues, これしかうごかん

	const	PDCEDT  operator[]( const byte epc ) const;
	PDCEDT& operator[]( const byte epc );

	// 状態表示系
	static void pdcedt_print( const PDCEDT pdcedt );
	static void pdcedt_print( const byte* pdcedt );
	void printAll() const;  // all print edt
};


#endif
//////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////
