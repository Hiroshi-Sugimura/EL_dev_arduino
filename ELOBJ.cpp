////////////////////////////////////////////////////////////////////////////////
// ECHONET Lite protocol object
//  Copyright (C) Hiroshi SUGIMURA 2018.07.02
////////////////////////////////////////////////////////////////////////////////
#include <new.h>
#include "ELOBJ.h"

//////////////////////////////////////////////////////////////////////
//	構築/消滅
//////////////////////////////////////////////////////////////////////
ELOBJ::ELOBJ(byte key, byte *dat, unsigned long dat_size)
{
  m_leftnode = NULL;
  m_rightnode = NULL;
  m_key = key;
  m_data = NULL;
  if (NULL != dat)
  {
    m_data = new byte[dat_size];
    memcpy(m_data, dat, dat_size);
  }
}

ELOBJ::~ELOBJ()
{
  if (NULL != this)
  {
    delete m_leftnode;
    m_leftnode = NULL;

    m_key = 0x00;

    delete[] m_data;
    m_data = NULL;

    delete m_rightnode;
    m_rightnode = NULL;
  }
}

//////////////////////////////////////////////////////////////////////
//	自身取得
byte ELOBJ::GetKey(void) const
{
  return (m_key);
}

byte *ELOBJ::GetData(void) const
{
  return (m_data);
}

ELOBJ *ELOBJ::GetLeftNode(void) const
{
  return (m_leftnode);
}

ELOBJ *ELOBJ::GetRightNode(void) const
{
  return (m_rightnode);
}

//////////////////////////////////////////////////////////////////////
//	キー文字列、データ追加
ELOBJ *&ELOBJ::SetKey(const byte key)
{
  ELOBJ *&returnal = Search(key);

  if (NULL == returnal)
  {
    ELOBJ *tmp = new ELOBJ(key);
    returnal = tmp;
    return returnal;
  }

  return returnal;
}

//	データ更新
ELOBJ *&ELOBJ::SetData(const byte key, byte *dat)
{
  unsigned long dat_size = dat[0] + 1; // PDC + 1
  ELOBJ *&returnal = SetKey(key);

  if (NULL != returnal->m_data)
  {
    delete[] returnal->m_data;
  }

  returnal->m_data = new byte[dat_size];
  memcpy(returnal->m_data, dat, dat_size);

  return (returnal);
}

//	キー文字列からデータ取得
byte *ELOBJ::GetData(const byte key) const
{
  int cmp = 0;
  //  文字列の一致、差分を確認
  cmp = m_key - key;

  if (0 == cmp)
  {
    //  発見
    return (this->m_data);
  }
  else if (0 < cmp)
  {
    //  右ノードを検索
    if (NULL == m_rightnode)
    {
      return (NULL);
    }
    else
    {
      return (m_rightnode->GetData(key));
    }
  }
  else
  {
    //  左ノードを検索
    if (NULL == m_leftnode)
    {
      return (NULL);
    }
    else
    {
      return (m_leftnode->GetData(key));
    }
  }
}

//////////////////////////////////////////////////////////////////////
//	検索
ELOBJ *&ELOBJ::Search(const byte key)
{
  int cmp = 0;
  //	文字列の一致、差分を確認
  cmp = m_key - key;

  if (0 == cmp)
  {
    //	発見
    return ((ELOBJ *&)*this);
  }
  else if (0 < cmp)
  {
    //	右ノードを検索
    if (NULL == m_rightnode)
    {
      return (m_rightnode);
    }
    else
    {
      return (m_rightnode->Search(key));
    }
  }
  else
  {
    //	左ノードを検索
    if (NULL == m_leftnode)
    {
      return (m_leftnode);
    }
    else
    {
      return (m_leftnode->Search(key));
    }
  }
}

//	配列らしいインターフェイス，左辺として使う用
//  代入不可, 読み取り専用と書き込み専用の違いをarduinoでは作れない？
// メモリリーク対策に，読み取り専用のみのインタフェースを公開
byte *ELOBJ::operator[](const byte key)
{
  ELOBJ *&returnal = Search(key);

  if (NULL == returnal)
  { // NULLなら新規作成になる
    ELOBJ *tmp = new ELOBJ(key);
    returnal = tmp;
  }

  return (returnal->m_data);
}

/*
  //	代入可だが，うまくやらないとメモリリークするので封印
  byte*&	ELOBJ::operator[]( byte key )
  {
  ELOBJ*& returnal = Search(key);

  if ( NULL == returnal ) { // NULLなら新規作成になる
    ELOBJ* tmp = new ELOBJ(key);
    returnal = tmp;
  }else{
    delete[] returnal->m_data;
  }

  return (returnal->m_data);
  }
*/

// 状態表示系
void ELOBJ::edt_print(const byte *edt)
{
  const byte *tmp = edt;
  Serial.print((int)tmp[0]);
  for (byte i = 1; i <= tmp[0]; i += 1)
  {
    Serial.print(", ");
    Serial.print((int)tmp[i]);
  }
  Serial.print("\n");
}

void ELOBJ::print() const
{
  if (m_key == 0x00)
  {
    return;
  }
  Serial.print((int)m_key);
  Serial.print(": ");
  Serial.print((int)m_data[0]);
  for (byte i = 1; i <= m_data[0]; i += 1)
  {
    Serial.print(", ");
    Serial.print((int)m_data[i]);
  }
  Serial.print("\n");
}

void ELOBJ::printAll() const
{
  //  右ノードがあればそれも表示
  if (NULL != m_rightnode)
  {
    m_rightnode->printAll();
  }

  print();

  //  左ノードがあればそれも表示
  if (NULL != m_leftnode)
  {
    m_leftnode->printAll();
  }
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
