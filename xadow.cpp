/*
  xadow.cpp
  2013 Copyright (c) Seeed Technology Inc.  All right reserved.

  Author:Loovee
  2013-6-17
 
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <Arduino.h>
#include <Wire.h>
#include <Streaming.h>

#include "xadowDfs.h"
#include "xadow.h"

/*********************************************************************************************************
** Function name:           init
** Descriptions:            init
*********************************************************************************************************/
void xadow::init()
{
#if EN_BARO
    Wire.begin();
#endif

}

/*********************************************************************************************************
** Function name:           init_io
** Descriptions:            init_io
*********************************************************************************************************/
void xadow::init_io()
{
    
}


/*********************************************************************************************************
** Function name:           bmp085Read
** Descriptions:            bmp085Read
*********************************************************************************************************/
#if EN_BARO
char xadow::bmp085Read(unsigned char address)
{
    unsigned char data;
    
    cout << "read" << endl;
    
    Wire.beginTransmission(ADDRBARO);
    Wire.write(address);
    Wire.endTransmission();

    Wire.requestFrom(ADDRBARO, 1);
    while(!Wire.available());
    return Wire.read();
}
#endif

/*********************************************************************************************************
** Function name:           bmp085ReadInt
** Descriptions:            bmp085ReadInt
*********************************************************************************************************/
#if EN_BARO
int xadow::bmp085ReadInt(unsigned char address)
{
    unsigned char msb, lsb;
    
    cout << "int" << endl;
    
    Wire.beginTransmission(ADDRBARO);
    Wire.write(address);
    Wire.endTransmission();
    Wire.requestFrom(ADDRBARO, 2);
    
    cout << "int 2" << endl;
    while(Wire.available()<2);
    msb = Wire.read();
    lsb = Wire.read();
    
    cout << "int 3" << endl;
    return (int) msb<<8 | lsb;
}
#endif

/*********************************************************************************************************
** Function name:           getBaro
** Descriptions:            getBaro
*********************************************************************************************************/
#if EN_BARO
long xadow::getBaro()
{

    long b5;
    long x1, x2;
    unsigned int ac5 = bmp085ReadInt(0xB2);
    int ac6 = bmp085ReadInt(0xB4);
    int mc = bmp085ReadInt(0xBC);
    int md = bmp085ReadInt(0xBE);

    // make ut
    unsigned int ut;
    // Write 0x2E into Register 0xF4
    // This requests a temperature reading
    Wire.beginTransmission(ADDRBARO);
    Wire.write(0xF4);
    Wire.write(0x2E);
    Wire.endTransmission();
    // Wait at least 4.5ms
    delay(5);
    // Read two bytes from registers 0xF6 and 0xF7
    ut = bmp085ReadInt(0xF6);

    x1 = (((long)ut - (long)ac6)*(long)ac5) >> 15;
    x2 = ((long)mc << 11)/(x1 + md);
    b5 = (x1 + x2);

    long x3, b3, b6, p;
    unsigned long b4, b7;

    int ac1 = bmp085ReadInt(0xAA);
    int ac2 = bmp085ReadInt(0xAC);
    int ac3 = bmp085ReadInt(0xAE);
    unsigned int ac4 = bmp085ReadInt(0xB0);
    int b1 = bmp085ReadInt(0xB6);
    int b2 = bmp085ReadInt(0xB8);

    b6 =  b5-4000;
    // Calculate B3
    x1 = (b2 * (b6 * b6)>>12)>>11;
    x2 = (ac2 * b6)>>11;
    x3 = x1 + x2;
    b3 = (((((long)ac1)*4 + x3)<<0) + 2)>>2;

    // Calculate B4
    x1 = (ac3 * b6)>>13;
    x2 = (b1 * ((b6 * b6)>>12))>>16;
    x3 = ((x1 + x2) + 2)>>2;
    b4 = (ac4 * (unsigned long)(x3 + 32768))>>15;

    unsigned char msb, lsb, xlsb;
    unsigned long up = 0;
    // Request a pressure reading w/ oversampling setting
    Wire.beginTransmission(ADDRBARO);
    Wire.write(0xF4);
    Wire.write(0x34 + (0<<6));
    Wire.endTransmission();
    // Wait for conversion, delay time dependent on 0
    delay(2 + (3<<0));
    // Read register 0xF6 (MSB), 0xF7 (LSB), and 0xF8 (XLSB)
    msb     = bmp085Read(0xF6);
    lsb     = bmp085Read(0xF7);
    xlsb    = bmp085Read(0xF8);
    up = (((unsigned long) msb << 16) | ((unsigned long) lsb << 8) | (unsigned long) xlsb) >> (8-0);

    b7 = ((unsigned long)(up - b3) * (50000>>0));

    p = (b7<0x80000000) ? (b7<<1)/b4 : (b7/b4)<<1;
    x1 = (p>>8) * (p>>8);
    x1 = (x1 * 3038)>>16;
    x2 = (-7357 * p)>>16;
    p += (x1 + x2 + 3791)>>4;

    return p;

}
#endif

xadow Xadow;

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/