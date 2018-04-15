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

#OSCCAL := $(_shell ../_osccal)

all:
	@echo type \"make isp\" to write AtmegaBOOT with corrected OSCCAL to a ATmega328p chip

atmega328_bare: MCU_TARGET = atmega328p
atmega328_bare: CFLAGS += '-DMAX_TIME_COUNT=F_CPU>>4' '-DNUM_LED_FLASHES=0' '-DWATCHDOG_MODS' -DBAUD_RATE=57600 -DDOUBLE_SPEED
atmega328_bare: CFLAGS += $(OSCCAL_FLAG)
atmega328_bare: AVR_FREQ = 8000000L
atmega328_bare: LDSECTION  = --section-start=.text=0x7800
atmega328_bare: $(PROGRAM)_$(TARGET).hex

isp: clean getosccal atmega328_bare
	avrdude -qq -p atmega328p -c usbasp -e -U lfuse:w:0xE2:m -U hfuse:w:0xDA:m -U efuse:w:0xfd:m -U flash:w:$(PROGRAM)_$(TARGET).hex

getosccal:
	$(eval OPTIMAL_OSCCAL_VALUE := $(if $(OPTIMAL_OSCCAL_VALUE),$(OPTIMAL_OSCCAL_VALUE),$(shell ../osccal)) )
	$(eval OSCCAL_FLAG := -DOPTIMAL_OSCCAL_VALUE=$(OPTIMAL_OSCCAL_VALUE) )

%.elf: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f *.o *.elf *.lst *.map *.sym *.lss *.eep *.srec *.bin *.hex

%.lst: %.elf
	$(OBJDUMP) -h -S $< > $@

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

%.srec: %.elf
	$(OBJCOPY) -j .text -j .data -O srec $< $@

%.bin: %.elf
	$(OBJCOPY) -j .text -j .data -O binary $< $@

