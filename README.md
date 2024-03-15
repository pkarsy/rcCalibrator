
![rcCalibrator](rcCalibrator.jpg)

```
 First Line: Factory OSCCAL=159 Frequency=8.11Mhz Error = +1.4%
Second Line: Optimal OSCCAL=157 Frequency=8.02Mhz Error = +0.2%
```

# rcCalibrator
Calibration of the internal RC oscillator of atmega328p chip, and OSCCAL aware Serial bootloader(ATmegaBOOT).
The hardware consists of a USBasp programmer and a DS3231 module, and of course the programmer can be
used for its normal purpose to flash the chips. The LCD is optional.To bypass the documentation go
down to [installation](#software-installation)

### WARNING
- The use of the "osccal" utility will erase all the contents of your MCU without notice.
- Even if you connect a crystall the chip will ignore it
Use it only if you are familiar with ISP programming and know how to set the chip to the old state.
- 57600 and even more 115200 is somewhat problematic. See the section "57600bps"

### The Problem
A lot of newer MCU's have an internal oscillator wich is 1% factory calibrated. For those MCU's this page is irrelevant. Atmega328p (and others) however can deviate from nominal freequency up to 10% according to the manufacturer (usually 0-3% and to be fair, most of the time, very close to 0%). This can cause numerus problems, especially with Serial communications. This page is about solutions on this problem without resorting to external crystal. And the dillema crystal/RC oscillator arises only with bare atmega's. All arduino boards with atmega328 (UNO, ProMini etc) come with external crystal.
The AVR microcontrollers have a register called OSCCAL (Oscillator Calibration) which can
be used to drift the RC frequency and reduse the error to less than 1%. Unfortunatelly the register is
volatile and need to be set every time the MCU starts.

### Purpose
There are a lot of internet pages to address the calibration problem, and atmel has released a lot of
related papers. This project aims to offer an alternative solution :
- To find the optimal OSCCAL value, using a USBASP programmer and a DS3231 rtc module.
- **More importantly** to automatically build bootloader and
application code capable of fixing the RC frequency.

### Serial communication problems
Even with a perfect crystal, serial communication introduces
another type of error, as the 8MHz clock speed is not divided exactly with the standard
serial bitrates. See [WormFood calculator](http://wormfood.net/avrbaudcalc.php) at 8Mhz

```
38400   +0.2%   This is an excellent choise when running at 8Mhz
57600   +2.1%   The proMinis(3.3V 8Mhz) are capable of this serial speed
115200  -3.5%   This is the reason ProMini@8Mhz cannot do 115.2k
```
If we use the internal oscillator, this error can be added or subtracted to the RC oscillator error.

### Serial bootloader: Even more problems, and a solution.
The use of a serial/UART bootloader (a standard, not the one provided here) and at the same time using the internal RC
oscillator, potentially generates a serious problem. Suppose we know that the optimal
OSCCAL value for a specific atmega chip is 139 : We develop an Arduino application,
and right after setup() we write:

```C++
OSCCAL=139;
```

Seems good ?<br/>
**Unfortunately it is not working.**<br/>
The bootloader starts first, without knowing anything about the magic 139 value, and happily
waits for code from the UART.
If the chip happens to be badly factory calibrated, the serial communication will fail
and we will not be able to upload any code to the chip.

The solution provided here is simple and very robust. "osccal"
utility finds the  correct OSCCAL value, and then the (modified)ATmegaBOOT is compiled
against this specific OSCCAL value. Then it is uploaded to the chip. The first
think the bootloader does, is to Fix the RC frequency, allowing serial communications. For another atmega chip
the OSCCAL value will be different, and so on.

### 57600bps
57600bps introduces +2.1% error. Suppose we have calibrated the RC oscillator and the error is
+0.3%. Then the total error becomes +2.4% which is marginal.<br/>
I suggest if you really need to use
57600bps in your appplication, to use an OSCCAL = OPTIMAL_OSCCAL - 4 to compensate the error.
The clock ( millis() and friends ) will be ~2% slower than realtime however.<br/>
The modified ATmegaBOOT provided, does exactly this, but before jump to the application it sets
the OSCCAL to the optimal value (The nearest to 8Mhz), because it does not know which speed
the application uses.<br/>
So the strategy is:

```C++
// Use it with the modified AtmagaBOOT
// At this point the OSCCAL is  the optimal
// as the bootloader runs first
void setup() {
    // The clock will be a little slow
    // but serial communication will be perfect
    // Note that "osccal" (and modified AtmagaBOOT) avoids the values 128-131
    // and there is no danger to go from the upper OSCCAL region (128, 129, ...)
    // to the lower (..., 126, 127)
    // See Frequency-OSCCAL graph in the datasheet
    OSCCAL-=4;
    Serial.begin(57600);
    ....
}
```

or (recommended)

```C++
void setup() {
   // No OSCCAL manipulation is needed
   Serial.begin(38400);
   ....
}
```

### Reasons to use an external crystal
- Generally whenever you need better accuracy than the RC oscillator can
provide. Anything more accurate than 1% should be done with external crystal/resonator<br/>
- If you need the speed (up to 20Mhz).<br/>
- When the trouble to calibrate the RC oscillator outweighs
the trouble to install the crystal.<br/>
I believe using the "osccal" utility, it is much easier (or at least, this is my intention) to have a calibrated
atmega with a perfectly working bootloader, than to install the crystal. "osccal -b" is all that is needed.

### Reasons to use the internal oscillator

- Fewer parts on the breadboard/PCB. This
is usually the first reason that comes in mind, but it is also the least importand.
A crystal is usually a tiny part of the complexity and the cost of
a project. For simple projects is fine however, if we can avoid the crystal.
- Ability to change the frequency at runtime. For example we can drift
the 8Mhz frequency -2.1% for extremely reliable 57600 serial communication and
drift it +3.5% for 115200. Of course we can calibrate the RC oscillator to
the Serial friendly 7.37(28) Mhz frequency. Note however that if you write
Arduino code, better use 8Mhz. A lot of
useful Arduino functions like millis() work correctly only for 8Mhz and 16Mhz
- You have 2 additional GPIO pins. The XTAL1 and XTAL2 can be used for any purpose.
A lot of projects need a lot of GPIO pins, and 2 more pins
can make the difference. I include a very simple "library" to control these pins
[xtal.h](xtal.h)
- A lot of projects don't need any accuracy of RC oscillator.
- **Much faster startup from sleep mode.** This is the basic reason this page created.
I have a project where the MCU is in sleep, it is connected to a GSM modem with
hardware serial(UART), and wakes up from an incoming SMS (or a TCP packet).
Here is the message we get, when the (calibrated) RC oscillator is in use.<br/>
**"+CMT: "+30691234567","pkar","17/06/18,08:10:41+12"**<br/>
Here is the message, if we use a crystal<br/>
**S��������b���ɉ,"17/06/18,09:51:13+12"**<br/>
The crystal needs a lot of time to stabilize it's frequency. As you can
see the incoming phone number is lost. Sometimes even the date and time. Ok there are other solutions, for example flow control, but are complex.
- This one seems a little strange, but is totally valid. The internal oscillator
has a lot of [jitter](https://en.wikipedia.org/wiki/Jitter), not usually a good thing, but can be used as an excellent source of randomness. In conjunction with the Watchdog
timer (which has its own RC oscillator), can be used to generate ***true*** random numbers much faster than the
Crystal-Watchdog combination.

### How "osccal" utility works
When "osscal" utility runs, it installs the "calibrator.hex" file to the MCU. This Arduino sketch calculates the optimal OSCCAL, using the DS3231 RTC module as clock reference. If the LCD is installed, it displays the values to the tiny screen. Finally and most
importantly it saves the OSCCAL value as:

**EEPROM byte 0: 0x05<br/>
EEPROM byte 1: OSCCAL<br/>
EEPROM byte 2: 255-OSCCAL<br/>
EEPROM byte 3: 0x05**

After a few seconds "osccal" reads back the value from the EEPROM, and prints it
to the console.


### Assembling the hardware
Although the photo at the start of the page says it all, here are some instructions:
- You need a usbasp ISP programmer. I recommend to use a module with 3.3V/5V option and switch it to the voltage you are going to run the atmega328 after the calibration. Probably the voltage will be 3.3V as we talk about a 8Mhz system.
- a ZIF developer board and
- a DS3231 RTC module.
- a few female-female Dupont 2.54 cables.
- If you want visual feedback, you need also a 16x2 LCD and an LCD i2c adapter(search ebay), and to solder the secondary
i2c header of the DS3231 module. The LCD modules
come as 5V and 3.3V variants. Probably you need the 3.3V as mentioned above.

Developer Board | Rtc | LCD (if you use it) | Cable color
---|---|---|---
+/VCC  | VCC | VCC | Red
-/GND  | GND | GND | Black
PC4(Arduino A4)  |   SDA | SDA | Green
PC5(Arduino A5)  |   SCL | SCL | Yellow
PC3(Arduino A3)  |   SQW |   - | Gray


### Software installation
The following instructions are for the linux command line (tested with Ubuntu 22.04,linux mint 21.2).

```sh
# The development environment, and git of course
> sudo apt-get install avr-libc avrdude git
...
> cd ~/Projects  # Change with the directory you will put the rcCalibrator
> git clone https://github.com/pkarsy/rcCalibrator.git
> cd rcCalibrator
# "osccal" is a python script
> chmod +x osccal
```

Note: There is no much point trying to use the avrdude wich comes with Arduino tarbals, as the
standard (debian/ubuntu) avrdude works fine.

Optionally put the "osccal" executable to the PATH. In most desktop oriented distributions
a symlink is enough:
```sh
# if the ~/bin does not exist "mkdir ~/bin" and then logout and login again
> cd ~/bin
# Do NOT copy osccal, just symlink it
> ln -s ~/Projects/rcCalibrator/osccal
```

### Usage

There are 2 strategies:
- To find some "good" atmegas and use them on serial applications. This of
course works only if you have a lot of atmegas and only some of them
need to be calibrated. This method has the advantage that no
modification of existing code is needed. If you need 57600 speed this
method is unreliable however. See "Serial communication problems" above. In fact
chips with about -1.5% to -2.5% error (Not 0% !) work the best for 57600bps.
- To be used with a custom bootloader who sets the OSCCAL register at
startup. I have modified the ATmegaBOOT (used in
Arduino proMini) to do exactly this. It sets the speed at about -2% (Reduces OSCCAL register by 4) of
the optimal 8Mhz value, to make 57600 upload very reliable because it conpensates the +2.1% error
(See "Serial communication problems" above) , and just
before the application code starts, sets the OSCCAL to the optimal
value for 8Mhz frequency, Although it can easily support 115200bps
by drifting the speed to +3.5% of the optimal, I
wanted it to be interchangeable with ProMini 3.3V@8Mhz  so you don't need to
define/use a custom board in the Arduino build system. "Standard is better than
better"

### Running the "osccal" utility .
Connect the usbasp programmer (with the RTC) to a USB port,
attach a atmega chip, and run the executable :
```sh
# use the full path name ie "~/Projects/rcCalibrator/osccal" if
# the executable is not in the PATH
> osccal
```
After a few seconds you will see the optimal osccal value in the command
line. If you have the LCD, you will also see the results there.
So from the perspective of the computer, "osccal" is a command witch
gives a number as a result (The best OSCCAL value = the closer to 8Mhz).

### Modified AtmegaBOOT installation
This can be used with a bare atmega328 **without a crystal**. Dont do this on arduino boards.

```sh
> osccal -b
```

wait a few seconds ... ready !
The chip can now programmed as a proMini 3.3V @ 8MHz.
**Note however that even if you install a crystal, the chip will use the internal RC oscilator until you fix the fuses**

It is also important to note that the bootloader does NOT use any predefined EEPROM or FLASH location to read the OSCCAL. This avoids the danger to accidentally erase the EEPROM by the application (to store some data), with probably catastrophic results for the project. The bootloader is recompiled for every new chip and the OSCCAL value is saved in the bootloader area and is unique for this chip. This is the reason there is no precompiled HEX for this bootloader.

### Comparing ATmegaBoot(with OSCCAL support) with the stock ATmegaBOOT/optiboot
There are some pages around, that give instructions to use
an uncalibrated atmega328p with a 38400 bootloader(usually the stock optiboot or ATmegaBOOT). This is unreliable however, as some chips come from the factory with clock errors far worse than 2%. It is also non standard and requires an Arduino custom board definition (as far as I know).<br/>
The ATmegaBOOT Makefile included here, uses the "osccal" utility to find the correct OSCCAL value. It compiles the ATmegaBOOT against this value and then uploads the .hex file to the atmega328p chip. This chip can then be used just like a proMini to upload code with 57600bps. Indeed, according to my tests, the upload process is as reliable as with a crystal.

### applications without bootloader
You have to modify the Makefile of your project to use the "osscal" as a shell command and
get the OSCCAL value before write the application code. Somewhere inside the application
just after main() or setup() and before Serial initialization, put a

```C++
OSCCAL = OPTIMAL_OSCCAL_VALUE;
Serial.begin(38400);
```

or if you insist on using 57600bps

```C++
OSCCAL = OPTIMAL_OSCCAL_VALUE-4; // The MCU runs ~2% slower
Serial.begin(57600);
```

OPTIMAL_OSCCAL_VALUE must be passed to the gcc by the Makefile.
See the Makefile of the ATmegaBOOT bootloader, included here.

### Alternative method. Read the OSCCAL value from EEPROM
See [How "osccal" utility works](#how-osccal-utility-works) above.<br/>
The application reads the EEPROM and uses the OSCCAL value provided.
However I find the method quite fragile. A programming mistake can overwrite the contents of the EEPROM.

