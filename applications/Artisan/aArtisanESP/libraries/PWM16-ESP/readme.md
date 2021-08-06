Works for slow PWM or ICC on ESP. PAC wasn't possible to implement, due to weak ESP timers architecture.
Also, ICC is exclusive with ModBus over TCP, any wifi activity make the ISR routines crash.

