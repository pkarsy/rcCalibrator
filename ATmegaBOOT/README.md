### OSCCAL aware ATmegaBoot, part of rcCalibrator

This is the ATmegaBoot used in proMini with some modifications

- The Makefile supports only atmega328p.The modifications needed to support other chips should not be hard.
- No precompiled .hex file. The compilation is successful only if the Makefile
can find a atmega328p chip and calculate the optimal OSCCAL.
- Watchdog patches are by default enabled. This means that that a watchdog reset does NOT crash the chip (the chip resets continiously and cannot start the application). This is what happens with proMini, unfortunatelly.
- No Bootloader LED blink, because probbably this bootloader will not be used in an arduino board.
- Arduino code and Arduino IDE can be used, and the booard should be set to proMini 3.3V (8Mhz)
