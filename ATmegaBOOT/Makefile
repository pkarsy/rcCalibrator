#
# This is part of OsccalCalibrator project
# https://github.com/pkar/OsccalCalibrator
# This is the ATmegaBOOt Makefile with theese modifications:
# Only atmega328 target.
# Allows compile only if can determine OSCCAL
# This means that the it needs a atmega328 chip in the usbasp socket
# the fuses are set to use internal oscillator @ 8MHz
#

# Makefile for ATmegaBOOT
# E.Lins, 18.7.2005
# $Id$
#
# Instructions
#
# To make bootloader .hex file:
# make diecimila
# make lilypad
# make ng
# etc...
#
# To burn bootloader .hex file:
# make diecimila_isp
# make lilypad_isp
# make ng_isp
# etc...

PROGRAM    = ATmegaBOOT
TARGET = atmega328_8MHz


OBJ        = $(PROGRAM).o
OPTIMIZE   = -Os

DEFS       =
LIBS       =

CC         = avr-gcc

# Override is only needed by avr-lib build system.

override CFLAGS        = -g -Wall $(OPTIMIZE) -mmcu=$(MCU_TARGET) -DF_CPU=$(AVR_FREQ) $(DEFS)
override LDFLAGS       = -Wl,$(LDSECTION)
#override LDFLAGS       = -Wl,-Map,$(PROGRAM).map,$(LDSECTION)

OBJCOPY        = avr-objcopy
OBJDUMP        = avr-objdump

all: atmega328_bare

#read_osccal:
#	OSCCAL:=(shell ./get_osccal.py)


#atmega328_bare:
atmega328_bare: MCU_TARGET = atmega328p
atmega328_bare: CFLAGS += '-DMAX_TIME_COUNT=F_CPU>>4' '-DNUM_LED_FLASHES=0' '-DWATCHDOG_MODS' -DBAUD_RATE=57600 -DDOUBLE_SPEED
atmega328_bare: CFLAGS += -DCALCULATED_OSCCAL_VALUE=$(shell ../osccal)
atmega328_bare: AVR_FREQ = 8000000L
atmega328_bare: LDSECTION  = --section-start=.text=0x7800
atmega328_bare: $(PROGRAM)_$(TARGET).hex

#atmega328_bare_isp: clean
atmega328_bare_isp: clean atmega328_bare
	avrdude -qq -p atmega328p -c usbasp -v -e -U lfuse:w:0xE2:m -U hfuse:w:0xDA:m -U efuse:w:0x05:m -U flash:w:$(PROGRAM)_$(TARGET).hex
	$(MAKE) clean

isp: atmega328_bare_isp

%.elf: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -rf *.o *.elf *.lst *.map *.sym *.lss *.eep *.srec *.bin *.hex

%.lst: %.elf
	$(OBJDUMP) -h -S $< > $@

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

%.srec: %.elf
	$(OBJCOPY) -j .text -j .data -O srec $< $@

%.bin: %.elf
	$(OBJCOPY) -j .text -j .data -O binary $< $@

