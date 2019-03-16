New files:
- aArtisanESP.ino - new file, containing the whole new logic specific to this fork
- libESP.h - new file, header with external variables from the original aArtisanQ_PID sketch

Third party:
- hw_timer.c, hw_timer.h - ESP8266 hardware timer

Changed files:
- aArtisanQ_PID.ino - a slighltly changed version of 6.7, that is 100% backward compatible with Arduino version, 
    changes being added only for platform compatibility and new features.
- phase_ctrl.cpp, phase_ctrl.h
- cmndreader.cpp

Hardware related details here:
https://github.com/renatoa/TC4-shield/tree/master/hardware/TC4ESP
