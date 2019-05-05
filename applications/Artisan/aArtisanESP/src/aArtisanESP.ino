// aArtisanESP.ino
// ------------

// ESP specific extensions and alternate custom roasting logic and UI, for aArtisanQ_PID TC4 app
// trying to maintain compatibility with future Arduino versions of Q_PID, as much as possible.

// ESP platform changes, mainly done modifying related libraries: Buttons, EEPROM, PWM, Phase Control... the later work in progress
// so far only 20x4 I2C LCD hardware configuration is supported.

// following rows are changes that can be adopted/merged into current Arduino version right now, without any known issue, yet... other than memory limitations
// >> improved TC architecture, resolution is now 0.012 C degrees, acquisition time 60 ms per channel
// >> signal processing improvements: RoR using Savitsky-Golay filters
// >> alternative UI and operation mode: no pots for heater/fan control, automated roasting with preheat/charge/TP management
// >> create at startup a default profile, if none exists
// >> some minor fixes of V6_7 code, everything done with 100% maintaining backward compatibility with 6_7 version

// *** BSD License ***
// ------------------------------------------------------------------------------------------
// Copyright (c) 2018-2019
// All rights reserved.
//
// Author:  Renato Aranghelovici
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
//   Neither the name of the copyright holder(s) nor the names of its contributors may be 
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
// 20181230 Created.
// 20190103 added a bit of roast phases processing automation
// 20190106 roast phases processing automation and custom buttons/LCD behaviour
// 20190418: added preliminary support for ESP32
#include "libESP.h"
#include "cmndreader.h"

#if defined ESP8266 || defined ESP32#define BANNER_ARTISAN "aArtisanESP 1_0_1  "
#endif

#define roastPhase_Idle 0
#define roastPhase_Preheat 1
#define roastPhase_Charge 2
#define roastPhase_TP 3
#define roastPhase_Roast_Dry 4
#define roastPhase_Roast 46
#define roastPhase_Roast_Yellow 5
#define roastPhase_Roast_Dev 6
#define roastPhase_Cool 7

#define BTN_UP 0
#define BTN_DOWN 1
#define BTN_ENTER 2
#define BTN_MODE 3
#define BTN_MASK_UP 1
#define BTN_MASK_DOWN 2
#define BTN_MASK_ENTER 4
#define BTN_MASK_MODE 8

#define MID_OT1 ((MIN_OT1 + MAX_OT1) / 2)

byte roastPhase;
bool modeManual = false;
float BT, ET, currROR;
int16_t estDE; // estimated dry end phase time, for default 150C temp
int16_t estFC; // estimated first crack time, for default 200C temp
char buffDE[6], buffFC[6];
byte modeOT = 1;
bool modeVirTemp = false;

void processCommand(String cmd) {
	if (cmd.startsWith("OT2;")) {
		if (cmd.endsWith("DOWN")) {
			if (levelIO3 <= MIN_OT2) return;
			--levelIO3;
		}
		if (cmd.endsWith("UP")) {
			if (levelIO3 >= MAX_OT2) return;
			++levelIO3;
		}
		outIO3();
	}
	else {
		if (cmd == "OT1;UP") {
			if (levelOT1 >= MAX_OT1) return;
			++levelOT1;
			if (levelOT1 < MIN_OT1) levelOT1 = MIN_OT1;
		}
		else if (cmd == "OT1;DOWN") {
			if (levelOT1 <= MIN_OT1) return;
			--levelOT1;
		}
		else if (cmd == "OT1;MID") {
			levelOT1 = MID_OT1;
		}
		else if (cmd == "OT1;MIN") {
			levelOT1 = MIN_OT1;
		}
		else if (cmd == "OT1;MAX") {
			levelOT1 = MAX_OT1;
		}
		outOT1();
	}
}


/////////////////////////////////////////
// Two custom buttons UI are provided as an alternative to original TC4 operating mode
// - a simple UI, which is close to the original, with some remapping to compensate for the lack of pots
// - a fully different complex UI, featuring roast phases management
/////////////////////////////////////////

#if defined SIMPLE_ALT_UI // && defined ESP8266

/////////////////////////////////////////
// Custom buttons behaviour in simple UI
// - up/down - control heater/fan instead pot in manual mode, long press on up/down are shortcuts for MIN/MID/MAX OT1
// - enter key - short press PID on-off in profile mode, nothing in manual mode
//             - long press resets timer and start profile, or stop roast in any moment
// - mode key, as standard TC4, but in Idle only, during roast acts as UP/DOWN buttons toggle between HTR and FAN
/////////////////////////////////////////

void checkButtons() { // take action if a button is pressed

	if (buttons.readButtons()) {

		byte longPressKeys = buttons.getPressedLongKeys();
		//Serial.print("longPressKeys "); Serial.println(longPressKeys);

		modeManual = (profile_number == 0 || myPID.GetMode() == MANUAL);

		switch (LCD_mode) {

		case 0: // Main LCD display

				// Serial.println("Dbg case 0 "); 
			if (buttons.keyClick(BTN_ENTER)) { // button Enter - PID on/off - roast start/stop on long press
				if (roastPhase == roastPhase_Cool && !(longPressKeys == BTN_MASK_ENTER)) { // one more short enter press in Cool phase, to return to Idle
					roastPhase = roastPhase_Idle;
					// reset roasting variables
					modeOT = 1;
					// blank out DE and FC when going from Cool to Idle
					lcd.setCursor(13, 2);
					lcd.print(F("       ")); 
					lcd.setCursor(13, 3);
					lcd.print(F("       ")); 
					return;
				}
				else if (roastPhase == roastPhase_Preheat && !(longPressKeys == BTN_MASK_ENTER)) { // one short enter press in preheat phase, do nothing
					return;
				}
#ifdef PID_CONTROL
				if (myPID.GetMode() == MANUAL && profile_number > 0) { // start PID for existing profile only, else stay manual
					myPID.SetMode(AUTOMATIC);
					// when PID start on long press, also reset timer
					if (longPressKeys == BTN_MASK_ENTER) {
						counter = 0;
						roastPhase = roastPhase_Roast;
					}
				}
				else { // no profile chosen
					if (myPID.GetMode() == AUTOMATIC) {
						myPID.SetMode(MANUAL);
						lcd.setCursor(13, 2);
						lcd.print(F("       ")); // blank out SP: nnn if PID is off
					}
					if (longPressKeys == BTN_MASK_ENTER) {
						if (roastPhase == roastPhase_Idle || roastPhase == roastPhase_Preheat) {
							levelOT1 = MIN_OT1;
							outOT1();
							roastPhase = roastPhase_Roast;
							counter = 0;
						}
						else if (roastPhase == roastPhase_Roast) {
							levelOT1 = 0;
							outOT1();
							roastPhase = roastPhase_Cool;
						}
					}
				}
#endif
			}
			else if (longPressKeys == BTN_MASK_DOWN) {
				if (modeManual) {
					if (levelOT1 > MID_OT1) processCommand("OT1;MID"); // long press on DOWN lowers power to mid,
					else if (levelOT1 <= MID_OT1) processCommand("OT1;MIN"); // else lowers power to min, 
																			 // else if (levelOT1 == MIN_OT1) levelOT1 = 0; // then second long press stop heater at all
				}
				return;
			}
			else if (buttons.keyClick(BTN_DOWN)) { // button DOWN - lower heat in NOPOTS manual mode
												   // Serial.println("Dbg click down ");
				if (modeOT == 2) {
					processCommand("OT2;DOWN");
				}
				else if (modeManual) {
					processCommand("OT1;DOWN");
				}
			}
			else if (longPressKeys == BTN_MASK_UP) {
				if (modeManual) {
					//roastPhase = roastPhase_Roast;
					if (levelOT1 < MID_OT1) processCommand("OT1;MID"); // long press on UP raise power to mid, 
					else if (levelOT1 >= MID_OT1) processCommand("OT1;MAX"); // else next press raise power to max, 
				}
				return;
			}
			else if (buttons.keyClick(BTN_UP)) { // button UP - raise heat in NOPOTS manual mode
												 //Serial.println("Dbg click up ");
				if (modeOT == 2) {
					processCommand("OT2;UP");
				} 
				else if (modeManual) {
					processCommand("OT1;UP");
					if (roastPhase == roastPhase_Idle) {
						//counter = 0;
						roastPhase = roastPhase_Preheat;
						processCommand("OT1;MID");
					}
				}
				return;
			}
			else if (buttons.keyClick(BTN_MODE)) { // button 4 - CHANGE LCD MODE
													//Serial.println("Dbg click mode ");
				if (roastPhase == roastPhase_Idle) { // allow profile change in idle only
					lcd.clear();
					LCD_mode++; // change mode
					getProfileDescription(profile_number); // required to populate profile data when change mode is pressed first time
#ifndef PID_CONTROL
					if (LCD_mode == 1) LCD_mode++; // deactivate LCD mode 1 if PID control is disabled
#endif
					if (LCD_mode > 1) LCD_mode = 0; // loop at limit of modes
					delay(5);
			}
				else { // mode button during roast switch between OT1 and OT2 control
					modeOT = (modeOT == 1) ? 2 : 1;
				}
			}
			break;

		case 1: // Profile Selection and PID parameter LCD display

				//Serial.println("Dbg case 1 ");

			if (buttons.keyPressed(0) && buttons.keyChanged(0)) { // button 1 - PID on/off - PREVIOUS PROFILE
#ifdef PID_CONTROL
				profile_number_new--;
				if (profile_number_new < 0) profile_number_new = NUM_PROFILES; // loop profile_number to end
				getProfileDescription(profile_number_new);
#endif
			}
			else if (buttons.keyPressed(1) && buttons.keyChanged(1)) { // button 2 - RESET TIMER - NEXT PROFILE
#ifdef PID_CONTROL
				profile_number_new++;
				if (profile_number_new > NUM_PROFILES) profile_number_new = 0; // loop profile_number to start

				getProfileDescription(profile_number_new);
#endif
			}
			else if (buttons.keyPressed(2) && buttons.keyChanged(2)) { // button 3 - ENTER BUTTON
#ifdef PID_CONTROL
				profile_number = profile_number_new; // change profile_number to new selection
				setProfile(); // call setProfile to load the profile selected
				lcd.clear();
				LCD_mode = 0; // jump back to main LCD display mode
#endif
			}
			else if (buttons.keyPressed(3) && buttons.keyChanged(3)) { // button 4 - CHANGE LCD MODE
				lcd.clear();
#ifdef PID_CONTROL
				profile_number_new = profile_number; // reset profile_number_new if profile wasn't changed
				setProfile(); // or getProfileDescription()?????????
#endif
				LCD_mode++; // change mode
				if (LCD_mode > 1) LCD_mode = 0; // loop at limit of modes

				delay(5);
			}
			break;

		} //end of switch
	} // end of if( buttons.readButtons() )

} // end of void checkButtons()

  /////////////////////////////////////////
  // Alternate UI LCD handler
  /////////////////////////////////////////

// #if defined SIMPLE_ALT_UI // && (defined ESP8266 || defined ESP32)
void updateLCDalt(byte roast_Phase = 255) {

#if defined PHASE_ANGLE_CONTROL
	// ZCD presence signaled by blinking the : of timer/counter
	lcd.setCursor(2, 0);
	if (ACdetect())
		if (counter % 2)
			lcd.print(" "); //clock[2] = ' '; //  
		else
			lcd.print(":"); // clock[2] = ':'; // 
							//	lcd.print(F(":")); // make this blink?? :)
#endif

/*#ifdef PID_CONTROL
	if (myPID.GetMode() == MANUAL) {
		lcd.setCursor(13, 2);
		lcd.print(F("       ")); // blank out SP: nnn if PID is off
	}
#endif // end ifdef PID_CONTROL*/

	if (LCD_mode == 0) {

		ET = convertUnits(T[0]);

		if (ET > 270) {
			if (counter % 2) { // flashing alternate "HIGH" with ET value
				lcd.setCursor(16, 0);
				lcd.print("HIGH"); // 
			}
		}

		if (roast_Phase == 255) roast_Phase = roastPhase;

		lcd.setCursor(6, 0); // AT area 6 char is overwriten with custom UI roasting phase name

							 //Serial.print("roast_Phase - counter"); Serial.print(roast_Phase); Serial.print("-"); Serial.println(counter);
		modeManual = (profile_number == 0 || myPID.GetMode() == MANUAL);

		if (levelOT1 > 0 && roast_Phase == roastPhase_Idle) roast_Phase = roastPhase_Roast; // when roast was been triggered by external commands, like PID; or OT1;

		if ((roast_Phase != roastPhase_Cool) && (counter % 5 == 0) && !(profile_number == 0 && modeManual)) {
			lcd.print(F("Prof "));
			lcd.print(profile_number);
		}
		else {
			if (roast_Phase == roastPhase_Idle) {
				lcd.print(F("Idle  "));
			}
			else if (roast_Phase == roastPhase_Preheat) {
				lcd.print(F("PreHT "));
			}
			else if (roast_Phase == roastPhase_Roast) {
				if ((counter % 3 == 0) && modeManual) {
					lcd.print(F("Man   "));
				}
				else {
					lcd.print(F("Roast "));
				}

				BT = convertUnits(T[1]);
				currROR = RoR[ROR_CHAN - 1];
				float deTemp = DE_TEMP;
				if (BT < deTemp) { // shows DE prediction
					estDE = (deTemp - BT) * 60 / currROR;				
					if (estDE <= 0) return;
					estDE += counter;
					if (profile_number == 0) // if no profile use SP location for DE
						lcd.setCursor(13, 2);
					else // if profile used share location for DE and FC 
						lcd.setCursor(13, 3);
					lcd.print(F("DE"));
					// if (estDE > 0 && estDE < 600) sprintf(buffDE, ":%01d:%02d", estDE / 60, estDE % 60);
					if (estDE < DE_MIN) sprintf(buffDE, ": low");
					else if (estDE > DE_MAX) sprintf(buffDE, ":high");
					else sprintf(buffDE, ":%01d:%02d", estDE / 60, estDE % 60);
					lcd.print(buffDE);				
				}
				else if (BT < FC_TEMP) { // shows FC prediction
					estFC = (FC_TEMP - BT) * 60 / currROR;
					if (estFC > 0) estFC += counter;

					lcd.setCursor(13, 3);
					if (profile_number > 0 && (counter % 2 == 0)) { // if profile flash alternating DE with FC
						lcd.print(F("DE"));
						lcd.print(buffDE);
					}
					else {
						lcd.print(F("FC"));
						if (estFC > 0) {
							if (estFC < 600)
								sprintf(buffFC, ":%01d:%02d", estFC / 60, estFC % 60);
							else
								sprintf(buffFC, "%02d:%02d", estFC / 60, estFC % 60);
						}
						lcd.print(buffFC);
					}
				} // end predictions

				// up/down buttons pointer
				if (modeOT == 1) {
					lcd.setCursor(9, 2);
					lcd.print('<');
					lcd.setCursor(9, 3);
					lcd.print(' ');
				}
				else {
					lcd.setCursor(9, 3);
					lcd.print('<');
					lcd.setCursor(9, 2);
					lcd.print(' ');
				}

			} // end Roast phase

			else if (roast_Phase == roastPhase_Cool) {
				lcd.print(F("Cool  "));
				//lcd.setCursor(13, 3);
				//lcd.print(F("FC"));
				//lcd.print(buffFC);
			}
		}

		if (modeVirTemp && (counter % 4 == 0)) { // flash virtual temperatures mode
			lcd.setCursor(6, 0);
			lcd.print(F("VirTmp"));
		}

	} // end LCD mode 0
} // end update LCD

#endif // SIMPLE_ALT_UI 

  /////////////////////////////////////////
  // additional setup for ESP platforms
  /////////////////////////////////////////
void setupESP() {

	//lcd.setCursor(0, 2);
	lcd.print(__DATE__ "-" __TIME__); // build date/time
	Serial.println(BANNER_ARTISAN); // display version banner
	Serial.println(__DATE__ "-" __TIME__); // build date/time

	// add profile related commands set
	ci.addCommand(&profile);
	// check default profile
	// Serial.print("getProfileDescription pn "); Serial.println(pn);
	// check if default profile exists
	int pn = 1; // default profile is first
	int ppn = 1024 + (400 * (pn - 1)); // 1024 = start of profile storage in EEPROM. 400 = size of each profile. location of profile number
	int16_t pne; // profile number from eeprom
	eeprom.read(ppn, (uint8_t*)&pne, sizeof(pne)); // read profile number
	//Serial.print("getProfileDescription pn-pne "); Serial.print(pn); Serial.println(-pne);
	if (pn == 1 && !isDefaultProfile()) {
		// profile 1 not initialised in eeprom, create default profile
		Serial.println("Creating default profile");
		profBlock defprof;
		if (!profileWrite(defprof, true)) {
			Serial.println("Default profile creation failed");
			//return;
		}
	}
	/*else if (pn > 1 && pn != pne) { // if profile number from eeprom not matching profile number, assume free location
	Serial.println("Free profile " + (String)pne);
	strcpy(profile_name, "Free"); // , sizeof(profile_name));
	strcpy(profile_description, "Unused profile slot"); // , sizeof(profile_name));
	return;
	}*/

#ifdef PHASE_ANGLE_CONTROL
	delay(100); // let some cycles to allow ISR doing its magic
	if (ACdetect()) {
		lcd.setCursor(0, 3);
		lcd.print("ZCD detected");
	}
#endif

	byte btn = buttons.rawRead();

	if (btn > 0 ) {
		lcd.setCursor(0, 3);
#ifdef ANDROID
		// BT adapter second serial additional setup, if any
		if (btn == BTN_MASK_MODE) { // buttons.keyClick(BTN_MODE)) {
			// activate BT if pressing Mode button during start
#ifdef ESP8266			// BT alternate serial activation
			Serial.end(); // stop USB serial
			lcd.print(F("BlueTooth enabled"));
			Serial.begin(9600); // start at low BT speed
			Serial.swap(); // swap to alternate pins set, where is connected BT adapter
							// this way the USB serial is free for sketch updates without any phisical switch

			log4Android = true;
			log4Artisan = false;
#elif defined ESP32 // ESP32		// ... TBD using BT serial#endif // platforms choice for serial BT

		} // end btn Mode pressed at startup = activate BT

#endif // Android

		else if (btn == BTN_MASK_DOWN) { // buttons.keyClick(BTN_DOWN)) {
			// activate virtual TC if pressing Down button during start
			modeVirTemp = true;
			roastPhase = roastPhase_Roast;
			lcd.print(F("Virtual temperatures"));
		}

		if (btn > 0) delay(1000); // delay see mode change

	}
	/*int adc = analogRead(A0);

	if (adc < 512) { // activate BT if pressing Mode button during start
					 // Serial.print("button catch");
		delay(10);

		Serial.end(); // stop USB serial
		lcd.setCursor(0, 3);
		lcd.print(F("BlueTooth enabled"));
		Serial.begin(9600); // start at low BT speed
		Serial.swap(); // swap to alternate pins set, where is connected BT adapter
					   // this way the USB serial is free for sketch updates without any phisical switch

		log4Android = true;
		log4Artisan = false;

		delay(1000);
	}
	else {
		log4Artisan = true;
		log4Android = false;
	}*/

	delay(1000);
	lcd.clear();

	//align next_loop_time to second
	next_loop_time = (int)(next_loop_time / 1000);
	next_loop_time *= 1000;

}


/////////////////////////////////////////
// use ADC3/4 ports of MCP3424 for pots (analog control)
// for platforms with not enough available ADCs
/////////////////////////////////////////

#if defined ANLG1ADC3 || defined ANLG2ADC4
int adcRead(byte port) {
	byte k = 0;
#ifdef ANALOGUE1
	if (port == anlg1) k = 3;
#endif
#ifdef ANALOGUE2
	if (port == anlg2) k = 4;
#endif
	if (k > 0) {
		--k;
		adc.nextConversion(k); // start ADC conversion on physical channel k
		checkStatus(60); // give the chips time to perform the conversions
		int32_t v = adc.readuV() / 4; // retrieve microvolt sample from MCP3424  
		v /= 32; // map the 15 bit value, corresponding to 0-2V pot range, to Arduino compatible 10 bit value range
		return v;
	}
}
#endif
