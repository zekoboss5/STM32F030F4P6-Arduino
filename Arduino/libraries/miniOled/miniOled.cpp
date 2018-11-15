/*
V 0.2.0  miniOled.cpp

A VERY BASIC oled SSD1306 (0.96" I2C) library, compatible with SoftWire I2C library.
Designed initially to fit on STM32F030F4P6 board (16K flash, 48 MHz):
       for software I2C able to have custom pin selection
       for small flash memory footprint

1. Text. (Very) small characters.  Optional double width. Variable chr-spacing.
      page mode: always in one of 8 fixed horizontal lines ("pages" of SSD1306)
      horizontal mode: streaming over many lines (no interline spacing control).
2. Also big font 18x24 (inside 24x32) numbers only, still aligned to start on one of the 8 pages/lines.
3. <hr> horizontal line.
4. Load bitmap image.

I2C pins (software) configurable.

I2C output is unbuffered, one byte transfer at a time. Slow.
Derived by substantial hacking (down) of following:
   https://github.com/mistertwo/ozoled-oled-display-096-i2c-driver-library     2014/2015 

You need to download & install two other-party libraries
https://www.arduinolibraries.info/libraries/soft-wire   The 2.0.0 version of SoftWire
https://github.com/stevemarple/AsyncDelay   The AsyncDelay library, used by SoftWire
*/

#include "miniOled.h"
#include <SoftWire.h>
SoftWire Wire(PA6, PA5);  // default gpio (SoftWire needs a default)


// 6x8 Font ASCII 32 - 127 Implemented

const byte BasicFont[][5]  = {   // 5x8 in a 6x8 space.
	{0x00,0x00,0x00,0x00,0x00},
	{0x00,0x5F,0x00,0x00,0x00},
	{0x00,0x07,0x00,0x07,0x00},
	{0x14,0x7F,0x14,0x7F,0x14},
	{0x24,0x2A,0x7F,0x2A,0x12},
	{0x23,0x13,0x08,0x64,0x62},
	{0x36,0x49,0x55,0x22,0x50},
	{0x00,0x05,0x03,0x00,0x00},
	{0x1C,0x22,0x41,0x00,0x00},
	{0x41,0x22,0x1C,0x00,0x00},
	{0x08,0x2A,0x1C,0x2A,0x08},
	{0x08,0x08,0x3E,0x08,0x08},
	{0xA0,0x60,0x00,0x00,0x00},
	{0x08,0x08,0x08,0x08,0x08},
	{0x60,0x60,0x00,0x00,0x00},
	{0x20,0x10,0x08,0x04,0x02},
	{0x3E,0x51,0x49,0x45,0x3E},
	{0x00,0x42,0x7F,0x40,0x00},
	{0x62,0x51,0x49,0x49,0x46},
	{0x22,0x41,0x49,0x49,0x36},
	{0x18,0x14,0x12,0x7F,0x10},
	{0x27,0x45,0x45,0x45,0x39},
	{0x3C,0x4A,0x49,0x49,0x30},
	{0x01,0x71,0x09,0x05,0x03},
	{0x36,0x49,0x49,0x49,0x36},
	{0x06,0x49,0x49,0x29,0x1E},
	{0x00,0x36,0x36,0x00,0x00},
	{0x00,0xAC,0x6C,0x00,0x00},
	{0x08,0x14,0x22,0x41,0x00},
	{0x14,0x14,0x14,0x14,0x14},
	{0x41,0x22,0x14,0x08,0x00},
	{0x02,0x01,0x51,0x09,0x06},
	{0x32,0x49,0x79,0x41,0x3E},
	{0x7E,0x09,0x09,0x09,0x7E},
	{0x7F,0x49,0x49,0x49,0x36},
	{0x3E,0x41,0x41,0x41,0x22},
	{0x7F,0x41,0x41,0x22,0x1C},
	{0x7F,0x49,0x49,0x49,0x41},
	{0x7F,0x09,0x09,0x09,0x01},
	{0x3E,0x41,0x41,0x51,0x72},
	{0x7F,0x08,0x08,0x08,0x7F},
	{0x41,0x7F,0x41,0x00,0x00},
	{0x20,0x40,0x41,0x3F,0x01},
	{0x7F,0x08,0x14,0x22,0x41},
	{0x7F,0x40,0x40,0x40,0x40},
	{0x7F,0x02,0x0C,0x02,0x7F},
	{0x7F,0x04,0x08,0x10,0x7F},
	{0x3E,0x41,0x41,0x41,0x3E},
	{0x7F,0x09,0x09,0x09,0x06},
	{0x3E,0x41,0x51,0x21,0x5E},
	{0x7F,0x09,0x19,0x29,0x46},
	{0x26,0x49,0x49,0x49,0x32},
	{0x01,0x01,0x7F,0x01,0x01},
	{0x3F,0x40,0x40,0x40,0x3F},
	{0x1F,0x20,0x40,0x20,0x1F},
	{0x3F,0x40,0x38,0x40,0x3F},
	{0x63,0x14,0x08,0x14,0x63},
	{0x03,0x04,0x78,0x04,0x03},
	{0x61,0x51,0x49,0x45,0x43},
	{0x7F,0x41,0x41,0x00,0x00},
	{0x02,0x04,0x08,0x10,0x20},
	{0x41,0x41,0x7F,0x00,0x00},
	{0x04,0x02,0x01,0x02,0x04},
	{0x80,0x80,0x80,0x80,0x80},
	{0x01,0x02,0x04,0x00,0x00},
	{0x20,0x54,0x54,0x54,0x78},
	{0x7F,0x48,0x44,0x44,0x38},
	{0x38,0x44,0x44,0x28,0x00},
	{0x38,0x44,0x44,0x48,0x7F},
	{0x38,0x54,0x54,0x54,0x18},
	{0x08,0x7E,0x09,0x02,0x00},
	{0x18,0xA4,0xA4,0xA4,0x7C},
	{0x7F,0x08,0x04,0x04,0x78},
	{0x00,0x7D,0x00,0x00,0x00},
	{0x80,0x84,0x7D,0x00,0x00},
	{0x7F,0x10,0x28,0x44,0x00},
	{0x41,0x7F,0x40,0x00,0x00},
	{0x7C,0x04,0x18,0x04,0x78},
	{0x7C,0x08,0x04,0x7C,0x00},
	{0x38,0x44,0x44,0x38,0x00},
	{0xFC,0x24,0x24,0x18,0x00},
	{0x18,0x24,0x24,0xFC,0x00},
	{0x00,0x7C,0x08,0x04,0x00},
	{0x48,0x54,0x54,0x24,0x00},
	{0x04,0x7F,0x44,0x00,0x00},
	{0x3C,0x40,0x40,0x7C,0x00},
	{0x1C,0x20,0x40,0x20,0x1C},
	{0x3C,0x40,0x30,0x40,0x3C},
	{0x44,0x28,0x10,0x28,0x44},
	{0x1C,0xA0,0xA0,0x7C,0x00},
	{0x44,0x64,0x54,0x4C,0x44},
	{0x08,0x36,0x41,0x00,0x00},
	{0x00,0x7F,0x00,0x00,0x00},
	{0x41,0x36,0x08,0x00,0x00},
	{0x02,0x01,0x01,0x02,0x01},
	{0x02,0x05,0x05,0x02,0x00} 
};


// Big numbers font, from 0 to 9 - 72 bytes each.
const byte bigNumbers [][72] = {   // 18x24 inside a 24x32 cell.
{0xC0, 0xE0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
	 0xF0, 0xF0, 0xF0, 0xF0, 0xE0, 0xC0, 0xFF, 0xFF, 0xFF, 0xFF, 0x01,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xFF, 0xFF,
	   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x80, 0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00, 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0x03, 0x07,
	     0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
	      0x0F, 0x0F, 0x0F, 0x07, 0x03}, // 0

{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0,
	 0xF0, 0xF0, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00,
	   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00,
	     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x07,
	      0x07, 0x03, 0x00, 0x00, 0x00},  // 1

{0xE0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
	 0xF0, 0xF0, 0xF0, 0xF0, 0xE0, 0x00, 0x81, 0xC1, 0xC0, 0xC0, 0xC0,
	  0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xE1, 0xFF, 0xFF, 0xFF,
	   0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x87, 0x03, 0x03, 0x03, 0x03,
	    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x83, 0x81, 0x00, 0x07, 0x0F,
	     0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
	      0x0F, 0x0F, 0x0F, 0x07, 0x00}, // 2

{0xE0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
	 0xF0, 0xF0, 0xF0, 0xF0, 0xE0, 0x00, 0x81, 0xC1, 0xC0, 0xC0, 0xC0,
	  0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xE1, 0xFF, 0xFF, 0xFF,
	   0x7F, 0x00, 0x81, 0x83, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	    0x03, 0x03, 0x03, 0x87, 0xFF, 0xFF, 0xFF, 0xFE, 0x00, 0x07, 0x0F,
	     0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
	      0x0F, 0x0F, 0x0F, 0x07, 0x00},  // 3

{0xE0, 0xF0, 0xF0, 0xF0, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x30, 0xF0, 0xF0, 0xF0, 0xE0, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xE0,
	  0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xE0, 0xFF, 0xFF, 0xFF,
	   0xFF, 0x00, 0x01, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	    0x03, 0x03, 0x03, 0x07, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00,
	     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C,
	      0x0F, 0x0F, 0x0F, 0x07, 0x00},  // 4

{0xE0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
	 0xF0, 0xF0, 0xF0, 0xF0, 0xE0, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xE1,
	  0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC1,
	   0x81, 0x00, 0x81, 0x83, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	    0x03, 0x03, 0x03, 0x87, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x07, 0x0F,
	     0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
	      0x0F, 0x0F, 0x0F, 0x07, 0x00},  // 5

{0xE0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
	 0xF0, 0xF0, 0xF0, 0xF0, 0xE0, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xE1,
	  0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC1,
	   0x81, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x87, 0x03, 0x03, 0x03, 0x03,
	    0x03, 0x03, 0x03, 0x87, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x07, 0x0F,
	     0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
	      0x0F, 0x0F, 0x0F, 0x07, 0x00},  // 6

{0xE0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
	 0xF0, 0xF0, 0xF0, 0xF0, 0xE0, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xFF, 0xFF, 0xFF,
	   0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00,
	     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C,
	      0x0F, 0x0F, 0x0F, 0x07, 0x00},  // 7

{0xE0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
	 0xF0, 0xF0, 0xF0, 0xF0, 0xE0, 0x00, 0x7F, 0xFF, 0xFF, 0xFF, 0xE1,
	  0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xE1, 0xFF, 0xFF, 0xFF,
	   0x3F, 0x00, 0xFE, 0xFF, 0xFF, 0xFF, 0x87, 0x03, 0x03, 0x03, 0x03,
	    0x03, 0x03, 0x03, 0x87, 0xFF, 0xFF, 0xFF, 0xFE, 0x00, 0x07, 0x0F,
	     0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
	      0x0F, 0x0F, 0x0F, 0x07, 0x00},  // 8

{0xE0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
	 0xF0, 0xF0, 0xF0, 0xF0, 0xE0, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xE1,
	  0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xE1, 0xFF, 0xFF, 0xFF,
	   0xFF, 0x00, 0x01, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	    0x03, 0x03, 0x03, 0x07, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00,
	     0x00, 0x0c, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
	      0x0F, 0x0F, 0x0F, 0x07, 0x00},  // 9


{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	  0x00, 0x3C, 0x7E, 0x7E, 0x7E, 0x7E, 0x3C, 0x00, 0x00, 0x00, 0x00,
	   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xF8, 0xF8,
	    0xF8, 0xF8, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	     0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
	      0x00, 0x00, 0x00, 0x00, 0x00}  // ":"
};  // fortunately, this does not compile into the flash unless it gets referenced in your code.



// ====================== LOW LEVEL =========================

void OLED::sendCommand(byte command){
	Wire.beginTransmission(OLED_ADDRESS); // begin transmitting
	Wire.write(OLED_COMMAND_MODE);//data mode
	Wire.write(command);
	Wire.endTransmission();    // stop transmitting



void OLED::sendData(byte data){
	
	Wire.beginTransmission(OLED_ADDRESS); // begin transmitting
	Wire.write(OLED_DATA_MODE);//data mode
	Wire.write(data);
	Wire.endTransmission();    // stop transmitting
}


void OLED::printChar(char C, byte X, byte Y){

    byte i, ch;
	//Ignore unused ASCII characters. Modified the range to support multilingual characters.
    if(C < 32 || C > 127)
		C='*'; //star - indicate characters that can't be displayed

	if ( X < 128 )
		setCursorXY(X, Y);

	for (i=0; i<chrSpace; i++)
    	sendData(0);

    for(i=0; i<5; i++) {
       ch = BasicFont[C-32][i];
       sendData(ch);
       if (wideFont)
           sendData(ch); 
    }
}

void OLED::printString(const char *String, byte X, byte Y, byte numChar){

	if ( X < 128 )
		setCursorXY(X, Y);

	
	byte count=0;
    while(String[count] && count<numChar){
		printChar(String[count++]);  
	}

}

void OLED::printInt(int j, byte X, byte Y)
{
    char buffer[20];
    itoa(j,buffer,10); 
    printString(buffer, X, Y);
}




void OLED::printBigNumber(const char *number, byte X, byte Y, byte numChar){
// X 0 - 12      Y 0 - 4
// big number pixels: chr size 24 x 32

 // Y - page
	byte column = 0;
	byte count = 0;

	while(number[count] && count<numChar){
		setCursorXY(X, Y);
		
		for(byte i=0; i<72; i++) {
		    if(column==0)
		        {sendData(0);sendData(0);sendData(0);}
			// if character is not "0-9" or ':'
			if(number[count] < 48 || number[count] > 58)	
				sendData(0);
			else 				
				sendData(bigNumbers[number[count]-48][i]);
			if(column >= 17){
		        sendData(0);sendData(0);sendData(0);
				column = 0;
				setCursorXY(X, ++Y);
			}
			else				
				column++;
		}
		count++;
		X = X + 3;
		Y = Y - 4;
	}
}


byte OLED::printNumber(long long_num, byte X, byte Y){

	if ( X < 128 )
		setCursorXY(X, Y);

	byte char_buffer[10] = "";
	byte i = 0;
	byte f = 0; // number of characters

	if (long_num < 0) {
		f++;
		printChar('-');
		long_num = -long_num;
	} 
	else if (long_num == 0) {
		f++;
		printChar('0');
		return f;
	} 

	while (long_num > 0) {
		char_buffer[i++] = long_num % 10;
		long_num /= 10;
	}

	f += i;
	for(; i > 0; i--) {
		printChar('0'+ char_buffer[i - 1]);
	}
	
	return f;

}




byte OLED::printNumber(float float_num, byte prec, byte X, byte Y){

	if ( X < 128 )
		setCursorXY(X, Y);

// precision - use 6 maximum

	byte num_int = 0;
	byte num_frac = 0;
	byte num_extra = 0;
	
	long d = float_num; // get the integer part
	float f = float_num - d; // get the fractional part
	
	if (d == 0 && f < 0.0){
		printChar('-');
		num_extra++;
		printChar('0');
		num_extra++;
		f *= -1;
	}
	else if (d < 0 && f < 0.0){
		num_int = printNumber(d); // count how many digits in integer part
		f *= -1;
	}
	else{
		num_int = printNumber(d); // count how many digits in integer part
	}
	
	// only when fractional part > 0, we show decimal point
	if (f > 0.0){
		printChar('.');
		num_extra++;
		long f_shift = 1;
		if (num_int + prec > 8) 
			prec = 8 - num_int;
		for (byte j=0; j<prec; j++){
			f_shift *= 10;
		}
		num_frac = printNumber((long)(f*f_shift)); // count how many digits in fractional part
	}
	return num_int + num_frac + num_extra;
}


void OLED::drawBitmap(const byte *bitmaparray, byte X, byte Y, byte width, byte height){

// max width = 16
// max height = 8
	setCursorXY( X, Y );
	byte column = 0; 
	for(int i=0; i<width*8*height; i++) {  
		sendData(bitmaparray[i]);
		if(++column == width*8) {
			column = 0;
			setCursorXY( X, ++Y );
		} 
	}
}


// =================== High Level ===========================

void OLED::init(int sda, int scl){
	Wire.enablePullups();
    Wire.setRxBuffer(ibuffer, 60);  // common buffer is OK, but rx buffer not used anyway
    Wire.setTxBuffer(ibuffer, 60);
    if(sda>=0) // SDA SCL optional parameters. Otherwise defaults apply 
    {
		Wire.setSda(sda);   // non-default pins
		Wire.setScl(scl);
    }
	Wire.begin() ;   

	static const byte ini[] = {0xae,0xa6,0xAE,0xD5,0x80,0xA8,0x3F,0xD3,
		0x0,0x40 | 0x0,0x8D,0x14,0x20,0x00,      
        0xA0 | 0x1,0xC8,0xDA,0x12,0x81,0xCF,
        0xd9,0xF1,0xDB,0x40,0xA4,0xA6,0x2e,0x20,0x00};

    for (byte i=0; i<29;i++)
        sendCommand(ini[i]);
    clearDisplay();
    setPowerOn();
}


void OLED::setCursorXY(byte X, byte Y){
	// Y - 1 unit = 1 page (8 pixel rows)   0 - 15
	// X - 1 unit = 8 pixel columns         0 - 7

    sendCommand(0x00 + (8*X & 0x0F)); 		//set column lower address
    sendCommand(0x10 + ((8*X>>4)&0x0F)); 	//set column higher address
	sendCommand(0xB0 + Y); 					//set page address
}


void OLED::clearDisplay()	{
	for(byte page=0; page<8; page++) {	
        drawLine(page,0);	
	}
	setCursorXY(0,0);  
}


void OLED::setPowerOff(){
	sendCommand(OLED_CMD_DISPLAY_OFF);
}

void OLED::setPowerOn(){
	sendCommand(OLED_CMD_DISPLAY_ON);
}

void OLED::drawLine(int page, byte pattern)
{
	setCursorXY(0, page);     
	for(byte column=0; column<128; column++){  
		sendData(pattern);    
	}
}

void OLED::setPageMode(){
	addressingMode = PAGE_ADDRESSING;
	sendCommand(0x20); 				//set addressing mode
	sendCommand(PAGE_ADDRESSING); 	//set page addressing mode
}

void OLED::setHorizontalMode(){
	addressingMode = HORIZONTAL_ADDRESSING;
	sendCommand(0x20); 				//set addressing mode
	sendCommand(HORIZONTAL_ADDRESSING); 	//set page addressing mode
}

OLED Oled;  // Preinstantiate Object

