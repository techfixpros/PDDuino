# SD2TPDD
A hardware emulator of the Tandy Portable Disk Drive using an SD card for mass storage

https://www.youtube.com/embed/_lFqsHAlLyg

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
* SD card reader  
  Example boards with sd card reader already built-in:  
  [Adafruit Feather 32u4 Adalogger](https://learn.adafruit.com/adafruit-feather-32u4-adalogger)  
  [Adafruit Feather M0 Adalogger](https://learn.adafruit.com/adafruit-feather-m0-adalogger)  
  [Teensy 3.5](https://www.pjrc.com/store/teensy35.html)  
  [Teensy 3.6](https://www.pjrc.com/store/teensy36.html)  
* RS232 level shifter for the serial port going to the TPDD client (to the M100)

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
* Compile the code and upload it to the microcontroller

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
* Hayes modem emulation using an ESP8266
* FTP server/client access using an ESP8266

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

### V0.2 (7/27/2018)
* Added DME support
* Corrected some file name padding bugs

### V0.1 (7/21/2018)
* Initial testing release with basic TPDD1 emulation
