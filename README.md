# PDDuino
A hardware emulator of the Tandy Portable Disk Drive using an SD card for mass storage

[![Video of PDDuino running on a Teeensy 3.5](http://img.youtube.com/vi/_lFqsHAlLyg/hqdefault.jpg)](https://youtu.be/_lFqsHAlLyg "SD2TPDD on a Teensy 3.5")

[![Video of PDDuino running on a Adafruit Feather 32u4 Adalogger](http://img.youtube.com/vi/kQyY_Z1aGy8/hqdefault.jpg)](https://youtu.be/kQyY_Z1aGy8 "SD2TPDD on Adafruit Feather 32u4 Adalogger")

## Verbose Description
PDDuino is forked from SD2TPDD.  
This project aims to provide an easy-to-use, cheap, and reliable mass storage solution for the TRS-80 Model 100 series of computers.  

At the moment, PDDuino can:
* Emulate the basic file-access functions of a TPDD1
* Provide DME directory access

This fork adds:
* Power saving sleep mode calls
* Disk-activity led
* Support for Teensy 3.5/3.6 special card reader hardware
* Support for Adafruit Feather 32u4 Adalogger
* Support for Adafruit Feather M0 Adalogger
* Current working directory is displayed in TS-DOS
* TPDD2-style bootstrapper

## Requirements
### Hardware
* Arduino Mega or compatible with at least one hardware serial port
* SD card reader  
* RS232 level shifter for the serial port going to the TPDD client (to the M100)

Arduino-compatible boards with sd card reader already built-in:  
  [Adafruit Feather 32u4 Adalogger](https://learn.adafruit.com/adafruit-feather-32u4-adalogger)  
  [Adafruit Feather M0 Adalogger](https://learn.adafruit.com/adafruit-feather-m0-adalogger)  
  [Teensy 3.5](https://www.pjrc.com/store/teensy35.html)  
  [Teensy 3.6](https://www.pjrc.com/store/teensy36.html)  
  (not yet tested/ported, way overkill)[Teensy 4.1](https://www.pjrc.com/store/teensy41.html)  
  (not yet tried, TODO)[OpenLog](https://www.ebay.com/sch/i.html?_nkw=OpenLog)  

RS-232<-->TTL level-shifter module:  
  [NulSom](https://www.amazon.com/dp/B00OPU2QJ4/)  
  (Has DTE pinout. Use the same null-modem cable as for a PC bwith no other adapters needed.)

RS-232 cable:  
  With the specific level-shifter module above, with male pins and DTE pinout: [PCCables 0103](https://www.pccables.com/products/00103.html)  
  Or [Any of these](http://tandy.wiki/Model_100_102_200_600_Serial_Cable)

Optional: [BCR-Power adapter](http://www.github.com/bkw777/BCR_Breakout/)

### Software
* Arduino IDE
* SPI library
* Bill Greiman's SdFat library
* For Teensy: Teensyduino
* For Adafruit Feather M0: Arduino SAMD boards support, Adafruit SAMD boards support

## Assembly
### Hardware

#### New hardware using custom adapter board - STILL IN TESTING
See [MonT](https://github.com/bkw777/MonT)  

#### Original hardware using a serial cable
See http://tandy.wiki/TPDD_Cable  
* SD Card reader (if not using a board with built-in card reader):  
 Attach the SPI SD card reader to the microcontroller using its SPI bus.  
 Connect the SD card reader's chip select pin to the pin specified by the SD_CS_PIN variable (default is pin 4).  
 Attach the SPI SD card reader to the microcontroller's power rail.
* Serial port:  
 Attach an RS232 level shifter to the TX/RX pins of a hardware serial port.
 Power the RS232 level shifter from the microcontroller's power rail, not for example from a separate 5v source, to ensure the rx/tx signal levels coming from the level shifter will safely match the microcontroller.  

 DSR/DTR. You have a couple of options:  
 Otion 1:  
   10-30k pulldown resistor from GPIO pin 6 to GND:  
   GPIO pin 6 --- 15k --- GND  

   Bridge the DTR and DSR pins on the RS232 connector.

   Optionally install a momentary pushbutton to manually bring pin 6 high:  
   GPIO pin 6 --- button --- 330ohm --- 3v3

 Option 2:  
   10-30k pulldown resistor from GPIO pin 6 to GND:  
   GPIO pin 6 --- 15k --- GND

   Connect DTR from M100 through MAX3232 to arduino gpio pin 6:  
   GPIO pin 6 --- MAX3232 R2OUT  
   MAX3232 R2IN --- M100 DTR (DB25 pin 20)  

   Connect GPIO pin 5 through MAX3232 to M100 DSR pin:  
   GPIO pin 5 --- MAX3232 T2IN  
   MAX3232 T2OUT --- M100 DSR (DB25 pin 6)


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
You can power the Arduino from the computer with this [BCR-USB-Power adapter](https://github.com/bkw777/BCR_Breakout)  
 and a usb cable.

## Usage:
### Bootstrap
Place an ascii format BASIC file named LOADER.DO on the root of the SD card.  
Example, take [TEENY.100](https://raw.githubusercontent.com/bkw777/dlplus/master/clients/teeny/TEENY.100) from [dlplus](https://github.com/bkw777/dlplus),  
and save it as LOADER.DO on the root of the SD card.

Power-off the arduino and insert the sdcard.

In BASIC do RUN "COM:98N1ENN" and press enter.

If you're using DSR/DTR option 1 above with the pushbutton, press and hold the button now.

Power-on the arduino.

As soon as the LED comes on steady you can release the button.

Follow the [post-install directions for TEENY.100](https://raw.githubusercontent.com/bkw777/dlplus/master/clients/teeny/TEENY.100.post-install.txt) from dlplus.

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
### 20200819 b.kenyon.w@gmail.com
* moved PCB to its own repo
* added TPDD2-style bootstrap function

### 20200817 b.kenyon.w@gmail.com
* Added PCB adapter "PDDuino_Feather".  
 Takes the place of the serial cable and ttl-rs232 module.  
 Supports Adafruit Feather 32u4 Adalogger and Adafruit Feather M0 Adalogger.  

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
