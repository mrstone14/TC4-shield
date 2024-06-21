This document assume you are familiar with loading software on ESP boards in Arduino IDE.\
Else check tutorials like this:\
https://randomnerdtutorials.com/how-to-install-esp8266-board-arduino-ide/ 

The board core version should be set to 2.4.2, version used for development.\
A successful build was also reported using V3.0.0, however...

- Create a folder in Arduino sketchbooks locations, named aArtisanESP
- download there all files from .../aArtisanESP/src folder
- move to Arduino libraries folder, and download there the content of .../aArtisanESP/libraries folder, preserving the structure.
- ... or, download all as a zip: https://github.com/renatoa/TC4-shield/blob/master/applications/Artisan/aArtisanESP/tags/REL_aArtisanESP_1_0/aArtisanESP.zip, and unpack
- make desired changes in user.h and espLib.h header files, if any required
- compile/load
- if any error, contact me for support.

