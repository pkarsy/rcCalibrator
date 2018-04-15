#!/bin/sh

d=$(dirname "$0")
cd "$d"

#echo "############################################################" >2
#echo "################## Burning calibrator ######################" >2
#echo "############################################################" >2

avrdude -qq  -p atmega328p -c usbasp -e -V  -U lfuse:w:0xE2:m -U hfuse:w:0xD2:m -U efuse:w:0xfd:m -U eeprom:w:0xff:m -U flash:w:calibrator.hex
