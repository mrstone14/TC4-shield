For anyone intending to approach this project, I want to warn about a connection trick between TC amplifier and ADC inputs.

For flexibility and in order to handle a broad range of temperatures, the AD849x family of TC amplifiers has been designed with a reference voltage pin, where you can apply a voltage to shift the whole measurement range up or down.\
If this reference voltage is 0v (grounded), then the circuit will output zero volts for 0 C degrees temperature.\
For an output voltage of 5V, and 5mV/C degree slope, this amplifier can measure up to 1000 C degrees.\
However, the designers of these boards decided to shift this range into negative temperatures realm too, and provided on board a 1.25V reference source, the result being a -250C range shift, thus -250c+750C total range, and 1.25V output voltage for 0C temperature.\
Apparently, this +750C maximum temperature should be suitable for coffee roasting purposes, but we have a second limiting factor that make difficult to use the amp boards as they are sold.\
This factor is the ADC input voltage range, only 2V.\
If we measure directly the amp board output voltage then the useful range will be from 1.25V to 2V, i.e. 0.75V, that equates to 150 C only. Any temperature over 150C will be clipped and displayed as 150C.

We have two choices to fix this issue:
- short the reference voltage pin to ground, thus removing the temperature range shift into negatives, and output zero volts for zero C degress. The -CHx inputs of ADC will be connected to ground
- use differential input feature of ADC, connecting the -CHx inputs to reference voltage, instead ground.

The result will be the same in both cases, a 0-400C range, enough for a roaster.

The two choices are pictured in the attached Amp-ADC connection.jpg image. The red circled pad is the reference voltage point.\
The upper amp board is connected in shorted/grounded reference mode, and the lower is connected in differential mode.\
You can use either connection mode, the result will be the same. \
The first choice has a small advantage over the second, because the wires from/to ADC are soldered on amp board pads, thus better support than soldering on that voltage reference pin. However, if the wire is thin enough, even the second choice can be improved by passing the wire through the gnd pad hole, and securing with a hot glue drop.
