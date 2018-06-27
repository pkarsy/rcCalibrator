// This file is part of the rcCalibrator project

#include <avr/boot.h>
#include <avr/wdt.h>
#include <Wire.h>

const uint8_t DS3231_I2C_ADDR=0x68;
const uint8_t LCD_I2C_ADDRESS = 0x27;
const uint8_t LOW_OSCCAL_START=80;

// New LiquidCrystal by Francisco Malpartida
// https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads/
// compliled with the 1.3.5 version
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

// LCD presense is determined at runtime
// so the same code works with and without LCD
bool lcd_is_present = false;

const byte SQW_PIN=A3;

// RTC SQW pin is configured to generate 1024 ticks/sec
// every busy loop is 62.5 msec
const uint32_t LOOPTIME = 1000000UL*64/1024; // == 62500 usec

void printLine(uint8_t line, int osccal, int freq) {
    char buf[40];
    buf[16]=0;
    lcd.setCursor(0,line);
    int p = (freq-8000)/8;
    char sign;
    if (p>0) sign='+';
    else if (p<0) sign='-';
    else sign=' ';
    p = abs(p);
    freq=(freq+5)/10;
    sprintf(buf,"%2d %d.%02dMh %c%d.%d%%", osccal, freq/100, freq%100, sign, p/10, p%10);
    lcd.write(buf);
}

// freq in  Khz
int frequency() {
    while (digitalRead(SQW_PIN)==LOW) ;
    while (digitalRead(SQW_PIN)==HIGH) ;
    uint32_t t = micros();
    for (byte i=0;i<64;i++) {
        while (digitalRead(SQW_PIN)==LOW) ;
        while (digitalRead(SQW_PIN)==HIGH) ;
    }
    return (int32_t)(micros()-t)*80/625;
}

bool osccal_calibrate() {
    // OSCCAL==128 is problematic as we can't reduce the value (See data sheet)
    if (OSCCAL==128) OSCCAL=LOW_OSCCAL_START;

    int start_freq = frequency();

    byte bestcal=OSCCAL;
    int bestfreq = start_freq;

    if (start_freq>8000) OSCCAL++; // we go a step further
    else OSCCAL--;

    bool last_loop=false;

    while (true) {
        int f = frequency();
        if (abs(f-8000)<abs(bestfreq-8000)) {
            bestfreq = f;
            bestcal = OSCCAL;
        }
        if (last_loop) {
            if (bestcal>=128 and bestcal<132) {
                // We avoid 128-131 as these values cannot be used by ATmegaBOOT
                // to setup 57600 bps correctly
                return false;
            }
            // otherwise we leave the loop and write the value to the eeprom.
            break;
        }
        if (start_freq>8000) {
            last_loop = (f<8000);
            if (129==OSCCAL and false==last_loop) {
                // We reurn failure and setup will run osccal_calibrate again
                return false;
            }
            OSCCAL--;
        }
        else {
            last_loop = (f>8000);
            OSCCAL++;
        }
    }


    // We write to the EEPROM so avrdude can read the value
    byte eep[4]={ 0x05, bestcal, (byte)(255-bestcal), 0x05};
    eeprom_update_block(eep,0,4);

    if (lcd_is_present) printLine(1, bestcal,bestfreq);

    return true;
}

void setup() {
    bool rtc_is_present = false;
    wdt_disable();
    Wire.begin();
    Wire.beginTransmission(DS3231_I2C_ADDR);
    if (Wire.endTransmission() == 0) { // RTC is present
        Wire.beginTransmission(DS3231_I2C_ADDR);
        Wire.write(0x0E);
        Wire.write(0b01001000); // SQW 1024Hz is set
        Wire.endTransmission();
        pinMode(SQW_PIN,INPUT_PULLUP);
        rtc_is_present = true;
    }

    Wire.beginTransmission(LCD_I2C_ADDRESS);
    if (Wire.endTransmission() == 0) { // LCD is present
        lcd_is_present = true;
        lcd.begin(16,2);  // initialize the lcd
        lcd.clear();
        lcd.backlight();
    }

    if (!rtc_is_present) {
        // We can't do much
        // but if LCD is connected (unlikely) we print a message
        if (lcd_is_present) {
            lcd.setCursor(0,0); // first line
            lcd.write("DS3231 not found");
        }
        while(1);
    }

    int start_freq = frequency();
    if (lcd_is_present) printLine(0, OSCCAL, start_freq);
    if (!osccal_calibrate()) {
        // We need to search optimal OSCCAL in the range 0-127
        // OSCCAL is set to LOW_OSCCAL_START and we run osccal_calibrate again
        OSCCAL=LOW_OSCCAL_START;
        osccal_calibrate();
    }
}

void loop() {
    // nothing to do
}
