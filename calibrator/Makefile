# These values are OK with the stock arduino environment coming with debian/ubuntu 1.05
# and the corresponding Arduino-Makefile
#BOARD_TAG = pro328
#include /usr/share/arduino/Arduino.mk

# For a custom Arduino / Arduino-Makefile
# Edit for your environment
BOARD_TAG     =  pro
BOARD_SUB  = 8MHzatmega328
ARDUINO_DIR = $(HOME)/Downloads/arduino-1.8.5
#ARDUINO_LIBS = Newliquidcrystal_1.3.5
include $(HOME)/Downloads/Arduino-Makefile/Arduino.mk



# Only useful at development, and only if serial port is enabled in the .ino file
#MONITOR_PORT = /dev/ttyUSB0

# Not needed, the python script ("osccal") does that
#isp:
#	avrdude -q  -p atmega328p -c usbasp -e -V  -U lfuse:w:0xE2:m -U hfuse:w:0xD2:m -U efuse:w:0x05:m -U flash:w:build-pro328/calibrator.hex


all:
	cp build-*/calibrator.hex .
