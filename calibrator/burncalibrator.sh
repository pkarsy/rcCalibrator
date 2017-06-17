#!/bin/sh

d=$(dirname "$0")
cd "$d"

avrdude -qq  -p atmega328p -c usbasp -e -V  -U lfuse:w:0xE2:m -U hfuse:w:0xD2:m -U efuse:w:0x05:m -U eeprom:w:0xff:m -U flash:w:calibrator.hex
