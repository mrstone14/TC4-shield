/*
 * derived from mcEEPROM.h
 *
 *  Created on: Jul 27, 2010
 *      Author: Jim Gallt
 *      Copyright (C) 2010 MLG Properties, LLC
 *      All rights reserved.
 *
 */
//-------------------------------------------
// Revision history
//
// 20100731  Version 1.00
// 20110903  Support for calibration block for TC4
// 20120126  Arduino 1.0 compatibility
//  (thanks and acknowledgement to Arnaud Kodeck for his code contributions).
// 20190106  modified for ESP8266 own EEPROM, by Renato Aranghelovici

//
// ------------------------------------------------
// *** BSD License ***
// ------------------------------------------------------------------------------------------
// Copyright (c) 2011, MLG Properties, LLC
// All rights reserved.
//
// Contributor:  Jim Gallt
//
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
//
//   Redistributions of source code must retain the above copyright notice, this list of
//   conditions and the following disclaimer.
//
//   Redistributions in binary form must reproduce the above copyright notice, this list
//   of conditions and the following disclaimer in the documentation and/or other materials
//   provided with the distribution.
//
//   Neither the name of the copyright holder nor the names of its contributors may be
//   used to endorse or promote products derived from this software without specific prior
//   written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
// OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// ------------------------------------------------------------------------------------------

#ifndef ESPEEPROM_H_
#define ESPEEPROM_H_

#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include <EEPROM.h>
#define BUFFER_SIZE 512 // arbitrary choice, to accomodate the 400 bytes profile size in a single commit
#define PAGE_SIZE 512 // same as above
#define MAX_ADDR FLASH_SECTOR_SIZE - 1 // 1000H = 4K EEPROM

#define STR_MAX 128 // default maximum string variable size

#define TC4_CAL_ADDR 0x00 // calibration block for TC4 is first spot in memory

// eeprom calibration data structure for TC4
struct calBlock {
  char PCB[40]; // identifying information for the board
  char version[16];
  float cal_gain;  // calibration factor of ADC at 50000 uV
  int16_t cal_offset; // uV, probably small in most cases
  float T_offset; // temperature offset (Celsius) at 0.0C (type T)
  float K_offset; // same for type K
};

class espEEPROM {
public:

	espEEPROM( uint8_t select = 0 );

	uint16_t write( uint16_t ptr, uint8_t barray[], uint16_t count = 1); // returns number written
	uint16_t write( uint16_t ptr, char str[] );
	uint16_t write( uint16_t ptr, float f[] );
	uint16_t write( uint16_t ptr, double xx[] );
	uint16_t write( uint16_t ptr, int16_t m[] );
	uint16_t write( uint16_t ptr, uint16_t m[] );
	uint16_t write( uint16_t ptr, int32_t m[] );
	uint16_t write( uint16_t ptr, uint32_t m[] );

	uint16_t read( uint16_t ptr, uint8_t barray[], uint16_t count = 1 ); // returns number read
	uint16_t read( uint16_t ptr, char str[], uint16_t max = STR_MAX );
	uint16_t read( uint16_t ptr, float f[] );
	uint16_t read( uint16_t ptr, double xx[] );
	uint16_t read( uint16_t ptr, int16_t m[] );
	uint16_t read( uint16_t ptr, uint16_t m[] );
	uint16_t read( uint16_t ptr, int32_t m[] );
	uint16_t read( uint16_t ptr, uint32_t m[] );

protected:
	uint16_t bytesOnPage( uint16_t ptr ); // returns bytes remaining till end of page

private:
	uint8_t chip_addr;
};

// reads calibration information from EEPROM.  Returns FALSE on error.
bool readCalBlock( espEEPROM& eeprm, calBlock& cal );

#endif /* ESPEEPROM_H_ */
