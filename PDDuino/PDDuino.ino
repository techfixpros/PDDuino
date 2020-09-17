/*
 * PDDuino - Tandy Portable Disk Drive emulator on Arduino
 * github.com/bkw777/PDDuino
 * Based on github.com/TangentDelta/SD2TPDD
 */

#define SKETCH_NAME __FILE__
#define SKETCH_VERSION "0.4.1"
// 0.4.1 20200913 bkw - more IDE-supplied macros to un-hardcode where possible, more debug output
// 0.4.0 20200828 bkw - sendLoader()
// 0.3.0 20180921 bkw - Teensy & Feather boards, CLIENT & CONSOLE, setLabel(), sleepNow()
// 0.2   20180728 Jimmy Pettit original

// Debugging settings
// Console is 115200 no flow
// DEBUG: 0 or undef = disable console, 1 = enabled, least verbose, 2+ = enabled, more verbose (max 3)
// When disabled, the port itself is disabled, which frees up significant ram and cpu on some hardware.
// On Teensy 3.5/3.6, the cpu must run at 24 MHz or more to enable the usb serial console.
// The sketch can run at the lowest setting, only 2 MHz without the usb serial console. (Tools -> CPU Speed).
// DEBUG will also be disabled if board_*.h doesn't define CONSOLE.
//#define DEBUG 1     // disable unless actually debugging, uses more battery, prevents full sleep, hangs at boot until connected
//#define DEBUG_LED   // use led to see (some) activity even if no DEBUG or CONSOLE (waiting for console, sleeping)
//#define DEBUG_SLEEP // use DEBUG_LED to debug sleepNow()

// File that sendLoader() will try to send to the client.
// Comment out to disable sendLoader() & ignore DSR_PIN
#define LOADER_FILE "LOADER.DO"

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

// Detect board/platform from IDE-supplied macros
//
// How to find these values.
// There is no list, and no consistent rule, every manufacturer & board makes up something.
// But in general, find the platform.txt and boards.txt for your board in the locations below (on linux)
// ~/arduino-*/hardware/*/*/boards.txt               Arduino and Teensy
// ~/arduino-*/hardware/*/*/platform.txt
// ~/.arduino*/packages/*/hardware/*/*/boards.txt    Most others, ex: Adafruit, Heltec, etc
// ~/.arduino*/packages/*/hardware/*/*/platform.txt
//
// platform.txt has gcc commandlines with "-D..." defines like "-DARDUINO_{something}"
// boards.txt has "something=AVR_UNO"
// so here we would look for "#if defined(ARDUINO_AVR_UNO)"

#if defined(ARDUINO_AVR_FEATHER32U4)
  #include "board_Feather_32u4.h"
#elif defined(ADAFRUIT_FEATHER_M0)
  #include "board_Feather_M0.h"
#elif defined(ARDUINO_TEENSY35) || defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)
  #include "board_Teensy_35_36_41.h"
#endif

#include "board_Defaults.h"
#include "cfg_behavior.h"

#if !defined(USE_SDIO)
#include <SPI.h>
#endif
#include <SdFat.h>
#include <sdios.h>
#include <SdFatConfig.h>
#include <SysCall.h>

#if defined(USE_SDIO)
SdFatSdioEX SD;
#else
SdFat SD;
#endif

#if defined(ENABLE_SLEEP)
 #if defined(USE_ALP)
  #include <ArduinoLowPower.h>
 #else
  #include <avr/sleep.h>
 #endif // USE_ALP
#endif // ENABLE_SLEEP

#define DATA_BUFFER_SZ 0x0100  // 256 bytes, 2 full tpdd packets? 0xFF did not work.
#define FILE_BUFFER_SZ 0x80    // 128 bytes at at time to/from files
#define DIRECTORY_SZ 0x40      // size of directory[] which holds full paths
#define FILENAME_SZ 0x18       // TPDD protocol spec 1C, minus 4 for ".<>"+NULL

File root;  //Root file for filesystem reference
File entry; //Moving file entry for the emulator
File tempEntry; //Temporary entry for moving files

byte head = 0x00;  //Head index
byte tail = 0x00;  //Tail index

byte checksum = 0x00;  //Global variable for checksum calculation

byte state = 0x00; //Emulator command reading state
bool DME = false; //TS-DOS DME mode flag

byte dataBuffer[DATA_BUFFER_SZ]; //Data buffer for commands
byte fileBuffer[FILE_BUFFER_SZ]; //Data buffer for file reading

char refFileName[FILENAME_SZ] = "";  //Reference file name for emulator
char refFileNameNoDir[FILENAME_SZ] = ""; //Reference file name for emulator with no ".<>" if directory
char tempRefFileName[FILENAME_SZ] = ""; //Second reference file name for renaming
char entryName[FILENAME_SZ] = "";  //Entry name for emulator
byte directoryBlock = 0x00; //Current directory block for directory listing
char directory[DIRECTORY_SZ] = "/";
byte directoryDepth = 0x00;
char tempDirectory[DIRECTORY_SZ] = "/";
char dmeLabel[0x07] = "";  // 6 chars + NULL

#if defined(ENABLE_SLEEP)
 #if defined(SLEEP_DELAY)
  unsigned long now = millis();
  unsigned long idleSince = now;
 #endif // SLEEP_DELAY
 #if !defined(USE_ALP)
  const byte rxInterrupt = digitalPinToInterrupt(CLIENT_RX_PIN);
 #endif // !USE_ALP
#endif // ENABLE_SLEEP

#if defined(SD_CD_PIN)
  const byte cdInterrupt = digitalPinToInterrupt(SD_CD_PIN);
#endif // SD_CD_PIN

//void setup() {}
//void loop() {}
