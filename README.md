# SD2TPDD
A hardware emulator of the Tandy Portable Disk Drive using an SD card for mass storage
## Verbose Description
The SD2TPDD is a project that aims to provide an easy-to-use, cheap, and reliable mass storage solution for the TRS-80 Model 100 series of computers. 

At the moment, SD2TPDD can:
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
* SPI SD card reader with logic level shifting  
  Example boards with sd card reader already built-in:  
  [Adafruit Feather 32u4 Adalogger](https://learn.adafruit.com/adafruit-feather-32u4-adalogger)  
  [Adafruit Feather M0 Adalogger](https://learn.adafruit.com/adafruit-feather-m0-adalogger)  
  [Teensy 3.5](https://www.pjrc.com/store/teensy35.html)  
  [Teensy 3.6](https://www.pjrc.com/store/teensy36.html)  
* RS232 level shifter for the TPDD port going to the computer (MAX232 or MAX3232 prefered!)

### Software
* Arduino IDE
* SPI library
* Bill Greiman's SdFat library
* For Teensy: Teensyduino
* For Adafruit M0: Arduino SAMD boards support, Adafruit SAMD boards support

## Assembly
### Hardware
* Attach the SPI SD card reader to the microcontroller using its SPI bus. Connect the SD card reader's chip select pin to the pin specified by the chipSelect variable (default is pin 4).
* Attach the SPI SD card reader to the microcontroller's power rail.
* Attach the RS232 level shifter to the TX/RX pins of hardware serial port 1 (hardware serial port 0 is the built-in port used for debugging)
* Attach the RS232 level shifter to the microcontroller's power rail.
* Bridge the DTR and DSR pins on the RS232 connector (Required for TS-DOS)
### Software
* Load the source file into the Arduino IDE
* Download the SPI and SdFat libraries from the library manager
* Make any changes if needed
* Compile the code and upload it to the microcontroller

## Notes
If you plan on using TS-DOS, some roms (like UR2) expect TS-DOS to be in a file named DOS100.CO on the root of the media.   This file can be downloaded from here: http://www.club100.org/nads/dos100.co

If you run into any issues, please let me know!

## To-Do
* Document the various Arduino IDE setup and config quirks needed for each board.
* (Done!) Move from SD.h to SDfat library for SD card access
* (Done!) Sub-directory support
* A protocol expansion allowing access to files greater than 64KB in size
* Full NADSBox compatibility
* A command-line that can be accessed from the computer's terminal emulator for quicker file manipulation
* Hayes modem emulation using an ESP8266
* FTP server/client access using an ESP8266

## other To-Dos
* RTC  (Teensy has built-in rtc)
* play & record audio as virtual cassette  (Teensy has built-in audio, and enough cpu & ram to use it)
* Battery level (Adalogger has built-in voltage reference and adc, and a built-in lipo charger)

## Change-log
### 20191025 b.kenyon.w@gmail.com
* Support Adafruit Feather M0 Adalogger  
 sleep() not implemented yet  
 needs "compiler.cpp.extra_flags=-fpermissive" in ~/.arduino15/packages/adafruit/hardware/samd/1.5.4/platform.local.txt
* Combine all boards support in the same code

### 20180921 b.kenyon.w@gmail.com
* Support Teensy 3.5/3.6
* Support Adafruit Feather 32u4 Adalogger
* Macro-ify all serial port access, debug and tpdd client
* sleepNow() powersaving, idles at 3ma
* dmeLabel[] & setLabel() TS-DOS shows current working dir in top-right corner
* disk-activity led

### V0.2 (7/27/2018)
* Added DME support
* Corrected some file name padding bugs

### V0.1 (7/21/2018)
* Initial testing release with basic TPDD1 emulation
