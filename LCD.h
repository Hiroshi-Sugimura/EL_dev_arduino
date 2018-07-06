//////////////////////////////////////////////////////////////////////
// LCD class
//	Copyright (C) Hiroshi SUGIMURA 2018.06.29
//////////////////////////////////////////////////////////////////////
#ifndef __LCD_H__
#define __LCD_H__
#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif


#define I2Caddr 0x3e

class LCD
{
  private:
    byte contrast; //< default 30

  public:
    LCD();  // constracter
    void begin(void) const;  //< begin
    void cmd(byte x) const;  //< write
    void clear(void) const; //< clear
    void DisplayOff(void) const; //< off
    void DisplayOn(void) const;  //< on
    void contdata(byte x) const; //< sub function
    void lastdata(byte x) const;
    void printStr(const char *s) const; //< print
    void setCursor(byte x, byte y) const; //< pos
};

#endif
//////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////
