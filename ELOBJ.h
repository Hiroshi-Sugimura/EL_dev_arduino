////////////////////////////////////////////////////////////////////////////////
// ECHONET Lite protocol object
//  Copyright (C) Hiroshi SUGIMURA 2013.09.27
////////////////////////////////////////////////////////////////////////////////
#ifndef __ELOBJ_H__
#define __ELOBJ_H__
#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif


//////////////////////////////////////////////////////////////////////
//	クラスの定義
class ELOBJ
{
    //	メンバ変数定義
  protected:
    byte	m_key;	//	キー文字列、この文字列を使用して検索
    ELOBJ*			m_leftnode;		//	左の子供ノード
    ELOBJ*			m_rightnode;		//	右の子供ノード
    byte*	m_data;	//	データ

  public:
    //	構築
    ELOBJ( byte key = 0x00, byte* dat = NULL, unsigned long dat_size = 0 );
    //	消滅
    virtual	~ELOBJ();

    //	キー文字列取得
    virtual	byte	GetKey(void) const;
    //	データ取得
    virtual	byte*	GetData(void) const;
    //	左の子供ノード取得
    virtual	ELOBJ*			GetLeftNode(void) const;
    //	右の子供ノード取得
    virtual	ELOBJ*			GetRightNode(void) const;

    //	キー文字列追加
    virtual	ELOBJ*&		SetKey( const byte key);
    //	データ更新
    virtual ELOBJ*&   SetData( const byte key, byte* dat);

    //	キー文字列からデータ取得
    virtual	byte*	GetData( const byte key) const;

    //	検索
    virtual	ELOBJ*&		Search( const byte key );
    // 読み取り可能なオブジェクトを渡す検索と，読み取り不可の検索をわたすのと変える必要がある。

    //	配列らしいインターフェイス
    // arduinoは読み取り，書き込みの区別を作れない？
    byte*  operator[]( const byte key );  // 読み取り
    // byte*& operator[]( byte key ); // 書き込み

    // 状態表示系
    static void edt_print( const byte* edt );
    void print() const; // this print edt
    void printAll() const;  // all print edt
};


#endif
//////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////
