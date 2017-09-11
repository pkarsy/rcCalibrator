// simple macros to control XTAL1 and XTAL2 pins
// They work only if the MCU has the fuses set
// for internal RC oscillator
// put the file in the same folder as the sketch and:
// #include "xtal.h"
// This file is part of rcCalibrator
// https://github.com/pkarsy/rcCalibrator

#ifndef XTAL_H
#define XTAL_H

#define XTAL1_OUTPUT  DDRB  |= _BV(6)
#define XTAL1_INPUT   DDRB  &= ~_BV(6)
#define XTAL1_HIGH    PORTB |= _BV(6)
#define XTAL1_LOW     PORTB &= ~_BV(6)
#define XTAL1_INPUT_PULLUP XTAL1_INPUT;XTAL1_HIGH

#define XTAL2_OUTPUT  DDRB  |= _BV(7)
#define XTAL2_INPUT   DDRB  &= ~_BV(7)
#define XTAL2_HIGH    PORTB |= _BV(7)
#define XTAL2_LOW     PORTB &= ~_BV(7)
#define XTAL2_INPUT_PULLUP XTAL2_INPUT;XTAL2_HIGH

#endif
