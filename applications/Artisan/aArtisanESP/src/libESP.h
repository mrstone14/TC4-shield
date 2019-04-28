#ifndef ESP_LIB_H
#define ESP_LIB_H

// External objects declarations section //
#include "user.h"

#if defined ESP8266
//#define ESP_PLATFORM
#endif

#ifndef useSGfilter
#define useSGfilter // use SG filtering, author Renato Aranghelovici
#endif // 

// Platform and extended UI choices
#ifndef AlternateUI
#define AlternateUI // Extended UI
#endif // AlternateUI

#if defined ESP8266 || defined ESP32
#define adcButtons
#endif

#include <adcButton.h>
extern adcButtonPE16 buttons; // class object to manage button presses

#include <LiquidCrystal_I2C.h>
extern LiquidCrystal_I2C lcd; // (0x27, 20, 4);

#include <espEEPROM.h>
extern espEEPROM eeprom;

#include <PID_v1.h>
extern PID myPID; // (&Input, &Output, &Setpoint,2,0.05,0,P_ON_E,DIRECT); // P_ON_M

#include <cmndproc.h> // for command interpreter
extern CmndInterp ci; //( DELIM ); // command interpreter object

#include <cADC.h> // MCP3424
extern cADC adc; // MCP3424


extern float AT; // ambient temp
extern float T[NC];  // final output values referenced to physical channels 0-3
extern float RoR[NC]; // final RoR values
extern uint8_t actv[NC];  // identifies channel status, 0 = inactive, n = physical channel + 1

extern int levelOT1, levelOT2;  // parameters to control output levels

extern double Setpoint, Input, Output, SV; // SV is for roasting software override of Setpoint

extern int profile_number; // number of the profile for PID control
extern int profile_number_new; // used when switching between profiles
extern int16_t times[2], temps[2]; // time and temp values read from EEPROM for setpoint calculation
extern char profile_CorF; // profile temps stored as Centigrade or Fahrenheit

extern uint32_t counter; // second counter
extern uint32_t next_loop_time; // 

extern int LCD_mode;

#ifdef ANALOGUE1
extern uint8_t anlg1; // analog input pins
//int32_t old_reading_anlg1; // previous analogue reading
//boolean analogue1_changed;
#endif

#ifdef ANALOGUE2
extern uint8_t anlg2; // analog input pins
//int32_t old_reading_anlg2; // previous analogue reading
//boolean analogue2_changed;
#endif

#if defined PHASE_ANGLE_CONTROL
extern ACdetect();
#endif

#ifdef ESP32
extern void analogWrite(int, int);
#endif

extern bool log4Artisan;
extern bool log4Android;

// End Externals //

#if ! (defined ANLG1ADC3 || defined ANLG2ADC4)
#define NOPOTS
#endif
/*
struct profBlock {
	int16_t number = 1;
	int16_t type = 1;
	char CorF = 'C';
	char name[40] = "Default profile \0";
	char description[80] = "Automatically created when no profiles\0";
	int16_t time[50] = { 0, 120, 180, 240, 300, 360, 420, 480, 540, 600, 660, 720, 780, 0 };
	//int16_t temp[50] = { 25,  67,  93, 115, 134, 150, 163, 175, 184, 193, 200, 205, 210, 0 }; // slowest, longest profile, 6:5:4
	// Ror                              28   24   20   17   15   12   10    9    8    6    5    4
	  int16_t temp[50] = { 25,  78, 107, 130, 150, 166, 179, 190, 199, 206, 212, 218,   0 }; // average profile, 5:4:3
	// Ror                              32   26   21   18   15   12   10    8    6    5    4    ...
	//int16_t temp[50] = { 25,  92, 124, 150, 170, 187, 200, 210, 218,   0,   0,   0 }; // fastest, shortest profile 4:3:2
	// Ror                              36   29   23   18   15   12    9    7    6    ...
};
*/
#define vETMaxC    270.0	// Limit ET at max heater power
#define SIMPLE_ALT_UI
#undef FULL_ALT_UI

#endif
