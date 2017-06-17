BOARD_TAG = pro328

include /usr/share/arduino/Arduino.mk

MONITOR_PORT := /dev/serial/by-id/usb-FTDI_FT232R_USB_UART_A600KQ3E-if00-port0

#isp:
#	avrdude -q  -p atmega328p -c usbasp -e -V  -U lfuse:w:0xE2:m -U hfuse:w:0xD2:m -U efuse:w:0x05:m -U flash:w:build-pro328/calibrator.hex


all:
	cp build-pro328/calibrator.hex .
