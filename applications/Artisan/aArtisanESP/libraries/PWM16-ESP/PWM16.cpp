// Timer1 and timer2 PWM control
// Version date: July 22, 2011
// Revision history:
//  20120126: Arduino 1.0 compatibility
//  20190104: adapted for ESP platform: ticker based slow on-off PWM, and fast analog IO3 style PWM
//  20190417: added preliminary support for ESP32

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

#include "PWM16.h"

#ifdef ESP8266

#define TIMER_INTERVAL_MS      5

#include <Ticker.h>  //Ticker Library

Ticker slowPWM;
//byte pwmACounter = 0;
volatile byte pwmA;
//byte lastPWMA;
//byte next_change_A; 
//byte next_status_A; 
volatile bool pwmAchanged = false;
// unsigned long pwmACounter;
volatile uint16_t pwmA1PulseLen , pwmA0PulseLen;

#ifdef pwmOutB
byte pwmBCounter = 0;
byte pwmB;
#endif // pwmOutB

volatile float pwmCycle;

bool pwmOffset50; // used for parallel diode mod, when Pwm range is always 50-100%
volatile unsigned long lastChangeStateMs;
volatile byte psA;
volatile int offset;

volatile uint32_t pwmOutAStatus = 0;
static char hdr[50];

static bool started = false;
//#define PWM_DEBUG

void changeStateA() {

	if (pwmA == 0 || pwmCycle == 0) {
		digitalWrite(pwmOutA, 0);
		psA = 0;
		//pinMode(pwmOutA, INPUT);
		return;
	}

	if (pwmAchanged) {
		pwmA1PulseLen = pwmCycle * (float)pwmA * 100 / pwmDutyMax * 10 - 1;
		pwmA0PulseLen = pwmCycle * 1000 - pwmA1PulseLen;
		//digitalWrite(pwmOutA, 0);
		//if (psA == 1) Serial.println();
#ifdef PWM_DEBUG
		Serial.print(millis()); Serial.print('\t'); Serial.print(pwmA); Serial.print("\t\t\t\t\t\t"); Serial.println(pwmA1PulseLen);
#endif // PWM_DEBUG
		pwmAchanged = false;
		lastChangeStateMs = millis();
		psA = digitalRead(pwmOutA);
		return;
	}

	//if (millis() - lastChangeStateMs > 20) {
	int lasttick = millis() - lastChangeStateMs;
	if (lasttick < TIMER_INTERVAL_MS - 1) return;

	lastChangeStateMs = millis();

	byte psA = digitalRead(pwmOutA);
#ifdef PWM_DEBUG
	Serial.print(psA);
#endif
	unsigned long pulse = millis() % (int)(pwmCycle * 1000);
	if (pulse >= 200) pulse -= 200; else pulse += 800;
	//Serial.print("pls: ");  Serial.print(pulse); Serial.print("; 1len: ");  Serial.print(pwmA1PulseLen); Serial.print("; ofs: ");  Serial.print(offset); Serial.print("; sA: "); Serial.println(psA); // Serial.print('-');
	if (pulse >= 10 && pulse < pwmA1PulseLen && psA == 0) { // 
			digitalWrite(pwmOutA, 1);
			psA = 1;
#ifdef PWM_DEBUG
			//Serial.println(millis());
#endif
			if (pulse <= 20) offset = pulse; else offset = 0;
			sprintf(hdr, "%d\t\t%d\t%d\t", millis(), pulse, psA);
	} // switch to 1
	else if (psA == 1 && pulse >= (pwmA1PulseLen + offset)) {
			digitalWrite(pwmOutA, 0);
			psA = 0;
#ifdef PWM_DEBUG
			Serial.println(millis());
			Serial.print(hdr);
			Serial.print(millis()); Serial.print('\t'); Serial.print(pulse); Serial.print('\t'); Serial.print(psA); Serial.print('\t'); Serial.println(pulse - offset); // Serial.print('-');
#endif
	} // switch to 0
	else {
			//Serial.print("pulse: ");  Serial.print(pulse); Serial.print("; psA: "); Serial.println(psA); // Serial.print('-');
			// do nothing, already right level
			// digitalWrite(pwmOutA, psA == 0 ? 1 : 0);
	}

} // end changeState

#ifdef pwmOutB
void changeStateB() {
	// ... TBD copy and adapt code from above
}
#endif // pwmOutB


char timerMessage[50];

#endif // ESP8266


//--------------------------------------------------------
// Constructor

PWM16::PWM16() {
  _pwmF = pwmN2sec; // default 2 sec
}


//--------------------------------------------------------
// Set up frequency / period and timer control registers
// pwmF = fixed time base frequency value
// This method must be called before using timer.  For some reason
// the initialization code does not work in the constructor.  fixme

void PWM16::Setup( unsigned int pwmF ) {
  _pwmF = pwmF;

#if (defined ESP8266 || defined ESP32)
#else
  noInterrupts();
  // non inverting, fast PWM, TOP is in ICR1
  TCCR1A = _BV(COM1A1) | _BV(COM1B1) | _BV(WGM11); 
  // fast PWM, TOP in ICR1, prescale N = 1024
  TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS12) | _BV(CS10);
  ICR1 = _pwmF;
  interrupts();
#endif // platform dep. code

}

//---------------------------------------------------------------
// Reset timer1 to its default state

void PWM16::Reset() {

#ifdef ESP8266
	noInterrupts();
	slowPWM.detach();
	digitalWrite(pwmOutA, 0);
	started = false;
	//pinMode(pwmOutA, INPUT);
	interrupts();
#ifdef pwmOutB
	digitalWrite(pwmOutB, 0);
	//pinMode(pwmOutB, INPUT);
#endif // pwmOutB
#elif defined ESP32
	// ... TBD
#else
  // first, disable timer1
  noInterrupts();
  TCCR1A = 0; 
  TCCR1B = 0; 
  interrupts();

  // next, reset timer1 to default values
  noInterrupts();
  TCCR1A = _BV(WGM10);             // 8-bit PWM, phase correct
  TCCR1B = _BV(CS11) | _BV(CS10);  // prescale = 64
  interrupts();
#endif // platforms
}

//--------------------------------------------------------------------------
// pwmF = fixed time base frequency value
// dutyA, dutyB = duty cycles, in percent
// dutyA = 0 and dutyB = 0 turns off timer 1
//

void PWM16::Out(unsigned int dutyA, unsigned int dutyB) {

	unsigned long nn;
	unsigned int nA;
	unsigned int nB;
	unsigned long pwmN1;

	// trap logic errors safely
	if (dutyA > pwmDutyMax) dutyA = pwmDutyError;
#ifdef pwmOutB
	if (dutyB > pwmDutyMax) dutyB = pwmDutyError;
#endif // pwmOutB

	// special case: force no output for zero duty cycle
	// otherwise will get 1 clock cycle ON in most modes
	if (dutyA != 0) pinMode(pwmOutA, OUTPUT); else pinMode(pwmOutA, INPUT);
#ifdef pwmOutB
	if (dutyB != 0) pinMode(pwmOutB, OUTPUT); else pinMode(pwmOutB, INPUT);
#endif // pwmOutB


#ifdef ESP8266
	if (pwmA == dutyA) return; // same value, no change

	pwmA = dutyA;
	pwmCycle = (float)_pwmF / (float)pwmN1Hz;
	pwmAchanged = true;
	/*if (pwmOffset50)
		onoffA = pwmCycle * (float)(pwmA - 50);
	else
		onoffA = pwmCycle * (float)pwmA;*/

	slowPWM.attach_ms(TIMER_INTERVAL_MS, changeStateA);

#elif defined ESP32
	// ... TBD
#else // Arduino
	// change timer registers only if there is something to do
	if (dutyA != 0 || dutyB != 0) {
		pwmN1 = _pwmF + 1;
		nn = dutyA;  nn *= pwmN1; nn /= 100; nA = nn;
		nn = dutyB;  nn *= pwmN1; nn /= 100; nB = nn;
		OCR1A = nA;      // double buffered registers
		OCR1B = nB;      // change effective only after TCNT1 next reaches TOP
	}
#endif // platform dependent code
  
} // end Out

// -------------------------------------------------------------------------
// returns TOP value for counter

unsigned int PWM16::GetTOP () {
  return _pwmF;
}

//char * PWM16::getMessage() {
//	return timerMessage;
//}

//void PWM16::setMessage(char msg) {
//	timerMessage[0] = msg;
//}


// ------------------------------------------- PWM_IO3 methods

// setup timer parameters
void PWM_IO3::Setup( uint8_t pwm_mode, uint8_t prescale ) {

	pinMode(IO3_PIN, OUTPUT);

#ifdef ESP8266
	analogWriteRange(255); // reduce ESP resolution from 1023, to match Arduino 255 steps

	uint32_t IO3_Freq;
	if (pwm_mode = IO3_FASTPWM)
		IO3_Freq = 16000000 / prescale / 256; // 7812.5 Hz, for prescaler 8
	else if (pwm_mode = IO3_PCORPWM)
		IO3_Freq = 16000000 / prescale / 2 / 255; // 3921.56862745 Hz

	analogWriteFreq(IO3_Freq);
#elif defined ESP32
	// ... TBD
#else
	_pwm_mode = pwm_mode;
	_prescale = prescale;
	TCCR2A = _pwm_mode;
	TCCR2B = _prescale;
#endif // platform dependent code
}

// output
void PWM_IO3::Out(uint8_t duty) {

	analogWrite(IO3_PIN, duty);

}

/*void PWM_IO3::Out( uint8_t duty, uint8_t duty_pin) {

	analogWrite(duty_pin, duty );

}*/

