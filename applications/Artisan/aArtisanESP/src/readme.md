New files:
- aArtisanESP.ino - new file, containing the new UI and roasting logic specific to this fork
file posted atm is a limited version, still work in progress
- SGfilters.ino - RoR computation based on Savitsky Golay smooth/filtering
- espProfiles.ino - profile related EEPROM management functions
- espData.h - data structures, you can define here the very first and most generic roasting profile you want to be loaded at first run
- libESP.h - new file, header with external variables from the original aArtisanQ_PID sketch.\
Contains also some specific settings that you can change, like maximum ET allowed.

Changed files:
- aArtisanQ_PID.ino - a slighltly changed version of 6.7, changes required for platform compatibility and new features.\
It  is 100% backward compatible with Arduino version, it should compile if replacing the original TC4 sketch.
- phase_ctrl.cpp, phase_ctrl.h
- cmndreader.cpp
- user.h - contains my current testing environment settings/preferences, so check and change according to yours.

Third party:
- hw_timer.c, hw_timer.h - ESP8266 hardware timer, required for slow pwm ssr.

Hardware related details here:
https://github.com/renatoa/TC4-shield/tree/master/hardware/TC4ESP
