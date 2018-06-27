# These values are OK with the stock arduino environment(1.0.5) coming with
# debian/ubuntu and the debian/ubuntu Arduino-Makefile.
BOARD_TAG = pro328
include /usr/share/arduino/Arduino.mk

# No need to do this on Debian/ubuntu
# For a newer/downloaded Arduino / Arduino-Makefile
#BOARD_TAG     =  pro
#BOARD_SUB  = 8MHzatmega328
#ARDUINO_DIR = $(HOME)/Downloads/arduino-1.8.5
#ARDUINO_LIBS = Wire Newliquidcrystal_1.3.5
#include $(HOME)/Downloads/Arduino-Makefile/Arduino.mk

all:
	cp build-*/calibrator.hex .
