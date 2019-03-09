/*
 * adcButton.cpp
 *
 *  Created on:  Sep  8, 2010
 *  Modified on: Dec 17-26, 2018
 */

// *** BSD License ***
// ------------------------------------------------------------------------------------------
// Copyright (c) 2010, MLG Properties, LLC
// All rights reserved.
//
// Contributor:  Jim Gallt, Renato Aranghelovici
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
//   Neither the name of the MLG Properties, LLC nor the names of its contributors may be 
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

// Revision history:
//  20120126: Arduino 1.0 compatibility
//   (thanks and acknowledgement to Arnaud Kodeck for his code contributions).
//  20150224: Changed function result to boolean for anyPressed().
//            Needed for compatibility with Arduino 1.60
//  20181217: New implementation as adcButtons, for buttons connected as a resistor ladder, read by ESP2866 ADC0
//            https://en.wikipedia.org/wiki/Resistor_ladder
//            Also changed logic to detect long press events - Renato Aranghelovici

#include "adcButton.h"

#if defined(ARDUINO) && ARDUINO >= 100
#define _READ read
#define _WRITE write
#else
#define _READ receive
#define _WRITE send
#endif

// ------------------------------------------ base class methods
adcButtonBase::adcButtonBase() {
  n = 0;
  bits = 0;
}

boolean adcButtonBase::keyPressed( uint8_t key ) {
  return ( prevstable ) & ( 1 << key );
}

boolean adcButtonBase::keyChanged( uint8_t key ) {
  return ( changed ) & ( 1 << key );
}

byte adcButtonBase::getPressedLongKeys() { // uint8_t key
	
	if (stableStart > 0 && (millis() - stableStart) > LONG_PRESS_INTERVAL) {
		nextCheck = millis() + looptime; // inactive window after long press, to allow button finger release, and no double click
		stableStart = 0; // reset long press start
		// Serial.print("# long press key info read and reset "); Serial.println(millis());
		return prevstable;
	} else {
		stableStart = 0; // reset long press start
		return 0;
	}
}

bool blnLongPressPending = false;

uint8_t adcButtonBase::readButtons() {

	if (blnLongPressPending) {
		// check if the key that caused previous long press trigger has debounced
		byte buttons = rawRead();
		if (prevstable & buttons) {
			//Serial.print("# key still pressed "); Serial.println(prevstable & buttons);
			return false;
		}
		else {
			// Serial.print("# long press key released "); Serial.println(millis());
			blnLongPressPending = false;
		}
	}

	uint32_t ms = millis();
	if (ms >= nextCheck) {
		debounce();
		nextCheck = ms + PERIOD;
		// changed logic to return on trailing edge only
		// required for long press detection
		if (changed > 0 && changed == stable) { // start long press interval counting
			stableStart = ms;
			//Serial.print("started stable interval at "); Serial.println(ms);
		}
		else if (changed == 0 && stable > 0 && stableStart > 0 && ((ms - stableStart) > LONG_PRESS_INTERVAL)) {// forced return on long press
			//Serial.print("forced exit, stable-prevstable "); Serial.print(stable); Serial.println(-prevstable);
			//stableStart = 0; // reset long press start
			changed = stable;
			stable = 0;
			blnLongPressPending = true;
		} 
		return (changed > 0 && stable == 0);
	}
	else return 0;  // if a new value was not read, than nothing can change
}

boolean adcButtonBase::anyPressed() {
  return !( stable == 0 );
}

// ------------------debounce code from www.ganssle.com/debouncing-pt2.htm
// ------------------debounces 8 switches at once (leading edge only)
// call this every PERIOD
void adcButtonBase::debounce() {
  uint8_t i, j;
  //uint8_t 
  prevstable = stable;
  state[sidx++] = rawRead();
  j = 0xFF; // all ones
  for( i = 0; i < NCHECKS; i++ )
    j &= state[i];
// on exit from loop, a j bit will be 1 only if all NCHECK readings are 1
  stable = j; // if switch press is stable, bit = 1
  changed = j ^ prevstable; // XOR j with the previous stable to pick up changes
  if( sidx >= NCHECKS )
    sidx = 0; // circular buffer
}

// ----------------------------------------------------- 
void adcButtonPE16::begin( uint8_t N, uint8_t addr ) {

// For ADC based buttons approach, do nothing

}

// ------------------------------------------
uint8_t adcButtonPE16::rawRead() {

	// buttons are connecting the ADC input of MCU to the outputs of a resistor ladder.
	// like this: https://i.stack.imgur.com/Hqt2f.png
	// R6 should be at least 10x times bigger than R1...R5

	int adc = analogRead(A0);
	bits = (( adc + 95) / 190); // translate 200, 400, 600, 800 adc values to 1...4
	//if (bits > 0) { Serial.print("# switch adc-bits "); Serial.println(adc); Serial.println(-bits); }
	//return 2^(bits-1);

	switch (bits) {
		case 1:    
		  return 1; // UP/PREV/PLUS/INCR
		  break;
		case 2:    
		  return 4; // ENTER/START/STOP
		  break;
		case 3:    
		  return 2; // DOWN/NEXT/MINUS/DECR
		  break;
		case 4:    
		  return 8; // MODE/SETTINGS
		  break;
	}

	return 0;
};

// ----------------------------------------------
void adcButtonPE16::ledUpdate( uint8_t b3) {
	// no LEDs
}

#undef _READ
#undef _WRITE

// ***********************************************************************************
