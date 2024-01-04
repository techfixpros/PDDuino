# PDDuino
A [Tandy Portable Disk Drive](http://tandy.wiki/TPDD) emultaor implemented on Arduino, using a micro-sd card for storage.

[![Video of PDDuino doing TPDD2-style bootstrap installing TS-DOS](https://img.youtube.com/vi/LTtkjnuTSZw/hqdefault.jpg)](https://youtu.be/LTtkjnuTSZw "PDDuino + Feather MounT + BCR_Breakout")

<!-- old gmail acct
[![Video of PDDuino doing TPDD2-style bootstrap installing TS-DOS](https://img.youtube.com/vi/3PorYduc5lk/hqdefault.jpg)](https://youtu.be/3PorYduc5lk "PDDuino + Feather MounT + BCR_Breakout")
-->

<!--
Earlier stages:<br>
[![Video of PDDuino running on a Teeensy 3.5](http://img.youtube.com/vi/_lFqsHAlLyg/hqdefault.jpg)](https://youtu.be/_lFqsHAlLyg "SD2TPDD on a Teensy 3.5")

[![Video of PDDuino running on a Adafruit Feather 32u4 Adalogger](http://img.youtube.com/vi/kQyY_Z1aGy8/hqdefault.jpg)](https://youtu.be/kQyY_Z1aGy8 "SD2TPDD on Adafruit Feather 32u4 Adalogger")

[![Video of PDDuino doing TPDD2-style bootstrap installing TEENY](https://img.youtube.com/vi/3ZgceFy4YZs/hqdefault.jpg)](https://youtu.be/3ZgceFy4YZs "TPDD2-style bootstrap")
-->

PDDuino was based originally on [SD2TPDD](https://github.com/tangentdelta/SD2TPDD) by Jimmy Petit.  

DISCLAIMER: This is not a great reference for a TPDD server yet. [dlplus2](github.com/bkw777/dl2) is.  
[pdd.sh](github.com/bkw777/pdd.sh) is a good reference for a TPDD client, though not especially easy to follow the bash code since it is written to conform to a purist standard as a stunt more than to be easy to read and follow.  
The next major TODO is to rewrite this modelled after dlplus as a reference for the correct way to handle each request from the client.  

## Features
* Tandy Portable Disk Drive 1 emulation
* TS-DOS directories<br>
* TPDD2-style bootstrapper to install a [TPDD Client](http://tandy.wiki/TPDD_client) onto the computer<br>

## Hardware support
PDDuino runs on most any Arduino-compatible microcontroller.  

Configs are already included to support the following particularly handy development boards that have a small form factor, built-in usb power & console, and built-in SD-card reader, and which are supported by [MounT](https://github.com/bkw777/MounT) and [BCR_Breakout](https://github.com/bkw777/BCR_Breakout) for the serial connection, power, and physical attachment to the computer.  
 * Teensy [3.5](https://www.pjrc.com/store/teensy35.html) and [3.6](https://www.pjrc.com/store/teensy36.html)  ([4.1](https://www.pjrc.com/store/teensy41.html) pending)  
 * Adafruit [Feather 32u4 Adalogger](https://www.adafruit.com/product/2795) and [Feather M0 Adalogger](https://www.adafruit.com/product/2796)

PDDuino may be used with any machine that has [TPDD client](http://tandy.wiki/TPDD_client) software or firmware.  
 * TRS-80/TANDY Models 100, 102, 200, WP-2  
 * NEC PC-8201, PC-8201A, PC-8300, PC-8401A, PC-8500  
 * Olivetti M10  
 * Kyotronic KC-85  
 * [Cambridge Z88](http://www.rakewell.com/xob/xob.shtml)  
 * [Brother knitting machines](http://www.k2g2.org/wiki:brother_fb-100)  
 * [Anything with a python interpreter](https://trs80stuff.net/tpdd/)  
 * [pdd.sh](https://github.com/bkw777/pdd.sh)  


## Requirements / Setup
### Software
[Arduino IDE Setup](Arduino_IDE_Setup.txt)

### Hardware
* Arduino-compatible microcontroller board with at least one hardware serial port
* SD card reader
* RS-232 to TTL/CMOS level shifter
* Serial cable
* Battery or usb power source for the microcontroller board

These boards have a small form-factor, and sd-card reader built-in:  
  [Adafruit Feather 32u4 Adalogger](https://learn.adafruit.com/adafruit-feather-32u4-adalogger)<br>
  [Adafruit Feather M0 Adalogger](https://learn.adafruit.com/adafruit-feather-m0-adalogger) (20200916 broken)<br>
  [Teensy 3.5](https://www.pjrc.com/store/teensy35.html)<br>
  [Teensy 3.6](https://www.pjrc.com/store/teensy36.html)<br>
  [Teensy 4.1](https://www.pjrc.com/store/teensy41.html) (not yet tested)

This adapter takes the place of a serial cable, and includes the level-shifter:<br>
  [MounT](https://github.com/bkw777/MounT)

This adapter can power the microcontroller board from the M100:<br>
  [BCR-USB-Power adapter](http://www.github.com/bkw777/BCR_Breakout)

**The following below are NOT needed if using the MounT adapter.  
You would only need these for bread-boarding or building in a box attached by a cable.**

 RS-232&lt;--&gt;TTL/CMOS level-shifter module:  
  [NulSom](https://www.amazon.com/dp/B00OPU2QJ4/)  
  Has a male connector and DTE pinout like a PC  
  Use the same special null-modem cable as for connecting a "Model T" to a PC.  
  Solder jumper wires on the back of the 9-pin connector to join pins 1, 4, and 6  
  [DSR-DTR-DCD Wiring, top](https://photos.app.goo.gl/oBpAbAbCgE8yCJzm7)  
  [DSR-DTR-DCD Wiring, bottom](https://photos.app.goo.gl/mj1SXce5fJnmZiyr6)  
  This short-circuits the DSR/DTR detection, which pacifies TS-DOS so it will run, but means you can't test the bootstrap function except by wiring up a fake DTR/DSR signal using a pulldown resistor to gnd and a momentary button to vcc/3v3 on gpio pin 6.  

 RS-232 cable:  
  [PCCables 0103](https://www.pccables.com/products/00103.html)  
  Or [Any of these](http://tandy.wiki/Model_T_Serial_Cable)

 TODO: find a ttl-serial module that actually supports the dsr/dtr lines.  
  https://www.pololu.com/product/126   breadboard-friendly single row of pins  
  https://www.amazon.com/dp/B0190WSINY/  double row pin header needs jumper wires to a breadboard  
  Female plug, DCE pinout, needs a different serial cable, or adapters.

## Usage:
### Normal usage
Use as you would use a real [TPDD](http://tandy.wiki/TPDD) or any other [TPDD emulator](http://tandy.wiki/TPDD_Emulators).

### Bootstrap Procedure
Assuming you are using a [MounT](https://github.com/bkw777/MounT) adapter to host a Feather or Teensy.  
Assuming you are using the [BCR-USB](https://github.com/bkw777/BCR_Breakout) adapter to power the MounT.  
Assuming the portable is a Model 100.  
Assuming you want to install TS-DOS.  

1. Start with the Model 100 turned off.  
 Plug the MounT and BCR adapters into the Model 100 and connect the usb cable from the BCR adapter to the MounT.  
 Eject the SD card.

1. Place an ascii format BASIC loader file on the root of the SD card, renamed as LOADER.DO.  
You can use any of the loader files from [dlplus](https://github.com/bkw777/dlplus/tree/master/clients).  
&nbsp;  
Example 1: To install TS-DOS on a TRS-80 Model 100: Take [TS-DOS.100](https://raw.githubusercontent.com/bkw777/dlplus/master/clients/ts-dos/TS-DOS.100), and save it as ```LOADER.DO``` on the root of the SD card.  
Note the associated [pre-install](https://raw.githubusercontent.com/bkw777/dlplus/master/clients/ts-dos/TS-DOS.100.pre-install.txt) and [post-install](https://raw.githubusercontent.com/bkw777/dlplus/master/clients/ts-dos/TS-DOS.100.post-install.txt) directions.  
&nbsp;  
Example 2: To install DSKMGR on an Olivetti M10: Take [DSKMGR.M10](https://raw.githubusercontent.com/bkw777/dlplus/master/clients/dskmgr/DSKMGR.M10), and save it as ```LOADER.DO``` on the root of the SD card.  
Note the associated [pre-install](https://raw.githubusercontent.com/bkw777/dlplus/master/clients/dskmgr/DSKMGR.M10.pre-install.txt) and [post-install](https://raw.githubusercontent.com/bkw777/dlplus/master/clients/dskmgr/DSKMGR.M10.post-install.txt) directions.  

1. Turn on the Model 100 while the SD card is still ejected.  
The Arduino board should now have a steady slow blinking LED, indicating it's waiting for an SD card.  
Don't insert the SD card yet.

1. In BASIC, type ```RUN "COM:98N1E"``` and press Enter.  
(Or follow the pre-install directions are for the loader you're using. The exact command is different for different models of computer. This example is for TRS-80 Model 100 or 102.)

1. Insert the SD card.

1. Wait while the loader program is transferred and then executed.  
For approximately 20 seconds (exact time depends on the size of LOADER.DO) there will be no indication of acivity on the computer, and the LED on the Arduino board will be ON solid and not blinking. The loader program is being sent to the computer during this time.  
&nbsp;  
After the LED goes out, the computer executes the program that was just loaded.  
&nbsp;  
Follow any on-screen directions (different loaders have different prompts & directions), and the post-install directions for the loader you're using.  
In the case if TS-DOS, this would be:  
&nbsp;&nbsp;Type ```SAVE "TS-DOS"``` and press Enter.  
&nbsp;&nbsp;Exit BASIC (press ```F8``` or type ```MENU```)  
&nbsp;&nbsp;Run ```TS-DOS.BA``` from the main menu.  
&nbsp;&nbsp;Use ```F2-Kill``` to delete ```TMP.DO```  

You now have the ram version of TS-DOS (or other tpdd client) installed.  
You can immediately use it to browse the contents of the SD card.

## Notes
### Ultimate ROM II TS-DOS loader
If you plan on using Ultimate Rom II, it has a "TS-DOS" feature which works by loading TS-DOS into ram on the fly, from a file on disk.  
The file must be named DOS100.CO, and be in the root directory of the media.  
This file can be downloaded from <http://www.club100.org/nads/dos100.co>, or,  
a modified/updated version can be found here: <http://www.club100.org/memfiles/index.php?direction=0&order=&directory=Ken%20Pettit/NewDos>

### "Model T" serial port control lines
At power-on the Model 100 rs232 port sets all data & control pins to -5v.  
On RUN "COM:98N1E", pins 4 and 20 go to +5v.  

## To-Dos, or merely ideas, not necessarily realistic
* Change "PARENT.<>" to "..<>" if possible
* https://www.arduinolibraries.info/libraries/double-reset-detector_generic
* https://github.com/rocketscream/Low-Power  
  simplify the sleep calls, same lib for both avr and samd21  
* RTC (Teensy has built-in rtc)
* play & record audio as virtual cassette (Teensy has built-in audio, and enough cpu & ram to use it)
* Battery level (Adalogger has built-in voltage reference and adc, and a built-in lipo charger)
* A command-line that can be accessed from the computer's terminal emulator for quicker file manipulation
* Hayes modem emulation using an ESP8266 (<https://www.cbmstuff.com/proddetail.php?prod=WiModem232>)
* FTP server/client access using an ESP8266

## BUGS/STATUS
* 20200916 Feather M0 isn't working.  
  Unknown why. It used to work.

* Works with TS-DOS.

* Doesn't work with TEENY. (hangs)

* File transfers don't work with TpddTool.py .  
Seems to be due to mis-matches in handling the space-padding in the filenames?  
Problem may be at least partly in TpddTool.py

* Some kind of working directory initial/default state issue, which affects Ultimate Rom II loading DOS100.CO on the fly.
If you have DOS100.CO in ram, then UR-2 works all the time, because it will use that copy if available.  
TS-DOS from rom or ram, not via UR-2, seems to be working pretty well all the time.  
TS-DOS from rom or ram, not via UR-2, can successfully load a file like DOS100.CO from the root dir, but Ultimate Rom 2 usually can not load that same file, but sometimes it can.  
If you try to use TS-DOS from UR2 after a fresh power-cycle of both Arduino and M100, It doesn't work.  
But If you load TS-DOS some other way (for example, use a REX to switch to TS-DOS rom), and use TS-DOS to read the directory listing once, then switch roms back to UR-2, THEN the TS-DOS menu entry in UR-2 works (successfully loads DOS100.CO from the disk).  
Maybe TS-DOS does some kind of initialization that UR-2 isn't doing?  
UR-2 works fine with a real TPDD/TPDD2 and other emulators like dlplus and LaddieAlpha, so maybe there is some sort of default condition that we should be resetting to?  

* If you use UR-2 to load TS-DOS in ram, and switch to a subdirectory like /Games while in TS-DOS, then exit TS-DOS, then you can't use TS-DOS any more, because the next time UR-2 tries to load DOS100.CO from disk, SD2TPDD looks for /Games/DOS100.CO, which does not exist.

* When UR-2 loads DOS100.CO, sucessfully or not, the LED doesn't shut off after.

* Sometimes displays the "PARENT.<>" directory entry even when you are already in the root dir.  
Goes away if you try to open PARENT.<> again when you're already in root.


## Change-log

### 20210404
* 0.5.0 Incorporated Jim Brain's replacement return_*() and main loop.

### 20200819
* Moved PCB to its own [repo](https://github.com/bkw777/MounT)  
* Added TPDD2-style bootstrap function  

### 20200817
* Added PCB adapter "PDDuino_Feather".  
 Takes the place of the serial cable and ttl-rs232 module.  
 Supports Adafruit Feather 32u4 Adalogger and Adafruit Feather M0 Adalogger.  

### 20191025
* Support Adafruit Feather M0 Adalogger  
 needs "compiler.cpp.extra_flags=-fpermissive" in ~/.arduino15/packages/adafruit/hardware/samd/1.5.4/platform.local.txt
* Combine all boards supported into the same code  
 Time to break this out into a config.h file?

### 20180921
* Support Teensy 3.5/3.6
* Support Adafruit Feather 32u4 Adalogger
* Macro-ify all serial port access, debug and tpdd client
* sleepNow() powersaving, idles at 3ma
* dmeLabel[] & setLabel() TS-DOS shows current working dir in top-right corner
* disk-activity led

### SD2TPDD - PDDuino

### v0.2 (7/27/2018)
* Added DME support
* Corrected some file name padding bugs

### v0.1 (7/21/2018)
* Initial testing release with basic TPDD1 emulation
