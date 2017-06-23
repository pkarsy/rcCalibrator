### OSCCAL aware ATmegaBoot, part of rcCalibrator

This is the ATmegaBoot used in proMini with some modifications
- The Makefile supports only atmega328p.The modifications needed to support other chips should not be hard.
- No precompiled .hex file. The compilation is successful only if the Makefile
can find a atmega328p chip and calculate the optimal OSCCAL.
- Watchdog paches are by default enabled.
- No Bootloader LED blink, because odds are this bootloader will not be used in an arduino board. Of course one can
use Arduino code and even Arduino IDE, and the booard should be set to proMini 3.3V (8Mhz)
