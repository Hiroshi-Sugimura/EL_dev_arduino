//////////////////////////////////////////////////////////////////////
/// @file ELOBJ.h
/// @brief Subclasses for ECHONET Lite protocol
/// @author SUGIMURA Hiroshi
/// @date 2013.09.27
/// @details https://github.com/Hiroshi-Sugimura/EL_dev_arduino
//////////////////////////////////////////////////////////////////////
#ifndef __ELOBJ_H__
#define __ELOBJ_H__
#pragma once

#include <initializer_list>

// auto config
#ifndef GPP
// arduino
#include <Arduino.h>

#else
// g++
typedef unsigned char byte;
#include <iostream>
using std::cout;
using std::dec;
using std::endl;
using std::hex;
using std::move;
#endif

//////////////////////////////////////////////////////////////////////
/// @class PDCEDT
/// @brief PDC and EDT in ELOBJ
//////////////////////////////////////////////////////////////////////
class PDCEDT
{
protected:
  byte *m_pdcedt; ///< PDC(1 Byte) + EDT(n Byte)
  byte length;    ///< length for m_pdcedt

public:
  PDCEDT();
  PDCEDT(const PDCEDT &val);
  PDCEDT(const byte *val);
  PDCEDT(std::initializer_list<byte> il);

  virtual ~PDCEDT();

  // setter
  const PDCEDT operator=(const PDCEDT val);
  const byte *operator=(const byte *val);
  const byte *operator=(std::initializer_list<byte> il);
  const byte *setEDT(const byte edt[], int size);
  const byte *setEDT(std::initializer_list<byte> il);

  // getter
  const byte getLength() const;
  const byte getPDC() const;
  const byte *getEDT() const;

  // キャスト
  operator byte *() const;

  // チェック
  const bool isEmpty() const;
  const bool isNull() const;

  void print(void) const;
};

//////////////////////////////////////////////////////////////////////
/// @class ELOBJ
/// @brief EL Object
//////////////////////////////////////////////////////////////////////
class ELOBJ
{
  //	メンバ変数定義
protected:
  const static byte PDC_MAX = 0x80; ///< PDC_MAX 0xFF - 0x79
  PDCEDT m_pdcedt[PDC_MAX]; ///< = m_pdcedt[EPC] (EPC mapped( EPC - 0x80 = 0.. 0xFF = 0x80 );

  // member function
public:
  ELOBJ();          //	構築
  virtual ~ELOBJ(); //	消滅

  //	データ取得
  const PDCEDT GetPDCEDT(const byte epc) const;
  const PDCEDT SetPDCEDT(const byte epc, const PDCEDT pdcedt);
  const PDCEDT SetPDCEDT(const byte epc, const byte *&&pdcedt);
  const PDCEDT SetProfile(const byte epc, std::initializer_list<byte> epcs); // for EPC: 9e,9d,9f
  const PDCEDT GetProfile(const byte epc) const;

  const bool hasGetProfile(const byte epc) const; // Get可能なEPC?
  const bool hasSetProfile(const byte epc) const; // Set可能なEPC?
  const bool hasInfProfile(const byte epc) const; // Inf必須なEPC?

  //	配列らしいインターフェイス
  const PDCEDT operator[](const byte epc) const;
  PDCEDT &operator[](const byte epc);

  // 状態表示系
  void printAll() const; // all print edt
};

#endif
//////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////
