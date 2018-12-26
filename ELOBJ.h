////////////////////////////////////////////////////////////////////////////////
// ECHONET Lite protocol object
//  Copyright (C) Hiroshi SUGIMURA 2013.09.27
////////////////////////////////////////////////////////////////////////////////
#ifndef __ELOBJ_H__
#define __ELOBJ_H__

// configure
#include <Arduino.h>

// auto config
#ifndef Arduino_h
#define byte unsigned char
#include <iostream>
using std::cout;
using std::dec;
using std::endl;
using std::hex;
using std::move;
#endif

//////////////////////////////////////////////////////////////////////
//	クラスの定義
class PDCEDT
{
protected:
  byte *m_pdcedt; //	データ
  byte length;

public:
  PDCEDT();
  PDCEDT(const PDCEDT& val);
  PDCEDT(const byte* val);
  virtual ~PDCEDT();
  const PDCEDT operator=(const PDCEDT val);
  const byte* operator=(const byte* val);
  operator byte* () const;
  void print(void) const;
};

class ELOBJ
{
  //	メンバ変数定義
public:
  const static byte PDC_MAX = 0x80; // 0xFF - 0x79

protected:
  PDCEDT m_pdcedt[PDC_MAX]; //	データ

  // member function
public:
  ELOBJ();          //	構築
  virtual ~ELOBJ(); //	消滅

  //	データ取得
  const PDCEDT GetPDCEDT(const byte epc) const;
  const PDCEDT SetPDCEDT(const byte epc, const PDCEDT pdcedt);
  const PDCEDT SetPDCEDT(const byte epc, const byte*&& pdcedt);

  //	配列らしいインターフェイス
  const PDCEDT operator[](const byte epc) const;
  PDCEDT& operator[](const byte epc);

  // 状態表示系
  void printAll() const; // all print edt
};

#endif
//////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////
