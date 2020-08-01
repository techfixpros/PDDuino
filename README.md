# PDDuino
A hardware emulator of the Tandy Portable Disk Drive using an SD card for mass storage

[![Video of SD2TPDD running on a Teeensy 3.5](http://img.youtube.com/vi/_lFqsHAlLyg/hqdefault.jpg)](https://youtu.be/_lFqsHAlLyg "SD2TPDD on a Teensy 3.5")

[![Video of SD2TPDD running on a Adafruit Feather 32u4 Adalogger](http://img.youtube.com/vi/kQyY_Z1aGy8/hqdefault.jpg)](https://youtu.be/kQyY_Z1aGy8 "SD2TPDD on Adafruit Feather 32u4 Adalogger")

## Verbose Description
PDDuino is forked from SD2TPDD, and is largely still the same as SD2TPDD.  
This project aims to provide an easy-to-use, cheap, and reliable mass storage solution for the TRS-80 Model 100 series of computers.  

At the moment, PDDuino can:
* Emulate the basic file-access functions of a TPDD1
* Provide DME directory access

This fork adds:
* power saving sleep mode calls
* disk-activity led
* support for Teensy 3.5/3.6 special card reader hardware
* support for Adafruit Feather 32u4 Adalogger
* support for Adafruit Feather M0 Adalogger
* the current working directory is displayed in the top-right corner of the TS-DOS display

## Requirements
### Hardware
* Arduino Mega or compatible with at least one hardware serial port
* SD card reader  
* RS232 level shifter for the serial port going to the TPDD client (to the M100)

Example arduino-compatible boards with sd card reader already built-in:  
  [Adafruit Feather 32u4 Adalogger](https://learn.adafruit.com/adafruit-feather-32u4-adalogger)  
  [Adafruit Feather M0 Adalogger](https://learn.adafruit.com/adafruit-feather-m0-adalogger)  
  [Teensy 3.5](https://www.pjrc.com/store/teensy35.html)  
  [Teensy 3.6](https://www.pjrc.com/store/teensy36.html)  

### Software
* Arduino IDE
* SPI library
* Bill Greiman's SdFat library
* For Teensy: Teensyduino
* For Adafruit Feather M0: Arduino SAMD boards support, Adafruit SAMD boards support

## Assembly
### Hardware
* SD Card reader (if not using a board with built-in card reader):  
 Attach the SPI SD card reader to the microcontroller using its SPI bus.  
 Connect the SD card reader's chip select pin to the pin specified by the SD_CS_PIN variable (default is pin 4).  
 Attach the SPI SD card reader to the microcontroller's power rail.
* Serial port:  
 Attach an RS232 level shifter to the TX/RX pins of a hardware serial port.
 Power the RS232 level shifter from the microcontroller's power rail, not for example from a separate 5v source, to ensure the rx/tx signal levels coming from the level shifter will safely match the microcontroller.  
 Bridge the DTR and DSR pins on the RS232 connector (Required for TS-DOS).
### Software
* Load the source file into the Arduino IDE
* Download the SPI and SdFat libraries from the library manager
* Make any changes if needed
  There are many configuration options in the form of #defines at the top of the file.  
  The main one you need to set is PLATFORM to select what type of board to build for. Many other things automatically derive from that, for the few built-in supported boards liek teensy and adalogger.
* Compile the code and upload it to the microcontroller
  You will need to consult your board's documentation to set up the Arduino IDE correctly to to program the board.  
  This usually means installing one or more board support libraries, and selecting the board type from the tools menu.  
  In the case of Teensy, you also should install "Teensyduino", and there are more options on the tools menu such as setting the cpu clock speed. You can underclock the teensy to save even more battery.

## Power from BCR port  
<!-- ![](https://github.com/bkw777/BCR_USB_PWR/blob/master/BCR_USB_PWR.png)  -->
You can power the WiModem232 from the computer with this [BCR-USB-Power adapter](https://github.com/bkw777/BCR_USB_Breakout)  
 and a usb cable.

## Notes
If you plan on using Ultimate Rom II, it has a "TS-DOS" feature which works by loading TS-DOS into ram on the fly, from a file on disk. The file must be named DOS100.CO, and be in the root directory of the media. This file can be downloaded from here: http://www.club100.org/nads/dos100.co

## To-Do
* Document the various Arduino IDE setup and config quirks needed for each board.
* Teeny and/or TS-DOS installer

## other To-Dos, or merely ideas
* RTC  (Teensy has built-in rtc)
* play & record audio as virtual cassette  (Teensy has built-in audio, and enough cpu & ram to use it)
* Battery level (Adalogger has built-in voltage reference and adc, and a built-in lipo charger)
* A protocol expansion allowing access to files greater than 64KB in size
* Full NADSBox compatibility
* A command-line that can be accessed from the computer's terminal emulator for quicker file manipulation
* Hayes modem emulation using an ESP8266 (<https://www.cbmstuff.com/proddetail.php?prod=WiModem232>)
* FTP server/client access using an ESP8266

## BUGS
* Works with real TS-DOS, but file transfers don't actually work with TpddTool.py  
This seems to be due to mis-matches in handling the space-padding in the filenames?

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

* Displays the "PARENT.<>" directory entry even when you are already in the root dir.  
Goes away if you try to open PARENT.<> when you're already in root.


## Change-log
### 20191025 b.kenyon.w@gmail.com
* Support Adafruit Feather M0 Adalogger  
 needs "compiler.cpp.extra_flags=-fpermissive" in ~/.arduino15/packages/adafruit/hardware/samd/1.5.4/platform.local.txt
* Combine all boards supported into the same code  
 Time to break this out into a config.h file?

### 20180921 b.kenyon.w@gmail.com
* Support Teensy 3.5/3.6
* Support Adafruit Feather 32u4 Adalogger
* Macro-ify all serial port access, debug and tpdd client
* sleepNow() powersaving, idles at 3ma
* dmeLabel[] & setLabel() TS-DOS shows current working dir in top-right corner
* disk-activity led

---

### V0.2 (7/27/2018)
* Added DME support
* Corrected some file name padding bugs

### V0.1 (7/21/2018)
* Initial testing release with basic TPDD1 emulation
