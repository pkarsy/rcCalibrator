16/6/2017. Allow some days to finish the github page before download.

# OsccalCalibrator
Calibration of the internal RC oscillator of atmega328p chip, and OSCCAL aware UART bootloader(ATmegaBOOT).

### The problem
Most of the projects using AVR have a crystal or resontator connected to pins XTAL1 and XTAL2.
Using FUSE settings the MCU can use its internal RC oscillator. The problem is however that
the RC oscillator is not very well calibrated. At least for atmega328p the frequency can deviate up to
10% from the 8Mhz (usually 0-3%). This is unacceptable for example for
UART communications which tolerate up to ~2-3% error.
The AVR microcontrollers have a register called OSCCAL (Oscillator Calibration) which can
be used to drift the RC frequency and reduse the factory erron.

### The purpose of this project
- to find the optimal OSCCAL value
- to provide a mechanism via the "osccal" utility to automatically build bootloader and
application code capable of fixing the RC frequency. The prinary purpose of this is to
allow UART communications. If the project needs better accuracy than 1% the internal RC
oscillator cannot be used no matter how well it is calibrated.

### UART complications
As if the RC high error margin wasn't enough, the use of UART communications introduces
additional error, because the 8MHz clock speed is not divided exactly with the standard
Serial bitrates. See [wormfood tables](http://wormfood.net/avrbaudcalc.php) at 8Mhz

38400 +0.2%<br/>
57600 +2.1%   The proMinis are marginally capable of this speed<br/>
115200 -3.5%  This is the reason ProMini@8Mhz cannot do 115.2K

### UART bootloader even more complications
The use of a UART bootloader and at the same time using the internal RC
oscillator, is a "chicken and egg" problem. Suppose we know that the optimal
OSCCAL value for a specific atmega chip is 139 : We write an arduino application
and right after setup() we write
```C++
OSCCAL=139;
```
Seems good **but** it will not work (reliably) :<br/>
The bootloader starts before the application. If the chip happens to be
badly factory calibrated, we will not be able to upload any code to the chip.
The solution provided here is simple and I believe very robust. "osccal"
utility finds the  correct OSCCAL value and then the (modified)ATmegaBOOT is compiled
aginst this specific OSCCAL value. Then it is uploaded to the chip.

### Reasons to use an external crystal
Generally whenever you need better accuracy than the RC iscillator can
provide.<br/>
Or when the trouble to calibrate the RC oscillator outweights
the trouble to install the crystal.<br/>
Or if you need the speed (up to 20Mhz).<br/>
When the question arises in the AVR forums :<br/>
**"How to calibrate the internal avr oscillator"**<br/>
the usual answer is<br/>
**"Use a crystal with 2 (22pF)caps and avoid all the trouble."**<br/>
I believe this project reverses this. It is much easier, or at least this is my intention, to have a calibrated
atmega, than to install the crystal. This is because the process is mostly automatic
and consequently less error prone.

### Why use the internal oscillator

- Fewer parts on the breadboard/PCB. Less clutter and more reliable. This
is usually the first reason that comes in mind, but it is also the least importand.
A crystal is usually a tiny part of the complexity and the cost of
a project. For simple projects is fine however, if we can do without a crystal.
- Ability to change the frequency at runtime. For example we can drift
the 8Mhz frequency -2.1% for extremely reliable 57600 UART communication and
drift it +3.5% for 115200. Of course we can calibrate the RC oscillator to
the UART friendly 7.37(28) Mhz frequency. Note however that if you write
Arduino code, better use 8Mhz. A lot of
useful Arduino functions like millis() work correctly only for 8Mhz and 16Mhz
- You have 2 additional GPIO pins. The XTAL1 and XTAL2 can be used for any purpose.
A high voltage programmer for example, needs a lot of GPIO pins and 2 more pins
can make the difference.
- A lot of projects don't need any accuracy of RC oscillator and they don't use a
serial bootloader.
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

### How "osccal" utility works
When "osscal" utility runs, it installs the calibrator.hex file to the MCU. This code is
an arduino sketch which calculates the optimal OSCCAL, using the DS3231 RTC module as clock reference, and
saves the OSCCAL value as:

EEPROM byte 0: 0x05<br/>
EEPROM byte 1: OSCCAL<br/>
EEPROM byte 2: 255-OSCCAL<br/>
EEPROM byte 3: 0x05

If the lcd is installed, it also displays the values, which is extremely usefull if you need
to find some "good" atmega's
After a few seconds "osccal" reads back the value from the EEPROM of the atmega and prints it
to the console.

### How this project can be used

There are multiple strategies:
- To spot the "good" atmegas and use them on UART applications. This of
course works only if you have a lot of atmegas and only some of them
need to be calibrated. This method has the advandage that is simple and no
modification of existing code is needed. If you need 57600 speed this
method is unreliable however. See UART complications above.
- To be used with a custom bootloader who sets the OSCCAL register at
startup. I have modified the ATmegaBOOT (used in
Arduino proMini) to do exactly this. It sets the speed at about -2% (Reduces OSCCAL register by 4) of
the optimal 8Mhz value, to make 57600 upload very reliable because it conpensates the +2.1% error
(See UART complication above) , and just
before the application code starts, sets the OSCCAL to the optimal
value for 8Mhz frequency, Although it can easily support 115200bps
by drifting the speed to +3.5% of the optimal, I
wanted it to be interchangeable with ProMini 3.3V  so you don't need to
define/use a custom board in the Arduino build system. "Standard is better than
better"

### Build Instructions
Although the photo at the start of the page says it all, here is the bill of meterials.
- You need a usbasp ISP programmer **Picture** .I recommend to use a module with 3.3V/5V option and switch
it to the voltage you are going to run the atmega328 after the calibration.
Probably the voltage will be 3.3V as we talk about a 8Mhz system.
- a ZIF developer board and
- a DS3231 RTC module. You
- also need some female-female dupont 2.54 cables, better to be sort. If
you need visual feedback (I recommend it, at least for the first steps)
you need also a 16x2 LCD and a i2c adapter, and to solder the secondary
i2c header of the DS3231 module. A complication here is that the LCD modules
come as 5V and 3.3V versions. Probably you need the 3.3V as mentioned above.

Developer Board | Rtc | LCD (if you use it)
---|---|---
+/VCC  | VCC | VCC
-/GND  | GND | GND
PDX(Arduino A4)  |   SDA | SDA
PDX(Arduino A5)  |   SCL | SCL
A3   |   SQW |   -

WARNING: if you are unfamiliar with ISP programmers and how to reprogram
a atmega328 chip do not go further.

How it works. The following instructions are for the linux command line.
I suppose they can be adapted for windows, but i didn't test it.
When you download the github page<br/>
```sh
> git clone https://github.com/pkarsy/OsccalCalibrator.git
> cd OsccalCalibrator
> chmod +x osccal
```
Optionally put he "osccal" executable to the PATH.
Connect the usbasp programmer (with the RTC) to a USB port
attach a atmega chip (WARNING whatever it has in the FLASH, it will be erased, including the
bootloader) and do a
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
See "How "osccal" utility works" above
It is possible that the application reads the EEPROM and uses the OSCCAL value provided.
However I find the method quite fragile. A programming mistace can overwrite the contents
of the EEPROM. The only good this method has is there is no need for recompilation for every
atmega chip, but again the compilation is quite fast.

