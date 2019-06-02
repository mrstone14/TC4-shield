Operating TC4ESP
================

Because in this architecture there could be no pots, due to ESP 8266 lack of ADC, the 4 buttons logic has been changed/improved, in order to replace the pots functionality and also add new features.\
There are two UI provided, at the moment of first draft only the simple UI is available, the extended UI is still work in progress.

There are some display changes, listed below, for the simple UI:
- the most significant: works on 20x4 I2C displays only
- the AT temperature area was been replaced by a roast phase/mode indicator. In that area you can read the following:\
Roast phase: Idle, Roast, Cool, for simple UI, and more for extended version: Charge, TP, Drying, Yellow, Dev\
Mode: Man of Prof #, alternating with the roast phase above, to show manual mode or the number of profile used
The reason of retiring AT temperature: missing sensor, because the TC dedicated amplifiers have have embeded CJC.
- a "<" sign near the HRT or FAN value, showing the target of UP/DOWN buttons control. This pointer can be moved with mode button, during roast only
- the lower right half of display is used now for phase end predictions, Artisan style\
The right half of row 3, where SV is shown during a profile roast, now host the DE prediction during a manual roast\
The right half of row 4, now host either the FC prediction during a manual roast, either DE and FC, flashed alternate, during a profile roast.\
Prediction temperatures are fixed in code, 150 C for DE, and 200 C for FC. If your machine has significant different values, for all bean origins, feel free to change them before load, DE_TEMP and FC_TEMP, in libESP.h

![TC4ESP UI](screenshots/TC4ESP-UI1.png "TC4ESP UI")

Buttons new behaviour in simple UI:
- up/down - control heater or fan, instead pot in manual mode.\
Use long press on up/down keys, when heater control is active, as shortcut for MIN/MID/MAX OT1.\
No long press effect for FAN control.\
Up key press, in Idle state only, will start preheat, with HTR set to MID_OT1 value.
- enter key - short press toggle PID on-off in profile mode, do nothing in manual mode\
Long press resets timer and start roast, or stop roast, in either mode.
- mode key: 
in Idle phase acts as profile browsing and choice, like for standard TC4\
During roast acts as UP/DOWN buttons target toggle, between HTR and FAN. The "<" pointer near the HTR/FAN value serve as a visual hint of which output will be controlled by UP/DOWN buttons.\
At startup, if powering TC4ESP with mode button pressed, Bluetooth will be activated, and USB serial output for Artisan disabled.

Buttons layout, meaning and their application codes, for the eBay buttons pad

![Buttons](screenshots/Buttons_small.jpg "TC4ESP UI")

