16/6/2017. Allow some days to finish the github page before download.

![OsccalCalibrator](OsccalCalibrator.jpg "Upper Line Default osccal")

# OsccalCalibrator
Calibration of the internal RC oscillator of atmega328p chip, and OSCCAL aware UART bootloader(ATmegaBOOT).
The hardware consists of a USBasp programmer and a DS3231 module, and of course the programmer can be
used for its normal purpose to flash the chips.

### WARNING
The use of this utility will erase all the contents of your MCU without notice.
Use it only if you are familiar with ISP programming and know how to set the chip to the old state.

### The problem
Most of the projects using atmega328p, have a crystal or resonator connected to pins XTAL1 and XTAL2.
If you don't mind to run at 8Mhz, you can use FUSE settings to set the MCU to use its internal RC oscillator.
The problem is however that
the RC oscillator is sometimes not very well calibrated. At least for atmega328p the frequency can deviate up to
10% from the 8Mhz (usually 0-3% and to be fair, most of the time, very close to 0%). This is a problem for
UART communications, which tolerate up to ~2-3% error.
The AVR microcontrollers have a register called OSCCAL (Oscillator Calibration) which can
be used to drift the RC frequency and reduse the error to less than 1%.

### The purpose of this project
There are a lot of pages to address the calibration problem, and atmel has released a lot of
related papers. This project aims to offer an alternative solution :
- To find the optimal OSCCAL value, using an ISP programmer and a DS3231 rtc module.
- **More importantly** to provide a mechanism via the "osccal" utility to automatically build bootloader and
application code capable of fixing the RC frequency. The prinary purpose of this is to
allow UART(Serial) communications.

### UART problems
The use of UART communications introduces unfortunatelly
another type of error, because the 8MHz clock speed is not divided exactly with the standard
serial bitrates. See [wormfood tables](http://wormfood.net/avrbaudcalc.php) at 8Mhz

```
38400   +0.2%   This is an excellent choise when running at 8Mhz
57600   +2.1%   The proMinis(3.3V 8Mhz) are capable of this serial speed
115200  -3.5%   This is the reason ProMini@8Mhz cannot do 115.2k
```

### UART bootloader: Even more problems
The use of a UART bootloader and at the same time using the internal RC
oscillator, is a subtle problem. Suppose we know that the optimal
OSCCAL value for a specific atmega chip is 139 : We write an arduino application
and right after setup() we write
```C++
OSCCAL=139;
```
Seems good ?<br/>
**Unfortunatelly it is not working reliably :**<br/>
The bootloader starts before the application and knows kothing about the magic 139 value. If the chip happens to be
badly factory calibrated, we will not be able to upload any code to the chip.<br/>
The solution provided here is simple and very robust. "osccal"
utility finds the  correct OSCCAL value and then the (modified)ATmegaBOOT is compiled
against this specific OSCCAL value. Then it is uploaded to the chip. The first
think the bootloader does, is to Fix the RC frequency. For another atmega chip
the OSCCAL value will be different, and so on.

### Reasons to use an external crystal
- Generally whenever you need better accuracy than the RC oscillator can
provide. Anything more acurrate than 1% should be done with external crystal/resonator<br/>
- If you need the speed (up to 20Mhz).<br/>
- When the trouble to calibrate the RC oscillator outweights
the trouble to install the crystal.<br/>
When the question arises in the AVR forums :<br/>
**"How to calibrate the internal avr oscillator"**<br/>
the usual answer is<br/>
**"Use a crystal with 2 (22pF)caps and avoid all the trouble."**<br/>
I believe this project reverses this. It is much easier, or at least this is my intention, to have a calibrated
atmega with a perfectly working bootloader, than to install the crystal. This is because the process is mostly automatic
and consequently less error prone.

### Why to use the internal oscillator

- Fewer parts on the breadboard/PCB. Less clutter and more reliable. This
is usually the first reason that comes in mind, but it is also the least importand.
A crystal is usually a tiny part of the complexity and the cost of
a project. For simple projects is fine however, if we can avoid the crystal.
- Ability to change the frequency at runtime. For example we can drift
the 8Mhz frequency -2.1% for extremely reliable 57600 UART communication and
drift it +3.5% for 115200. Of course we can calibrate the RC oscillator to
the UART friendly 7.37(28) Mhz frequency. Note however that if you write
Arduino code, better use 8Mhz. A lot of
useful Arduino functions like millis() work correctly only for 8Mhz and 16Mhz
- You have 2 additional GPIO pins. The XTAL1 and XTAL2 can be used for any purpose.
A lot of projects need a lot of GPIO pins, and 2 more pins
can make the difference.
- A lot of projects don't need any accuracy of RC oscillator.
- **Much faster startup from sleep mode.** This is the reason this project exists.
I have a project where the MCU is in sleep, it is connected to a GSM modem with
hardware serial, and wakes up from an incoming
 SMS. Here is the message it receives when it uses a crystal<br/>
**"*%&%^$^&%*&%&*%^*&^456 ...."**<br/>
The crystal needs a lot of time to stabilize it's freequency. As you can
see we are able to get only the last 3 digits of the incoming phone number.
Here is the message, when the (calibrated) RC oscillator is in use.<br/>
**"+CMT: "+30691234567","pkar","17/06/18,00:10:41+12"**<br/>
This time we did't lose a single character.
- This one seems a little strange, but is totally valid. The internal oscillator
has a lot of jitter, making it an exellent source of randomness. In conjunction with the Watchdog
timer (which has its own RC oscillator) Can be used to generate random numbers much faster than the
crystal-Watchdog combination. See https://github

### How "osccal" utility works
When "osscal" utility runs, it installs the "calibrator.hex" file to the MCU. This code is
an arduino sketch which calculates the optimal OSCCAL, using the DS3231 RTC module as clock reference, and
saves the OSCCAL value as:

EEPROM byte 0: 0x05<br/>
EEPROM byte 1: OSCCAL<br/>
EEPROM byte 2: 255-OSCCAL<br/>
EEPROM byte 3: 0x05

After a few seconds "osccal" reads back the value from the EEPROM of the atmega and prints it
to the console.<br/>
If the LCD is installed, it also displays the values, which is extremely usefull if you need
to find some "good" atmega's. Note that there are in fact a lot of "good" chips.<br/>

### How this project can be used

There are multiple strategies:
- To find some "good" atmegas and use them on UART applications. This of
course works only if you have a lot of atmegas and only some of them
need to be calibrated. This method has the advandage that no
modification of existing code is needed. If you need 57600 speed this
method is unreliable however. See UART complications above. In fact
chips with about -1.5% to -2.5% error (Not 0% !) work the best for 57600bps.
- To be used with a custom bootloader who sets the OSCCAL register at
startup. I have modified the ATmegaBOOT (used in
Arduino proMini) to do exactly this. It sets the speed at about -2% (Reduces OSCCAL register by 4) of
the optimal 8Mhz value, to make 57600 upload very reliable because it conpensates the +2.1% error
(See UART complication above) , and just
before the application code starts, sets the OSCCAL to the optimal
value for 8Mhz frequency, Although it can easily support 115200bps
by drifting the speed to +3.5% of the optimal, I
wanted it to be interchangeable with ProMini 3.3V@8Mhz  so you don't need to
define/use a custom board in the Arduino build system. "Standard is better than
better"

### Build Instructions
Although the photo at the start of the page says it all, here are some instructions:
- You need a usbasp ISP programmer. I recommend to use a module with 3.3V/5V option and switch
it to the voltage you are going to run the atmega328 after the calibration.
Probably the voltage will be 3.3V as we talk about a 8Mhz system.
- a ZIF developer board and
- a DS3231 RTC module. You
- also need some female-female dupont 2.54 cables, better to be sort.
- If you want visual feedback, you need also a 16x2 LCD and an i2c adapter, and to solder the secondary
i2c header of the DS3231 module. The LCD modules
come as 5V and 3.3V variants. Probably you need the 3.3V as mentioned above.

Developer Board | Rtc | LCD (if you use it) | Cable color
---|---|---|---
+/VCC  | VCC | VCC | Red
-/GND  | GND | GND | Black
PC4(Arduino A4)  |   SDA | SDA | Green
PC5(Arduino A5)  |   SCL | SCL | Yellow
A3   |   SQW |   - | Gray


How it works. The following instructions are for the linux command line.
I suppose they can be adapted for Windows, but i didn't test it.
```sh
# The arduino development environment and the excellent Arduino-Makefile
> sudo apt-get install arduino-core arduino-mk
...
> git clone https://github.com/pkarsy/OsccalCalibrator.git
> cd OsccalCalibrator
> chmod +x osccal
```
Optionally put he "osccal" executable to the PATH.
Connect the usbasp programmer (with the RTC) to a USB port
attach a atmega chip (WARNING whatever it has in the FLASH, it will be erased, including
the bootloader) and do a
```sh
> ./osccal
```
After a few seconds you will see the optimal osccal value in the command
line. If you have the LCD you will also see the results there.
So from the perspective of the computer, "osccal" is a command witch
 gives a number as a result (The best OSCCAL value = the closer to 8Mhz)
If your intention is to just find some "good" MCUs then this is the end of
the story.

### Why to use the MCU with a modified UART bootloader.

There are some pages around, that give instructions to use
an uncalibrated atmega328p with a 38400 bootloader. This is VERY
unreliable, as a lot of chips come from the factory with clock errors
far worse than 2%. It is also non standard and requires an Arduino custom board definition.
The ATmegaBOOT Makefile included here, uses the "osccal" utility
to find the correct OSCCAL value. It compiles
the ATmegaBOOT against this value and then uploads the .hex file to the atmega328p chip. This chip
can then be used just like a proMini to upload code with 57600bps.
According to my tests the upload process is bulletproof.

### Modified AtmegaBOOT installation
The process is quite automatic. Go to the folder where you downloaded OsccalCalibrator
```sh
> cd atmegaBOOT
# The Makefile uses "osccal" utility to find the optimal OSCCAL value
> make isp
```
wait a few seconds ... ready !

It is important to note that the bootloader does NOT use any predefined
EEPROM location to read the OSCCAL. This avoids the danger to accidentally erase
the EEPROM by the application (to store some data), with probably catastrophic results
for the project. The bootloader is recompiled for every
new chip and the OSCCAL value is saved in the bootloader
area and is unique for this chip. This is the reason there is no precompiled HEX for this
bootloader.

### rfboot
I have written the bootloader [rfboot](https://github.com/pkarsy/rfboot) which can (optionally) set the optimal OSCCAL before
jump to the application. The bootloader does not need any OSCCAL calibration to work (it uses SPI),
but any UART device the project is using, might need it. As shown abobe, 38400bps is a very
good choice for 8Mhz systems.

### applications without bootloader
You have to modify the Makefile of your project to use the "osscal" as a shell command and
get the best OSCCAL value before burn the application code. Somewhere inside the application
just after main() or setup() and before Serial.begin() setup put a

```C++
OSCCAL = CALIBRATED_OSCAL_VALUE;
```

CALIBRATED_OSCAL_VALUE must be passed to the gcc by the Makefile.
See the Makefile of the ATmegaBOOT bootloader, included in this site.

### Alternative method. Read the OSCCAL value from the EEPROM
See [How "osccal" utility works](README.md#how-osccal-utility-works) above.<br/>
The application reads the EEPROM and uses the OSCCAL value provided.
However I find the method quite fragile. A programming mistake can overwrite the contents
of the EEPROM. The only good this method has is : There is no need to recompile the bootloader for every
atmega chip. The compilation is much faster than the upload however, so I don't find this a good reason to
use this method.

