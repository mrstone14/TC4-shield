// espProfiles.ino
// ------------
//
// this code is part of the TC4ESP project, a fork of aArtisanQ_PID TC4 
//
// *** BSD License ***
// ------------------------------------------------------------------------------------------
// Copyright (c) 2018-2019
// All rights reserved.
//
// Author:  Renato Aranghelovici, Brad Collins
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
// 20190418: added preliminary support for ESP32
// 20190424: split into a separate file, for better portability/compatibility

#if defined ESP8266 || defined ESP32
/////////////////////////////////////////
// the code below contains updated excerpts from aArtisanQ_PID Profile Loader, author Brad Collins
// purpose is to provide a roasting profile management via a new command, 
// and a default roasting profile, when none exists
/////////////////////////////////////////

int prof_size = 400; // number of bytes in EEPROM allocated for each profile
int start_of_profiles = 1024; // address of first profile in EEPROM

profBlock defprof;

espEEPROM ep;

bool isDefaultProfile() { // char *profname) {
	return verifyProfile(defprof, 1); //  strncmp(defprof_name, profname, sizeof(defprof_name));
}

/*bool setDefaultProfile() {
	profile_CorF = 'C';
	memcpy(times, defprof.time, sizeof(defprof.time));
	memcpy(temps, defprof.temp, sizeof(defprof.temp));
}*/

void getdefprof_name(char *profile_name) {
	strncpy(profile_name, defprof.name, sizeof(defprof.name));
}

void getdefprof_desc(char *profile_desc) {
	strncpy(profile_desc, defprof.description, sizeof(defprof.description));
}


struct profBlock profileRead(int16_t pn) {

	profBlock eeProfile;

	int prof_ptr = (pn - 1) * prof_size + start_of_profiles;  // set EEPROM address to start of this profile

	ep.read(prof_ptr, (uint8_t*)&eeProfile.number, sizeof(eeProfile.number)); // read profile from EEPROM
	// if (eeProfile.number != pn) return null;
	prof_ptr += sizeof(eeProfile.number);
	ep.read(prof_ptr, (uint8_t*)&eeProfile.type, sizeof(eeProfile.type));
	prof_ptr += sizeof(eeProfile.type);
	ep.read(prof_ptr, (uint8_t*)&eeProfile.CorF, sizeof(eeProfile.CorF));
	prof_ptr += sizeof(eeProfile.CorF);
	ep.read(prof_ptr, (uint8_t*)&eeProfile.name, sizeof(eeProfile.name));
	prof_ptr += sizeof(eeProfile.name);
	ep.read(prof_ptr, (uint8_t*)&eeProfile.description, sizeof(eeProfile.description));
	prof_ptr += sizeof(eeProfile.description);
	ep.read(prof_ptr, (uint8_t*)&eeProfile.time, sizeof(eeProfile.time));
	prof_ptr += sizeof(eeProfile.time);
	ep.read(prof_ptr, (uint8_t*)&eeProfile.temp, sizeof(eeProfile.temp));
	//prof_ptr += sizeof(temp);

	return eeProfile;

}


bool verifyProfile(profBlock checkProfile, int16_t pn) {

	profBlock eeProfile = profileRead(pn); // ) return false;

	if (pn != checkProfile.number) return false;

	if (eeProfile.type != checkProfile.type) return false;

	if (eeProfile.CorF != checkProfile.CorF) return false;

	if ((String)eeProfile.name != (String)checkProfile.name) return false;

	if ((String)eeProfile.description != (String)checkProfile.description) return false;

	// profile points are not checked, assumed right if all above passed

	//prof_ptr += sizeof(desc);
	//ep.read(prof_ptr, (uint8_t*)&time, sizeof(time));
	//prof_ptr += sizeof(time);
	//ep.read(prof_ptr, (uint8_t*)&temp, sizeof(temp));
	//prof_ptr += sizeof(temp);

	return true;
}

bool profileWrite(profBlock writeProfile, bool verify = true) {

	int prof_ptr = (writeProfile.number - 1) * prof_size + start_of_profiles; // set EEPROM address to start of this profile
	int prof_start = prof_ptr;

	ep.write(prof_ptr, (uint8_t*)&writeProfile.number, sizeof(writeProfile.number)); // write to EEPROM
	prof_ptr += sizeof(writeProfile.number);
	ep.write(prof_ptr, (uint8_t*)&writeProfile.type, sizeof(writeProfile.type));
	prof_ptr += sizeof(writeProfile.type);
	ep.write(prof_ptr, (uint8_t*)&writeProfile.CorF, sizeof(writeProfile.CorF));
	prof_ptr += sizeof(writeProfile.CorF);
	ep.write(prof_ptr, (uint8_t*)&writeProfile.name, sizeof(writeProfile.name));
	prof_ptr += sizeof(writeProfile.name);
	ep.write(prof_ptr, (uint8_t*)&writeProfile.description, sizeof(writeProfile.description));
	prof_ptr += sizeof(writeProfile.description);
	ep.write(prof_ptr, (uint8_t*)&writeProfile.time, sizeof(writeProfile.time));
	prof_ptr += sizeof(writeProfile.time);
	ep.write(prof_ptr, (uint8_t*)&writeProfile.temp, sizeof(writeProfile.temp));
	prof_ptr += sizeof(writeProfile.temp);

	Serial.print("# profile length"); Serial.println(prof_ptr-prof_start);

	//Serial.println(prof_ptr); // show pointer value for end of profile

	delay(100);

	if (verify) return verifyProfile(writeProfile, writeProfile.number);
	else return true;

}

#endif
