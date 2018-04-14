// TODO check dc3231 connection

#include <avr/boot.h>
#include <avr/wdt.h>
#include <Wire.h>
//#include <EEPROM.h>
//#include
//#include <LiquidCrystal.h>
#include <LiquidCrystal_I2C.h>

// only for debuging
const bool USE_UART=false;

const uint8_t DS3231_I2C_ADDR=0x68;
const byte LCD_I2C_ADDRESS = 0x27;

LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

// LCD presense is determined at runtime
// so the same code works with and without LCD
bool LCD_IS_PRESENT = false;

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

void printToEeeprom(const char *s) {
    eeprom_update_block( s,0,strlen(s)+1 );
}

void osccal_calibrate() {

    int start_freq = frequency();

    if (LCD_IS_PRESENT) printLine(0, OSCCAL, start_freq);

    if (OSCCAL<129) {
        const char msg[]="ERR:OUT OF RANGE";
        if (LCD_IS_PRESENT) {
            lcd.setCursor(0,1);
            lcd.write(msg);
        }
        printToEeeprom(msg);
        return;
    }

    byte bestcal=OSCCAL;
    int bestfreq = start_freq;

    if (start_freq>8000) OSCCAL++; // we go a step further
    else OSCCAL--;

    bool last_loop=false;

    while (true) {
        wdt_reset();
        int f = frequency();

        if (abs(f-8000)<abs(bestfreq-8000)) {
            bestfreq = f;
            bestcal = OSCCAL;
        }

        if (USE_UART) {
            Serial.println("##################");
            Serial.print(OSCCAL);
            Serial.print(" f=");
            Serial.println(f);
            Serial.print("best=");
            Serial.print(bestcal);
            Serial.print(" f=");
            Serial.println(f);
        }

        if (last_loop) break;

        if (start_freq>8000) {
            last_loop = (f<8000);
            if (OSCCAL>129) {
                OSCCAL--;
            }
            /* if (OSCCAL==128) {
                if (LCD_IS_PRESENT) {
                    lcd.setCursor(0,1);
                    lcd.write("ERR: 128 LIMIT");
                }
                return;
            }*/
        }
        else {
            last_loop = (f>8000);
            OSCCAL++;
            if (OSCCAL==192) {
                if (LCD_IS_PRESENT) {
                    lcd.setCursor(0,1);
                    lcd.write("ERR: 192 LIMIT");
                }
                return;
            }
            if (f>8000) break;
        }
    }

    if (LCD_IS_PRESENT) printLine(1, bestcal,bestfreq);

    byte eep[4]={ 0x05, bestcal, (byte)(255-bestcal), 0x05};
    eeprom_update_block(eep,0,4);

    if (! USE_UART) {
        pinMode(0,OUTPUT);
        digitalWrite(0,HIGH);
    }
}

void setup() {
    wdt_disable();
    // Only for debugging. Needs a USB to UART module
    // like FTDI or CP2102 or PL2303
    if (USE_UART) {
        Serial.begin(38400);
    }

    Wire.begin();

    Wire.beginTransmission(DS3231_I2C_ADDR);
    if (Wire.endTransmission() == 0) { // RTC is present
        Wire.beginTransmission(DS3231_I2C_ADDR);
        Wire.write(0x0E);
        Wire.write(0b01001000); // SQW 1024Hz is set
        Wire.endTransmission();
        pinMode(SQW_PIN,INPUT_PULLUP);
        if (USE_UART) Serial.println("DS3231 found");
    }
    else { // RTC is NOT present
        if (USE_UART) {
            Serial.println("FATAL: DS3231 not found");
            while (1);
        }
        printToEeeprom("DS3231 not found");
        while (1); // Hang here
    }

    Wire.beginTransmission(LCD_I2C_ADDRESS);
    if (Wire.endTransmission() == 0) { // LCD is present
        LCD_IS_PRESENT = true;
        lcd.begin(16,2);  // initialize the lcd
        lcd.clear();
        lcd.backlight();
    }

    if (USE_UART) {
        if (LCD_IS_PRESENT) Serial.println("LCD found");
        else Serial.println("LCD is missing");
    }

    osccal_calibrate();
}

void loop() {

}
