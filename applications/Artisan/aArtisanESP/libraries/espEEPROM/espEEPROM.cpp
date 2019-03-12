/*
 * derived from mcEEPROM.cpp
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

#undef epDEBUG

#ifdef epDEBUG
#define DPRINTLN(s,var) Serial.print(s);Serial.print(var,DEC);Serial.println()
#define DPRINT(s,var) Serial.print(s);Serial.print(var,DEC);Serial.print(",")
#endif

#include "espEEPROM.h"


#if defined(ARDUINO) && ARDUINO >= 100
#define _READ read
#define _WRITE write
#else
#define _READ receive
#define _WRITE send
#endif

// --------------------------
espEEPROM::espEEPROM( uint8_t select ) {

EEPROM.begin(4096);

}

// -------------------------- returns bytes available on this page
uint16_t espEEPROM::bytesOnPage( uint16_t ptr ) {
	return PAGE_SIZE - ( ptr % PAGE_SIZE );
}

// -------------------------- read array of bytes
uint16_t espEEPROM::read( uint16_t ptr, uint8_t barray[], uint16_t count ) {
		
	uint16_t j, k, n;
//	if( count > MAX_COUNT )
//		return 0;
	if( ptr != 0 && count >( MAX_ADDR - ptr ) + 1 ) // don't go past the end
		count = ( MAX_ADDR - ptr ) + 1;
	k = 0;
	while( k < count ) {
		n = BUFFER_SIZE; // don't exceed the Wire buffer size
		if( n + k >= count )  // don't exceed requested byte count
			n = count - k;
#ifdef epDEBUG
			DPRINT("ptrr = ", ptr + k);
#endif
			for( j = 0; j < n ; j++ ) {  // read the next "n" bytes
				EEPROM.get(ptr + k, barray[k]);

#ifdef epDEBUG
		DPRINT("-", barray[k]);
#endif
				++k;
			} // end for

			// ptr += n;

#ifdef epDEBUG
			DPRINTLN("-bytes read size: ", k);
#endif
	}

	return k;

}
// -------------------------- read string
uint16_t espEEPROM::read( uint16_t ptr, char str[], uint16_t max ) {

	
	uint16_t j, k, n;
	uint8_t b;
	uint8_t done = 0;

	if( ptr != 0 && max > ( MAX_ADDR - ptr ) + 1  ) // don't go past the end
		max = ( MAX_ADDR - ptr ) + 1;

	k = 0;
	while( done == 0 ) {
		n = BUFFER_SIZE; // don't exceed the Wire buffer size
		if( n > max - k )  // don't exceed max byte count
			n = max - k;

#ifdef epDEBUG
			DPRINT("ptr = ", ptr);
#endif
			for( j = 0; j < n && done == 0 ; j++ ) {  // read the next "n" bytes

				EEPROM.get(ptr + k, b);

				str[k] = (char)b;
#ifdef epDEBUG
				DPRINT(".", str[k]);
#endif
				++k;
				if( k == max ) {
					done = 1;
					str[k-1] = '\0'; // force string termination
				} else
				if( (char)b == '\0' )
					done = 1;
			} // end for
#ifdef epDEBUG
			DPRINT(".string read len: ", k);
#endif

			// ptr += n;


	} // end while not done
	return k;
	
}

// --------------------------- read float
uint16_t espEEPROM::read( uint16_t ptr, float f[] ) {
	return read( ptr, (uint8_t*)f, sizeof( float ) );
}

// --------------------------- read double
uint16_t espEEPROM::read( uint16_t ptr, double d[] ) {
	return read( ptr, (uint8_t*)d, sizeof( double ) );
}

// --------------------------- read integer
uint16_t espEEPROM::read( uint16_t ptr, int16_t m[] ) {
	return read( ptr, (uint8_t*)m, sizeof( int16_t ));
}

// --------------------------- read unsigned integer
uint16_t espEEPROM::read( uint16_t ptr, uint16_t m[] ) {
	return read( ptr, (uint8_t*)m, sizeof( uint16_t ));
}

// --------------------------- read long integer
uint16_t espEEPROM::read( uint16_t ptr, int32_t m[] ) {
	return read( ptr, (uint8_t*)m, sizeof( int32_t ));
}

// --------------------------- read unsigned long integer
uint16_t espEEPROM::read( uint16_t ptr, uint32_t m[] ) {
	return read( ptr, (uint8_t*)m, sizeof( uint32_t ));
}

// -------------------------- write array of bytes
uint16_t espEEPROM::write( uint16_t ptr, uint8_t barray[], uint16_t count ) {

#ifdef epDEBUG
	DPRINTLN("count = ", count );
#endif
//	if( count > MAX_COUNT )
//		return 0;
	uint16_t j,k,n;
	k = 0; 
	while( k < count ) {
		// next, fill up the remaining buffer with data
#ifdef epDEBUG
		DPRINTLN("ptrw = ",ptr);
#endif
		n = bytesOnPage( ptr ); // cannot write across page boundaries
		if( n > BUFFER_SIZE - 2 ) // don't exceed Wire buffer
			n = BUFFER_SIZE - 2;
		if( n > count - k ) // don't exceed requested byte count
			n = count - k;
		if( ptr != 0 && n > ( MAX_ADDR - ptr ) + 1 ) // cannot allow n == 0
			n = ( MAX_ADDR - ptr ) + 1;
#ifdef epDEBUG
		DPRINTLN("nw = ",n);
#endif
		for( j = 0; j < n; j++ ) {
#ifdef epDEBUG
			DPRINT("kw = ",k);
			DPRINTLN("bytew = ",barray[k]);
#endif

			EEPROM.put(ptr + k, barray[k]);

			k++;
		}

		EEPROM.commit();

#ifdef epDEBUG
		// verify write in debug mode
		delay(100);
		byte check;
		for (j = 0; j < n; j++) {
			EEPROM.get(ptr + j, check);
			if (check != barray[j]) {
				DPRINTLN("Missmatch at ", j);
				DPRINT("write-", barray[j]); DPRINTLN("read-", check);
				return j;
			}
		}
		DPRINTLN("Verify done ", k);
#endif

		if( ptr >= ( MAX_ADDR - n ) + 1 )
			break;
		ptr += n;
	} // end while
	return k;
	
}

// ----------------------------------- write string
uint16_t espEEPROM::write( uint16_t ptr, char str[] ) {
	int len = strlen ( str ) + 1;
#ifdef epDEBUG
	DPRINTLN("STR length = ",len);
#endif
	return write( ptr, (uint8_t*)str, len );
}

// --------------------------------------- write float
uint16_t espEEPROM::write( uint16_t ptr, float f[] ) {
	return write( ptr, (uint8_t*)f, sizeof( float ) );
}

// --------------------------------------- write double
uint16_t espEEPROM::write( uint16_t ptr, double f[] ) {
	return write( ptr, (uint8_t*)f, sizeof( double ) );
}

// ---------------------------------------- write integer
uint16_t espEEPROM::write( uint16_t ptr, int16_t m[] ) {
	return write( ptr, (uint8_t*)m, sizeof( int16_t ) );
}

// ---------------------------------------- write unsigned integer
uint16_t espEEPROM::write( uint16_t ptr, uint16_t m[] ) {
	return write( ptr, (uint8_t*)m, sizeof( uint16_t ) );
}

// ---------------------------------------- write long integer
uint16_t espEEPROM::write( uint16_t ptr, int32_t m[] ) {
	return write( ptr, (uint8_t*)m, sizeof( int32_t ) );
}

// ---------------------------------------- write unsigned long integer
uint16_t espEEPROM::write( uint16_t ptr, uint32_t m[] ) {
	return write( ptr, (uint8_t*)m, sizeof( uint32_t ) );
}

// reads calibration information from EEPROM.  Returns FALSE on error.
bool readCalBlock( espEEPROM& eeprm, calBlock& caldata ){
  // read calibration and identification data from eeprom
  // this is not real strong error checking, but should be OK in most situations
  uint16_t len;
  len = eeprm.read( TC4_CAL_ADDR, (uint8_t*) &caldata, sizeof( caldata) );
  return( (len == sizeof( caldata )) && (strncmp( "TC4", caldata.PCB, 3 ) == 0 ) );
}

#undef _READ
#undef _WRITE

