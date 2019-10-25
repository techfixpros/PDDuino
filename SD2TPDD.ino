/*
 * SD2TPDD  V0.2
 * A TPDD emulator for the Arduino Mega that uses an SD card for mass storage.
 * Written by Jimmy Pettit
 * 07/27/2018

 * 20180921 Brian K. White bw.aljex@gmail.com
 * macroified all the console and client serial port access
 * #ifdefs to enable/disable a lot of stuff, added sleep_mode(), added disk activity led,
 * #defines for the main configurable constants, use byte instead of int or larger where possible
 * converted most number literals to hex just for cosistency and because we are using byte in place of int so much
 * added dmeLabel[] and setLabel()
 */

#define Generic         0      // common settings that may work on most standard hardware
#define Custom          1      // edit the "PLATFORM == Custom" block below to suit your hardware
#define Teensy_3_5      2
#define Teensy_3_6      3
#define Adalogger_32u4  4
#define Adalogger_M0    5

#define PLATFORM Adalogger_M0

// log activity to serial monitor port
// Console is set for 115200 no flow
// 0 = serial console disabled, 1 = enabled, least verbose, 2+ = enabled, more verbose
// When disabled, the port itself is disabled, which frees up significant ram and cpu cycles on some hardware.
// Example, on Teensy3.5, with the port enabled at all, the sketch must be compiled to run the cpu at a minimum of 24mhz
// but with the port disabled, the sketch can be compiled to run at the lowest possible option of only 2Mhz,
// and that is still plenty enough to do the TPDD job, while burning the least amount of battery.
#define DEBUG 3

// Where to send debug messages, if enabled.
// Typically "Serial" for the built-in usb-serial port.
// This can easily be changed to an oled or other device display,
// just load the display driver and put the port name here.
#define CONSOLE Serial

// Which serial port is the TPDD connected to?
// Preferrably a real uart port, and with a hardware interrupt on the RX pin.
// Typically "Serial1"
#define CLIENT Serial1

//--- configs for some handy boards that are small and have sd readers built in ---

#if PLATFORM == Custom
  // User-defined platform details
  #define SD_CS_PIN 4                       // sd card reader chip-select pin #
  #define DISABLE_SD_CS 0               // disable CS on SD_CHIP_SELECT pin
  #define USE_SDIO 0                    // sd card reader communication method false = SPI (most boards), true = SDIO (Teensy 3.5/3.6)
  #define ENABLE_SLEEP 1                 // sleep() while idle for power saving
  #define CLIENT Serial1                    // what serial port is the TPDD connected to
  #define WAKE_PIN 0                        // Use pin# of RX of CLIENT serial port
  #define SLEEP_INHIBIT 5000                // Idle grace period before sleeping, 0 = disable wait (always sleep immediately), 300,000 = wait idle for 5 minutes before sleeping
  #define DISK_ACTIVITY_LIGHT 1
  #define PINMODE_SD_LED_OUTPUT pinMode(13,OUTPUT);
  #define SD_LED_ON digitalWrite(13,HIGH);
  #define SD_LED_OFF digitalWrite(13,LOW);

// Generic/Typical Arduino platform
#elif PLATFORM == Generic
  #define SD_CS_PIN 4
  #define DISABLE_SD_CS 0
  #define USE_SDIO 0
  #define ENABLE_SLEEP 1
  #define WAKE_PIN 0
  #define SLEEP_INHIBIT 5000
  #define DISK_ACTIVITY_LIGHT 1
  #define PINMODE_SD_LED_OUTPUT pinMode(13,OUTPUT);
  #define SD_LED_ON digitalWrite(13,HIGH);
  #define SD_LED_OFF digitalWrite(13,LOW);

// Teensy3.5, Teensy3.6
// https://www.pjrc.com/store/teensy35.html
// https://www.pjrc.com/store/teensy36.html
#elif PLATFORM == Teensy_3_5 || PLATFORM == Teensy_3_6
  #define SD_CS_PIN -1
  #define DISABLE_SD_CS 0
  #define USE_SDIO 1
  #define ENABLE_SLEEP 1
  #define WAKE_PIN 0
  #define SLEEP_INHIBIT 0       // Teensy3.5/3.6 can sleep instantly
  #define DISK_ACTIVITY_LIGHT 1
  // Main LED: PB5
  #define PINMODE_SD_LED_OUTPUT DDRB = DDRB |= 1UL << 5;
  #define SD_LED_ON PORTB |= _BV(5);
  #define SD_LED_OFF PORTB &= ~_BV(5);
  // There is only one led on the Teensy, so you probably don't want to use it for both disk activity and sleep debugging at once.
  // Set DISK_ACTIVITY_LIGHT false or install another led on any available pin.
  // Main LED: PB5
  #define PINMODE_DEBUG_LED_OUTPUT DDRB = DDRB |= 1UL << 5;
  #define DEBUG_LED_ON PORTB |= _BV(5);
  #define DEBUG_LED_OFF PORTB &= ~_BV(5);

// Adafruit Adalogger Feather 32u4
// https://learn.adafruit.com/adafruit-feather-32u4-adalogger
#elif PLATFORM == Adalogger_32u4
  #define SD_CS_PIN 4
  #define DISABLE_SD_CS 0
  #define USE_SDIO 0
  #define ENABLE_SLEEP 0
  #define WAKE_PIN 0
  #define SLEEP_INHIBIT 5000          // Adalogger 32u4 needs a few seconds before sleeping
  #define DISK_ACTIVITY_LIGHT 1
  // Green LED near card reader: PB4
  #define PINMODE_SD_LED_OUTPUT DDRB = DDRB |= 1UL << 4;
  #define SD_LED_ON PORTB |= _BV(4);
  #define SD_LED_OFF PORTB &= ~_BV(4);
  // Main LED: PC7
  #define PINMODE_DEBUG_LED_OUTPUT DDRC = DDRC |= 1UL << 7;
  #define DEBUG_LED_ON PORTC |= _BV(7);
  #define DEBUG_LED_OFF PORTC &= ~_BV(7);

// Adafruit Adalogger Feather M0
// https://learn.adafruit.com/adafruit-feather-m0-adalogger
#elif PLATFORM == Adalogger_M0
  #define SD_CS_PIN
  #define DISABLE_SD_CS 0
  #define USE_SDIO 0
  #define ENABLE_SLEEP 0
  #define WAKE_PIN 0
  #define SLEEP_INHIBIT 5000
  #define DISK_ACTIVITY_LIGHT 1
  // Green LED near card reader: PA6
  #define PINMODE_SD_LED_OUTPUT DDRA = DDRA |= 1UL << 6;
  #define SD_LED_ON PORTA |= _BV(6);
  #define SD_LED_OFF PORTA &= ~_BV(6);
  // Main LED: PA17
  #define PINMODE_DEBUG_LED_OUTPUT DDRA = DDRA |= 1UL << 17;
  #define DEBUG_LED_ON PORTA |= _BV(17);
  #define DEBUG_LED_OFF PORTA &= ~_BV(17);

#endif // PLATFORM

// Use an LED to debug sleepNow() without using Serial within an ISR.
// Some LED port/bit addrs: Adalogger_32u4:PC7, Adalogger_M0:PA17, Teensy3.x:PB5
// Normally want the opposite, turn things off during sleep, not on!

#if 0
  #if PLATFORM == Adalogger_32u4 // Main LED: PC7
    #define PINMODE_DEBUG_LED_OUTPUT DDRC = DDRC |= 1UL << 7;
    #define DEBUG_LED_ON PORTC |= _BV(7);
    #define DEBUG_LED_OFF PORTC &= ~_BV(7);
  #elif PLATFORM == Adalogger_M0 // Main LED: PA17
    #define PINMODE_DEBUG_LED_OUTPUT DDRA = DDRA |= 1UL << 17;
    #define DEBUG_LED_ON PORTA |= _BV(17);
    #define DEBUG_LED_OFF PORTA &= ~_BV(17);
  #elif PLATFORM == Teensy_3.5 || Teensy_3.6 // Main LED: PB5
    #define PINMODE_DEBUG_LED_OUTPUT DDRB = DDRB |= 1UL << 5;
    #define DEBUG_LED_ON PORTB |= _BV(5);
    #define DEBUG_LED_OFF PORTB &= ~_BV(5);
  #else // use pin 13 the generic Arduino way
    #define PINMODE_DEBUG_LED_OUTPUT pinMode(13,OUTPUT);
    #define DEBUG_LED_ON digitalWrite(13,HIGH);
    #define DEBUG_LED_OFF digitalWrite(13,LOW);
  #endif // PLATFORM
#else // disabled
#define PINMODE_DEBUG_LED_OUTPUT
#define DEBUG_LED_ON
#define DEBUG_LED_OFF
#endif

//
// end of config section
//
//////////////////////////////////////////////////////////////////////////////////////////////////

#define DATA_BUFFER_SZ 0x0100  // 256 bytes, 2 full tpdd packets? 0xFF did not work.
#define FILE_BUFFER_SZ 0x80    // 128 bytes at at time to/from files
#define DIRECTORY_SZ 0x40      // size of directory[] which holds full paths
#define FILENAME_SZ 0x18       // TPDD protocol spec 1C, minus 4 for ".<>" null

//-----------------------------------------------------------------------------

#if DISK_ACTIVITY_LIGHT
  #define PINMODE_SD_LED_OUTPUT
  #define SD_LED_ON
  #define SD_LED_OFF
#endif

#if DEBUG && defined(CONSOLE)
 #define DEBUG_PRINT(x)     CONSOLE.print (x)
 #define DEBUG_PRINTI(x,y)  CONSOLE.print (x,y)
 #define DEBUG_PRINTL(x)    CONSOLE.println (x)
 #define DEBUG_PRINTIL(x,y) CONSOLE.println (x,y)
#else
 #define DEBUG_PRINT(x)
 #define DEBUG_PRINTI(x,y)
 #define DEBUG_PRINTL(x)
 #define DEBUG_PRINTIL(x,y)
#endif

#include <SdFat.h>
// sometime between 1.8.6 and 1.8.10 it became included?
//#include <avr/sleep.h>

SdFat SD; //SD card object

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
char dmeLabel[0x07] = "";  // 6 chars

#if WITH_SLEEP
const byte wakeInterrupt = digitalPinToInterrupt(WAKE_PIN);
  #if SLEEP_INHIBIT
unsigned long now = millis();
unsigned long idleSince = now;
  #endif // SLEEP_INHIBIT
#endif // WITH_SLEEP

void setup() {
  PINMODE_SD_LED_OUTPUT
  PINMODE_DEBUG_LED_OUTPUT
  //pinMode(WAKE_PIN, INPUT_PULLUP);  // typical but don't do on RX

#if DEBUG && defined(CONSOLE)
  CONSOLE.begin(115200);
    while(!CONSOLE){
      DEBUG_LED_ON
      delay(0x60);
      DEBUG_LED_OFF
      delay(0x60);
    }
  CONSOLE.flush();
#endif

  CLIENT.begin(19200);
  CLIENT.flush();

  DEBUG_PRINTL(F("\r\n-----------[ SD2TPDD setup() ]------------"));

  for(byte i=0x00;i<FILE_BUFFER_SZ;++i) dataBuffer[i] = 0x00;

#if SD_CHIP_SELECT >= 0 && !USE_SDIO
  #if DISABLE_CHIP_SELECT
    //DEBUG_PRINT(F("Disabling SPI device on pin "));
    //DEBUG_PRINTL(SD_CHIP_SELECT);
    pinMode(SD_CHIP_SELECT, OUTPUT);
    digitalWrite(SD_CHIP_SELECT, HIGH);
  #else
    //DEBUG_PRINTL(F("Assuming the SD is the only SPI device."));  
  #endif
  //DEBUG_PRINT(F("Using SD chip select pin: "));
  //DEBUG_PRINTL(SD_CHIP_SELECT);
#endif  // !USE_SDIO

  initCard();
}

/*
 *
 * General misc. routines
 *
 */

#if WITH_SLEEP
void wakeNow () {
}

void sleepNow() {
#if SLEEP_INHIBIT
  now = millis();
  if ((now-idleSince)<SLEEP_INHIBIT) return;
  idleSince = now;
#endif
// prevent the built-in usb-serial device from powering-off if we're using it
#if DEBUG && defined(CONSOLE)
  set_sleep_mode(SLEEP_MODE_IDLE);
#else
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
#endif
  DEBUG_LED_ON
  attachInterrupt(wakeInterrupt,wakeNow,CHANGE);
  sleep_mode();
  detachInterrupt(wakeInterrupt);
  DEBUG_LED_OFF
}
#endif // WITH_SLEEP

void initCard () {
  while(true){
    DEBUG_PRINT(F("Opening SD card..."));
    SD_LED_ON
#if USE_SDIO
    if (SD.begin()) {
#else  /// USE_SDIO
    //if (SD.begin(SD_CS_PIN,SD_SCK_MHZ(50))) {
    if (SD.begin(SD_CS_PIN)) {
#endif  // USE_SDIO
      DEBUG_PRINTL(F("OK."));
#if DISK_ACTIVITY_LIGHT
      SD_LED_OFF
      delay (0x60);
      SD_LED_ON
      delay (0x60);
      SD_LED_OFF
      delay (0x60);
      SD_LED_ON
      delay (0x60);
#endif
      break;
    } else {
      DEBUG_PRINTL(F("No SD card."));
#if DISK_ACTIVITY_LIGHT
      SD_LED_OFF
      delay(1000);
#endif
    }
  }

  SD.chvol();

  // TODO - get the FAT volume label and use it inplace of rootLabel

  // Always do this open() & close(), even if we aren't doing the printDirectory()
  // It's needed to get the SdFat library to put the sd card to sleep.
  root = SD.open(directory);
#if DEBUG
  DEBUG_PRINTL(F("--- printDirectory(root,0) start ---"));
  printDirectory(root,0x00);
  DEBUG_PRINTL(F("--- printDirectory(root,0) end ---"));
#endif
  root.close();

  SD_LED_OFF
}

#if DEBUG
void printDirectory(File dir, byte numTabs) {
  char fileName[FILENAME_SZ] = "";

  SD_LED_ON
  while (true) {
    File entry = dir.openNextFile();
    if (!entry) break;
    entry.getName(fileName,FILENAME_SZ);
    for (byte i = 0x00; i < numTabs; i++) DEBUG_PRINT(F("\t"));
    DEBUG_PRINT(fileName);
    if (entry.isDirectory()) {
      DEBUG_PRINTL(F("/"));
      DEBUG_PRINT(F("--- printDirectory(")); DEBUG_PRINT(fileName); DEBUG_PRINT(F(",")); DEBUG_PRINT(numTabs+0x01); DEBUG_PRINTL(F(") start ---"));
      printDirectory(entry, numTabs + 0x01);
      DEBUG_PRINT(F("--- printDirectory(")); DEBUG_PRINT(fileName); DEBUG_PRINT(F(",")); DEBUG_PRINT(numTabs+0x01); DEBUG_PRINTL(F(") end ---"));
    } else {
      DEBUG_PRINT(F("\t\t")); DEBUG_PRINTIL(entry.fileSize(), DEC);
    }
    entry.close();
  }
  SD_LED_OFF
}
#endif // DEBUG

// Append a string to directory[]
void directoryAppend(char* c){
  bool t = false;
  byte i = 0x00;
  byte j = 0x00;

  DEBUG_PRINT(F("directoryAppend(")); DEBUG_PRINT(c); DEBUG_PRINTL(F(")"));
  DEBUG_PRINT(F("directory[")); DEBUG_PRINT(directory); DEBUG_PRINTL(F("]"));

  DEBUG_PRINTL(F("->"));

  while(directory[i] != 0x00) i++;

  while(!t){
    directory[i++] = c[j++];
    t = c[j] == 0x00;
  }

  DEBUG_PRINT(F("directory[")); DEBUG_PRINT(directory); DEBUG_PRINTL(F("]"));
  DEBUG_PRINT(F("directoryAppend(")); DEBUG_PRINT(c); DEBUG_PRINTL(F(") end"));
}

// Remove the last path element from directoy[]
void upDirectory(){
  byte j = DIRECTORY_SZ;

  DEBUG_PRINTL(F("upDirectory()"));
  DEBUG_PRINT(F("directory[")); DEBUG_PRINT(directory); DEBUG_PRINTL(F("]"));

  while(directory[j] == 0x00) j--;
  if(directory[j] == '/' && j!= 0x00) directory[j] = 0x00;
  while(directory[j] != '/') directory[j--] = 0x00;

  DEBUG_PRINT(F("directory[")); DEBUG_PRINT(directory); DEBUG_PRINTL(F("]"));
}

void copyDirectory(){ //Makes a copy of the working directory to a scratchpad
  for(byte i=0x00; i<DIRECTORY_SZ; i++) tempDirectory[i] = directory[i];
}


// Fill dmeLabel[] with exactly 6 chars from s[], space-padded.
// We could just read directory[] directly instead of passng s[]
// but this way we can pass arbitrary values later. For example
// FAT volume label, RTC time, battery level, ...
void setLabel(char* s) {
  byte z = DIRECTORY_SZ;
  byte j = z;

  DEBUG_PRINT(F("setLabel(")); DEBUG_PRINT(s); DEBUG_PRINTL(F(")"));
  DEBUG_PRINT(F("directory[")); DEBUG_PRINT(directory); DEBUG_PRINTL(F("]"));
  DEBUG_PRINT(F("dmeLabel["));  DEBUG_PRINT(dmeLabel);  DEBUG_PRINTL(F("]"));

  while(s[j] == 0x00) j--;            // seek from end to non-null
  if(s[j] == '/' && j > 0x00) j--;    // seek past trailing slash
  z = j;                              // mark end of name
  while(s[j] != '/' && j > 0x00) j--; // seek to next slash

  // copy 6 chars, up to z or null, space pad
  for(byte i=0x00 ; i<0x06 ; i++) if(s[++j]>0x00 && j<=z) dmeLabel[i] = s[j]; else dmeLabel[i] = 0x20;
  dmeLabel[0x06] = 0x00;

  DEBUG_PRINT(F("dmeLabel[")); DEBUG_PRINT(dmeLabel); DEBUG_PRINTL(F("]"));
}



/*
 *
 * TPDD Port misc. routines
 *
 */

void tpddWrite(const char* c){  //Outputs char c to TPDD port and adds to the checksum
  checksum += c;
  CLIENT.write(c);
}

void tpddWriteString(const char* c){  //Outputs a null-terminated char array c to the TPDD port
  byte i = 0x00;
  while(c[i]!=0x00){
    checksum += c[i];
    CLIENT.write(c[i]);
    i++;
  }
}

void tpddSendChecksum(){  //Outputs the checksum to the TPDD port and clears the checksum
  CLIENT.write(checksum^0xFF);
  checksum = 0x00;
}


/*
 *
 * TPDD Port return routines
 *
 */

void return_normal(byte errorCode){ //Sends a normal return to the TPDD port with error code errorCode
  DEBUG_PRINTL(F("return_normal()"));
#if (DEBUG > 1)
  DEBUG_PRINT("R:Norm ");
  DEBUG_PRINTIL(errorCode, HEX);
#endif
  tpddWrite(0x12);  //Return type (normal)
  tpddWrite(0x01);  //Data size (1)
  tpddWrite(errorCode); //Error code
  tpddSendChecksum(); //Checksum
}

void return_reference(){  //Sends a reference return to the TPDD port
  byte term = 0x06;
  bool terminated = false;
  DEBUG_PRINTL(F("return_reference()"));

  tpddWrite(0x11);  //Return type (reference)
  tpddWrite(0x1C);  //Data size (1C)

  for(byte i=0x00;i<FILENAME_SZ;++i) tempRefFileName[i] = 0x00;

  entry.getName(tempRefFileName,FILENAME_SZ);  //Save the current file entry's name to the reference file name buffer

  if(DME && entry.isDirectory()){ //      !!!Tacks ".<>" on the end of the return reference if we're in DME mode and the reference points to a directory
    for(byte i=0x00; i < 0x07; i++){ //Find the end of the directory's name by looping through the name buffer
      if(tempRefFileName[i] == 0x00) term = i; //and setting a termination index to the offset where the termination is encountered
    }
    tempRefFileName[term++] = '.';  //Tack the expected ".<>" to the end of the name
    tempRefFileName[term++] = '<';
    tempRefFileName[term++] = '>';
    for(byte i=term; i<FILENAME_SZ; i++) tempRefFileName[i] = 0x00;
    term = 0x06; //Reset the termination index to prepare for the next check
  }



  for(byte i=0x00; i<0x06; i++){ //      !!!Pads the name of the file out to 6 characters using space characters
    if(term == 0x06){  //Perform these checks if term hasn't changed
      if(tempRefFileName[i]=='.'){
        term = i;   //If we encounter a '.' character, set the temrination pointer to the current offset and output a space character instead
        tpddWrite(' ');
      }else{
        tpddWrite(tempRefFileName[i]);  //If we haven't encountered a period character, output the next character
      }
    }else{
      tpddWrite(' '); //If we did find a period character, write a space character to pad the reference name
    }
  }

  for(byte i=0x00; i<0x12; i++) tpddWrite(tempRefFileName[i+term]);  //      !!!Outputs the file extension part of the reference name starting at the offset found above

  tpddWrite(0x00);  //Attribute, unused
  tpddWrite((byte)((entry.fileSize()&0xFF00)>>0x08));  //File size most significant byte
  tpddWrite((byte)(entry.fileSize()&0xFF)); //File size least significant byte
  tpddWrite(0x80);  //Free sectors, SD card has more than we'll ever care about
  tpddSendChecksum(); //Checksum
#if (DEBUG > 1)
  DEBUG_PRINTL("R:Ref");
#endif
}

void return_blank_reference(){  //Sends a blank reference return to the TPDD port
  DEBUG_PRINTL(F("return_blank_reference()"));
  tpddWrite(0x11);    //Return type (reference)
  tpddWrite(0x1C);    //Data size (1C)

  for(byte i=0x00; i<FILENAME_SZ; i++) tpddWrite(0x00);  //Write the reference file name to the TPDD port

  tpddWrite(0x00);    //Attribute, unused
  tpddWrite(0x00);    //File size most significant byte
  tpddWrite(0x00);    //File size least significant byte
  tpddWrite(0x80);    //Free sectors, SD card has more than we'll ever care about
  tpddSendChecksum(); //Checksum
#if (DEBUG > 1)
  DEBUG_PRINTL("R:BRef");
#endif
}

void return_parent_reference(){
  DEBUG_PRINTL(F("return_parent_reference()"));
  tpddWrite(0x11);    // return type
  tpddWrite(0x1C);    // data size

  tpddWriteString("PARENT.<>");
  for(byte i=0x09; i<FILENAME_SZ; i++) tpddWrite(0x00);

  tpddWrite(0x00);    //Attribute, unused
  tpddWrite(0x00);    //File size most significant byte
  tpddWrite(0x00);    //File size least significant byte
  tpddWrite(0x80);    //Free sectors, SD card has more than we'll ever care about
  tpddSendChecksum(); //Checksum
}

/*
 *
 * TPDD Port command handler routines
 *
 */

void command_reference(){ //Reference command handler
  byte searchForm = dataBuffer[(byte)(tail+0x1D)];  //The search form byte exists 29 bytes into the command
  byte refIndex = 0x00;  //Reference file name index

  DEBUG_PRINTL(F("command_reference()"));

#if (DEBUG > 1)
  DEBUG_PRINT("SF:");
  DEBUG_PRINTIL(searchForm,HEX);
#endif

  if(searchForm == 0x00){ //Request entry by name
    for(byte i=0x04; i<0x1C; i++){  //Put the reference file name into a buffer
      if(dataBuffer[(tail+i)]!=0x20){ //If the char pulled from the command is not a space character (0x20)...
        refFileName[refIndex++]=dataBuffer[(tail+i)]; //write it into the buffer and increment the index.
      }
    }
    refFileName[refIndex]=0x00; //Terminate the file name buffer with a null character

#if (DEBUG > 1)
    DEBUG_PRINT("Ref: ");
    DEBUG_PRINTL(refFileName);
#endif

    if(DME){  //        !!!Strips the ".<>" off of the reference name if we're in DME mode
      if(strstr(refFileName, ".<>") != 0x00){
        for(byte i=0x00; i<FILENAME_SZ; i++){  //Copies the reference file name to a scratchpad buffer with no directory extension if the reference is for a directory
          if(refFileName[i] != '.' && refFileName[i] != '<' && refFileName[i] != '>'){
            refFileNameNoDir[i]=refFileName[i];
          }else{
            refFileNameNoDir[i]=0x00; //If the character is part of a directory extension, don't copy it
          }
        }
      }else{
        for(byte i=0x00; i<FILENAME_SZ; i++) refFileNameNoDir[i]=refFileName[i]; //Copy the reference directly to the scratchpad buffer if it's not a directory reference
      }
    }

    directoryAppend(refFileNameNoDir);  //Add the reference to the directory buffer

    SD_LED_ON
    if(SD.exists(directory)){ //If the file or directory exists on the SD card...
      entry=SD.open(directory); //...open it...
      return_reference(); //send a refernce return to the TPDD port with its info...
      entry.close();  //...close the entry
    }else{  //If the file does not exist...
      return_blank_reference();
    }

    upDirectory();  //Strip the reference off of the directory buffer

  }else if(searchForm == 0x01){ //Request first directory block
    SD_LED_ON
    root.close();
    root = SD.open(directory);
    ref_openFirst();
  }else if(searchForm == 0x02){ //Request next directory block
    SD_LED_ON
    root.close();
    root = SD.open(directory);
    ref_openNext();
  }else{  //Parameter is invalid
    return_normal(0x36);  //Send a normal return to the TPDD port with a parameter error
  }
  SD_LED_OFF
}

void ref_openFirst(){
  DEBUG_PRINTL(F("ref_openFirst()"));
  directoryBlock = 0x00; //Set the current directory entry index to 0
  if(DME && directoryDepth>0x00 && directoryBlock==0x00){ //Return the "PARENT.<>" reference if we're in DME mode
    SD_LED_OFF
    return_parent_reference();
  }else{
    ref_openNext();    //otherwise we just return the next reference
  }
}

void ref_openNext(){
  DEBUG_PRINTL(F("ref_openNext()"));
  directoryBlock++; //Increment the directory entry index
  SD_LED_ON
  root.rewindDirectory(); //Pull back to the begining of the directory
  for(byte i=0x00; i<directoryBlock-0x01; i++) root.openNextFile();  //skip to the current entry offset by the index

  entry = root.openNextFile();  //Open the entry

  if(entry){  //If the entry exists it is returned
    if(entry.isDirectory() && !DME){  //If it's a directory and we're not in DME mode
      entry.close();  //the entry is skipped over
      ref_openNext(); //and this function is called again
    }

    return_reference(); //Send the reference info to the TPDD port
    entry.close();  //Close the entry
    SD_LED_OFF
  }else{
    SD_LED_OFF
    return_blank_reference();
  }
}

void command_open(){  //Opens an entry for reading, writing, or appending
  byte rMode = dataBuffer[(byte)(tail+0x04)];  //The access mode is stored in the 5th byte of the command
  DEBUG_PRINTL(F("command_open()"));
  entry.close();

  if(DME && strcmp(refFileNameNoDir, "PARENT") == 0x00){ //If DME mode is enabled and the reference is for the "PARENT" directory
    upDirectory();  //The top-most entry in the directory buffer is taken away
    directoryDepth--; //and the directory depth index is decremented
  }else{
    directoryAppend(refFileNameNoDir);  //Push the reference name onto the directory buffer
    SD_LED_ON
//    if(DME && (byte)strstr(refFileName, ".<>") != 0x00 && !SD.exists(directory)){ //If the reference is for a directory and the directory buffer points to a directory that does not exist
    if(DME && strstr(refFileName, ".<>") != 0x00 && !SD.exists(directory)){ //If the reference is for a directory and the directory buffer points to a directory that does not exist
      SD.mkdir(directory);  //create the directory
      upDirectory();
    }else{
      entry=SD.open(directory); //Open the directory to reference the entry
      if(entry.isDirectory()){  //      !!!Moves into a sub-directory
        entry.close();  //If the entry is a directory
        directoryAppend("/"); //append a slash to the directory buffer
        directoryDepth++; //and increment the directory depth index
      }else{  //If the reference isn't a sub-directory, it's a file
        entry.close();
        switch(rMode){
          case 0x01: entry = SD.open(directory, FILE_WRITE); break;             // Write
          case 0x02: entry = SD.open(directory, FILE_WRITE | O_APPEND); break;  // Append
          case 0x03: entry = SD.open(directory, FILE_READ); break;              // Read
        }
        upDirectory();
      }
    }
  }

  if(SD.exists(directory)){ //If the file actually exists...
    SD_LED_OFF
    return_normal(0x00);  //...send a normal return with no error.
  }else{  //If the file doesn't exist...
    SD_LED_OFF
    return_normal(0x10);  //...send a normal return with a "file does not exist" error.
  }
}

void command_close(){ //Closes the currently open entry
  DEBUG_PRINTL(F("command_close()"));
  entry.close();  //Close the entry
  SD_LED_OFF
  return_normal(0x00);  //Normal return with no error
}

void command_read(){  //Read a block of data from the currently open entry
  DEBUG_PRINTL(F("command_read()"));
  SD_LED_ON
  byte bytesRead = entry.read(fileBuffer, FILE_BUFFER_SZ); //Try to pull 128 bytes from the file into the buffer
  SD_LED_OFF
#if (DEBUG > 1)
  DEBUG_PRINT("A: ");
  DEBUG_PRINTIL(entry.available(),HEX);
#endif
  if(bytesRead > 0x00){  //Send the read return if there is data to be read
    tpddWrite(0x10);  //Return type
    tpddWrite(bytesRead); //Data length
    for(byte i=0x00; i<bytesRead; i++) tpddWrite(fileBuffer[i]);
    tpddSendChecksum();
  }else{
    return_normal(0x3F);  //send a normal return with an end-of-file error if there is no data left to read
  }
}

void command_write(){ //Write a block of data from the command to the currently open entry
  byte commandDataLength = dataBuffer[(byte)(tail+0x03)];

  DEBUG_PRINTL(F("command_write()"));
  SD_LED_ON
  for(byte i=0x00; i<commandDataLength; i++) entry.write(dataBuffer[(byte)(tail+0x04+i)]);
  SD_LED_OFF
  return_normal(0x00);  //Send a normal return to the TPDD port with no error
}

void command_delete(){  //Delete the currently open entry
  DEBUG_PRINTL(F("command_delete()"));
  SD_LED_ON
  entry.close();  //Close any open entries
  directoryAppend(refFileNameNoDir);  //Push the reference name onto the directory buffer
  entry = SD.open(directory, FILE_READ);  //directory can be deleted if opened "READ"

  if(DME && entry.isDirectory()){
    entry.rmdir();  //If we're in DME mode and the entry is a directory, delete it
  }else{
    entry.close();  //Files can be deleted if opened "WRITE", so it needs to be re-opened
    entry = SD.open(directory, FILE_WRITE);
    entry.remove();
  }
  SD_LED_OFF
  upDirectory();
  return_normal(0x00);  //Send a normal return with no error
}

void command_format(){  //Not implemented
  DEBUG_PRINTL(F("command_format()"));
  return_normal(0x00);
}

void command_status(){  //Drive status
  DEBUG_PRINTL(F("command_status()"));
  return_normal(0x00);
}

void command_condition(){ //Not implemented
  DEBUG_PRINTL(F("command_condition()"));
  return_normal(0x00);
}

void command_rename(){  //Renames the currently open entry
  byte refIndex = 0x00;  //Temporary index for the reference name

  DEBUG_PRINTL(F("command_rename()"));

  directoryAppend(refFileNameNoDir);  //Push the current reference name onto the directory buffer

  SD_LED_ON

  if(entry) entry.close(); //Close any currently open entries
  entry = SD.open(directory); //Open the entry
  if(entry.isDirectory()) directoryAppend("/"); //Append a slash to the end of the directory buffer if the reference is a sub-directory

  copyDirectory();  //Copy the directory buffer to the scratchpad directory buffer
  upDirectory();  //Strip the previous directory reference off of the directory buffer

  for(byte i=0x04; i<0x1C; i++){  //Loop through the command's data block, which contains the new entry name
      if(dataBuffer[(byte)(tail+i)]!=0x20 && dataBuffer[(byte)(tail+i)]!=0x00){ //If the current character is not a space (0x20) or null character...
        tempRefFileName[refIndex++]=dataBuffer[(byte)(tail+i)]; //...copy the character to the temporary reference name and increment the pointer.
      }
  }

  tempRefFileName[refIndex]=0x00; //Terminate the temporary reference name with a null character

  if(DME && entry.isDirectory()){ //      !!!If the entry is a directory, we need to strip the ".<>" off of the new directory name
    if(strstr(tempRefFileName, ".<>") != 0x00){
      for(byte i=0x00; i<FILENAME_SZ; i++){
        if(tempRefFileName[i] == '.' || tempRefFileName[i] == '<' || tempRefFileName[i] == '>'){
          tempRefFileName[i]=0x00;
        }
      }
    }
  }

  directoryAppend(tempRefFileName);
  if(entry.isDirectory()) directoryAppend("/");

  DEBUG_PRINTL(directory);
  DEBUG_PRINTL(tempDirectory);
  SD.rename(tempDirectory,directory);  //Rename the entry

  upDirectory();
  entry.close();

  SD_LED_OFF

  return_normal(0x00);  //Send a normal return to the TPDD port with no error
}

/*
 *
 * TS-DOS DME Commands
 *
 */

void command_DMEReq() {  //Send the dmeLabel

  DEBUG_PRINT(F("command_DMEReq(): dmeLabel[")); DEBUG_PRINT(dmeLabel); DEBUG_PRINTL(F("]"));

  if(DME){  // prepend "/" to the root dir label just because my janky-ass setLabel() assumes it
    if (directoryDepth>0x00) setLabel(directory); else setLabel("/SD:   ");
    tpddWrite(0x12);
    tpddWrite(0x0B);
    tpddWrite(0x20);
    for (byte i=0x00 ; i<0x06 ; i++) tpddWrite(dmeLabel[i]);
    tpddWrite(".");
    tpddWrite("<");
    tpddWrite(">");
    tpddWrite(0x20);
    tpddSendChecksum();
  }else{
    return_normal(0x36);
  }
}

/*
 *
 * Main code loop
 *
 */

void loop() {
  byte rType = 0x00; //Current request type (command type)
  byte rLength = 0x00; //Current request length (command length)
  byte diff = 0x00;  //Difference between the head and tail buffer indexes

  DEBUG_PRINTL(F("loop(): start"));
  state = 0x00; //0 = waiting for command 1 = waiting for full command 2 = have full command

#if ENABLE_SLEEP
  sleepNow();
#endif // ENABLE_SLEEP
  while(state<0x02){ //While waiting for a command...
#if ENABLE_SLEEP
    sleepNow();
#endif // ENABLE_SLEEP
    while (CLIENT.available() > 0x00){  //While there's data to read from the TPDD port...
#if ENABLE_SLEEP
    #if SLEEP_INHIBIT
      idleSince = millis();
    #endif
#endif // ENABLE_SLEEP
      dataBuffer[head++]=(byte)CLIENT.read();  //...pull the character from the TPDD port and put it into the command buffer, increment the head index...
      if(tail==head)tail++; //...if the tail index equals the head index (a wrap-around has occoured! data will be lost!)
                            //...increment the tail index to prevent the command size from overflowing.

#if (DEBUG > 1)
      DEBUG_PRINTI((byte)(head-1),HEX);
      DEBUG_PRINT("-");
      DEBUG_PRINTI(tail,HEX);
      DEBUG_PRINTI((byte)(head-tail),HEX);
      DEBUG_PRINT(":");
      DEBUG_PRINTI(dataBuffer[head-1],HEX);
      DEBUG_PRINT(";");
      DEBUG_PRINTL((dataBuffer[head-1]>=0x20)&&(dataBuffer[head-1]<=0x7E)?(char)dataBuffer[head-1]:' ');
#endif
    }

    diff=(byte)(head-tail); //...set the difference between the head and tail index (number of bytes in the buffer)

    if(state == 0x00){ //...if we're waiting for a command...
      if(diff >= 0x04){  //...if there are 4 or more characters in the buffer...
        if(dataBuffer[tail]=='Z' && dataBuffer[(byte)(tail+0x01)]=='Z'){ //...if the buffer's first two characters are 'Z' (a TPDD command)
          rLength = dataBuffer[(byte)(tail+0x03)]; //...get the command length...
          rType = dataBuffer[(byte)(tail+0x02)]; //...get the command type...
          state = 0x01;  //...set the state to "waiting for full command".
        }else if(dataBuffer[tail]=='M' && dataBuffer[(byte)(tail+0x01)]=='1'){ //If a DME command is received
          DME = true; //set the DME mode flag to true
          tail=tail+0x02;  //and skip past the command to the DME request command
        }else{  //...if the first two characters are not 'Z'...
          tail=tail+(tail==head?0x00:0x01); //...move the tail index forward to the next character, stop if we reach the head index to prevent an overflow.
        }
      }
    }

    if(state == 0x01){ //...if we're waiting for the full command to come in...
      if(diff>rLength+0x04){ //...if the amount of data in the buffer satisfies the command length...
          state = 0x02;  //..set the state to "have full command".
        }
    }
  }

#if (DEBUG > 1)
  DEBUG_PRINTI(tail,HEX); // show the tail index in the buffer where the command was found...
  DEBUG_PRINT("=");
  DEBUG_PRINT("T:"); //...the command type...
  DEBUG_PRINTI(rType, HEX);
  DEBUG_PRINT("|L:");  //...and the command length.
  DEBUG_PRINTI(rLength, HEX);
  DEBUG_PRINTL(DME?'D':'.');
#endif

  switch(rType){  //Select the command handler routine to jump to based on the command type
    case 0x00: command_reference(); break;
    case 0x01: command_open(); break;
    case 0x02: command_close(); break;
    case 0x03: command_read(); break;
    case 0x04: command_write(); break;
    case 0x05: command_delete(); break;
    case 0x06: command_format(); break;
    case 0x07: command_status(); break;
    case 0x08: command_DMEReq(); break; //DME Command
    case 0x0C: command_condition(); break;
    case 0x0D: command_rename(); break;
    default: return_normal(0x36); break;  //Send a normal return with a parameter error if the command is not implemented
  }

#if (DEBUG > 1)
  DEBUG_PRINTI(head,HEX);
  DEBUG_PRINT(":");
  DEBUG_PRINTI(tail,HEX);
  DEBUG_PRINT("->");
#endif

  tail = tail+rLength+0x05;  //Increment the tail index past the previous command

#if (DEBUG > 1)
  DEBUG_PRINTIL(tail,HEX);
#endif
}
