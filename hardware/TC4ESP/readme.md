Before starting, a short warn: this is a builders project, there is no shield available, or a complete ready to buy version, as is TC4C ! Yet...
However, to aprroach this build only simple pins/wires soldering knowledge is required, nothing smd is involved, the whole project use ready soldered breadboard modules.

And now, explanation of TC4ESP schematic... components description:

- buttons pad - done using the resistor ladder principle - https://en.wikipedia.org/wiki/Resistor_ladder - and the ADC input of ESP8266.
    
    The buttons pad can be DIY, or you can buy ready to use boards for this purpose from eBay, search for "Analog button for Arduino".
    
    To handle this new button pad option, a new library adcButtons has been created, forking the initial cButton library. 
    A new feature of this library is the long press handling.
    
    This library, along with the resistor ladder concept, can be used also with the original TC4 shield, changing the ADC GPIO pin accordingly.
    
- heater SSR - any random fire SSR, only slow PWM support so far, ICC and PAC is work in progress.
    
    No fan support for now, other than a PWM output, possible, but disabled in actual code.

- LCD - any cheap 20x4 I2C LCD. Library used for this display: https://github.com/marcoschwartz/LiquidCrystal_I2C

- thermocouples interface
    TC4ESP use the same MCP3424 capable ADC, but raise the performance bar at other level, adding for each thermocouple a dedicate amplifier with cold junction compensation, from the AD849x line, manufactured by Analog Devices.
    
    With this setup the resolution and sample rate is in the same ballpark as the phidgets device, at 1/4 cost, or less. 
    
    ADC and amplifiers used were bought as ready soldered breadboards from eBay, total cost of an ADC and two amplifier boards, about $15.
    
    The performance of this solution is now 0.012 C degree resolution (LSB), and 60 ms sample time - per channel - so 16 samples per second for single TC, and 8 samples per second for two measurements. 
    The sample time is smaller because ADC works in 16 bit mode, instead 18 bit, faster and more stable.
    
    Amplifiers are available for K and J types only. Not sure how popular is the T type thermocouple, though...
    
    No more ambient sensor is used, because CJC is a feature of TC amp itself.
    
    Software changes to handle this configuration are very small... actually it could operate with the original library unchanged, using the linear model class, just specifying a slope of 5mV per C degree. However, for more code optimization, some sections related to ambient sensor were disabled, and also the code for K/J/T thermocouples was been conditionally disabled based on several defines. 
    The resulting library should be of greater interest for Arduino users than for ESP platform, because a serious memory amount was been freed this way.
    
- potentiometers
     Because one of the most important drawbacks of the ESP platform is the lack of multiple ADC, this issue was been addressed in two ways:
     - Additional operating modes, based on buttons instead pots, were been added in a separate user interface;
     - in a later iteration, for those not happy to operate clicking buttons, the pots based functionality was been reinstated, using the unused channels 3 and 4 of MCP3424, assuming they aren't used for additional thermocouples !
     
     So, anyone interested to approach this new platform should be aware about this possible scenario limitation: if all the 4 TC channels are desired to be used for temperature, AND not interested to explore a new user interface where heater is operated using buttons, then the only available operating mode is profile based roasting, with no manual heater control.
     
     Another approach, that is now in the evaluation queue, is to use a rotary encoder instead potentiometer.
     
The code branch for this platform can be found here:
... to come
