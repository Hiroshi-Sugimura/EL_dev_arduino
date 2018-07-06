//////////////////////////////////////////////////////////////////////
// LCD class
//	Copyright (C) Hiroshi SUGIMURA 2018.06.29
//////////////////////////////////////////////////////////////////////
#include <Wire.h>
#include "LCD.h"

LCD::LCD()
{
  contrast = 30;
}

//< begin
void LCD::begin(void) const
{
  Wire.begin();
  cmd(0x38);
  cmd(0x39);
  cmd(0x04);
  cmd(0x14);
  cmd(0x70 | (contrast & 0xF));
  cmd(0x5C | ((contrast >> 4) & 0x3));
  cmd(0x6C);
  delay(200);
  cmd(0x38);
  cmd(0x0C);
  cmd(0x01);
  delay(2);
}

//< write
void LCD::cmd(byte x) const
{
  Wire.beginTransmission(I2Caddr);
  Wire.write(0x00); // C0=0, RS=0
  Wire.write(x);
  Wire.endTransmission();
}

//< clear
void LCD::clear(void) const
{
  cmd(0x01);
}

//< off
void LCD::DisplayOff(void) const
{
  cmd(0x08);
}

//< on
void LCD::DisplayOn(void) const
{
  cmd(0x0C);
}

//< sub function
void LCD::contdata(byte x) const
{
  Wire.write(0xC0);
  Wire.write(x);
}

void LCD::lastdata(byte x) const
{
  Wire.write(0x40);
  Wire.write(x);
}

//< print
void LCD::printStr(const char *s) const
{
  Wire.beginTransmission(I2Caddr);
  while (*s)
  {
    if (*(s + 1))
    {
      contdata(*s);
    }
    else
    {
      lastdata(*s);
    }
    s++;
  }
  Wire.endTransmission();
}

//< pos
void LCD::setCursor(const byte x, const byte y) const
{
  cmd(0x80 | (y * 0x40 + x));
}
//////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////
