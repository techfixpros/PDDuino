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
#### See [MonT](https://github.com/bkw777/MonT) (MCU on Model T)  
A MonT adapter takes the place of the serial cable and ttl/rs232 tranceiver described below.

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
(If not using a MonT board)

See http://tandy.wiki/TPDD_Cable  
* SD Card reader (if not using a board with built-in card reader):  
 Attach the SPI SD card reader to the microcontroller using its SPI bus.  
 Connect the SD card reader's chip select pin to the pin specified by the SD_CS_PIN variable (default is pin 4).  
 Attach the SPI SD card reader to the microcontroller's power rail.
* Serial port:  
 Attach an RS232 level shifter to the TX/RX pins of a hardware serial port.
 Power the RS232 level shifter from the microcontroller's power rail, not for example from a separate 5v source, to ensure the rx/tx signal levels coming from the level shifter will safely match the microcontroller.  
TS-DOS requires DTR/DSR. You can do a couple different options:
Option 1:
  Bridge the DTR and DSR pins on the RS232 connector on the Model-100 end.
  Tie gpio pin 6 to ground with a 10-30k pulldown resistor.
  Optionally, also add a momentary pushbutton and a 150ohm resistor to override the pulldown resistor and pull pin 6 high manually.
  The pushbutton provides a way to manually invoke the bootstrapper, which otherwise would never happen with this option.
Option 2:
  Connect DTR on the M100 through a MAX3232 to GPIO pin 6 on the arduino.
  Connect a 10-30k pull-down resitor from pin 6 to ground.
  Connect GPIO pin 5 on the arduino though a MAX-3232, to DSR on the Model 100.
  See the schematic for MonT Feather https://github.com/bkw777/MonT for reference.

### Software
* Load the source file into the Arduino IDE
* Download the SPI and SdFat libraries from the library manager
* Make any changes if needed
  There are many configuration options in the form of #defines at the top of the file.  
  The main one you need to set is BOARD to select what type of board to build for. Many other things automatically derive from that, for the few built-in supported boards like teensy and adalogger.
* Compile the code and upload it to the microcontroller
  You will need to consult your board's documentation to set up the Arduino IDE correctly to to program the board.  
  This usually means installing one or more board support libraries, and selecting the board type from the tools menu.  
  In the case of Teensy, you also should install "Teensyduino", and there are more options on the tools menu such as setting the cpu clock speed. You can underclock the teensy to save even more battery.

## Power from BCR port  
<!-- ![](https://github.com/bkw777/BCR_USB_PWR/blob/master/BCR_USB_PWR.png)  -->
You can power the Arduino from the computer with this [BCR-USB-Power adapter](https://github.com/bkw777/BCR_Breakout)  
 and a usb cable.
 
## Usage
### TPDD2-style bootstrap
PDDuino looks at the state of GPIO pin 6 at power-on (actually, right after the SD card is successfully initialized, so you can delay this check by powering-on with no SD card inserted, and then the check will happen right after you insert a card).  
If GPIO pin 6 is high when the SD card is first initialized, then PDDuino sends reads LOADER.DO and sends it over the serial line into BASIC.  
You need to use DSR/DTR Option 2 above, or you need to have the pushbutton with Option 1.

* Take an ascii format BASIC file, name it LOADER.DO, and place it in the root of the SD card.  
For example, any of the BASIC files in the "clients" dir from [dlplus](http://github.com/bkw777/dlplus).  
IE, take [TEENY.100](https://raw.githubusercontent.com/bkw777/dlplus/master/clients/teeny/TEENY.100) ,
Save it as LOADER.DO in the root of the SD card.

* Either power-off the arduino and insert the sd-card and leave the power off, or power-cycle the arduino now with no sd-card inserted, and leave the card ejected.

* Start BASIC on the Model 100 adn enter RUN "COM:98N1ENN" [Enter] just like the bootstrap directions for the TPDD2 or dlplus or mComm.

* If you did wiring option 1, press & hold the pushbutton.

* Power-on the arduino, or, insert the sd-card if it's already powered on.

* Once the LED comes on you can release the pushbutton.

* Follow any on-screen directions, and follow the post-install.txt file from dlplus for the loader you're using.  
IE: https://raw.githubusercontent.com/bkw777/dlplus/master/clients/teeny/TEENY.100.post-install.txt for TEENY.100

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
* Moved PCB to it's own repo
* Added TPDD2-style bootstrapper

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
