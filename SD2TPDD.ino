/*
 * SD2TPDD  V0.2
 * A TPDD emulator for the Arduino Mega that uses an SD card for mass storage.
 * Written by Jimmy Pettit
 * 07/27/2018
 */

// sd card reader chip-select pin # - comment out for Teensy 3.5/3.6
#define SD_CHIP_SELECT 4

// -1 = disabled, otherwise pin#
#define DISABLE_CHIP_SELECT -1

// sd card reader communication method 0 = SPI (most boards), 1 = SDIO (Teensy 3.5/3.6)
#define USE_SDIO 0

// TPDD client serial port
#define CLIENT Serial1

// Use pin# for RX of CLIENT port to wake from sleep on CLIENT serial activity
#define WAKE_PIN 0
#define WAKE_TRIGGER FALLING
#define SLEEP_MODE SLEEP_MODE_PWR_DOWN
#define SLEEP_INHIBIT_MILLIS 30000  // 300,000 = 5 minutes, 0 = disabled

// disk activity light - 0 = disabled, 1 = Adalogger 32u4, 2 = Teensy 3.5/3.6
#define DISK_ACTIVITY_LIGHT 1

// serial monitor port - 0 = disabled, 1 = enabled
#define DEBUG 0
#define CONSOLE Serial

// Displayed in the top-right corner in TS-DOS
// Must be exactly 6 characters. (not counting trailing null)
const char defaultLabel[] = "SDTPDD";

//-----------------------------------------------------------------------------

// SLEEP_MODE_PWR_DOWN screws up the usb serial device on the host pc
// if debug, then override normal desired sleep mode setting
#if DEBUG && (CONSOLE == Serial)
#define SLEEP_MODE SLEEP_MODE_IDLE
#endif

// Debug led, ie to show sleepNow without using Serial within ISR
#if 0
// Adalogger 32u4 led = PC7
// Teensy led = PB5
#define PINMODE_DEBUG_LED_OUTPUT DDRC = DDRC |= 1UL << 7;
#define DEBUG_LED_ON PORTC |= _BV(7);
#define DEBUG_LED_OFF PORTC &= ~_BV(7);
#else
// disabled
#define PINMODE_DEBUG_LED_OUTPUT
#define DEBUG_LED_ON
#define DEBUG_LED_OFF
#endif

// turn led on/off by direct port manipulation, digitalWrite() is inefficient
#if DISK_ACTIVITY_LIGHT
  #if DISK_ACTIVITY_LIGHT == 1
    // Adalogger 32u4 green LED near card reader is PB4
    #define PINMODE_SD_LED_OUTPUT DDRB = DDRB |= 1UL << 4;
    #define SD_LED_ON PORTB |= _BV(4);
    #define SD_LED_OFF PORTB &= ~_BV(4);
  #endif
  #if DISK_ACTIVITY_LIGHT == 2
    // Teensy 3.5/3.6 on-board led is PB5
    #define PINMODE_SD_LED_OUTPUT DDRB = DDRB |= 1UL << 5;              // pinMode(13,OUTPUT);
    #define SD_LED_ON PORTB |= _BV(5);                                  // digitalWrite(13,HIGH);
    #define SD_LED_OFF PORTB &= ~_BV(5);                                // digitalWrite(13,LOW);
  #endif
#else
  // no disk-activity light
  #define PINMODE_SD_LED_OUTPUT
  #define SD_LED_ON
  #define SD_LED_OFF
#endif

#if DEBUG && defined(CONSOLE)
 #define DEBUG_BEGIN        CONSOLE.begin(9600);
 #define DEBUG_PRINT(x)     CONSOLE.print (x)
 #define DEBUG_PRINTI(x,y)  CONSOLE.print (x,y)
 #define DEBUG_PRINTL(x)    CONSOLE.println (x)
 #define DEBUG_PRINTIL(x,y) CONSOLE.println (x,y)
#else
 #define DEBUG_BEGIN
 #define DEBUG_PRINT(x)
 #define DEBUG_PRINTI(x,y)
 #define DEBUG_PRINTL(x)
 #define DEBUG_PRINTIL(x,y)
#endif

#include <SdFat.h>
#include <avr/sleep.h>

SdFat SD; //SD card object

File root;  //Root file for filesystem reference
File entry; //Moving file entry for the emulator
File tempEntry; //Temporary entry for moving files

byte head = 0x00;  //Head index
byte tail = 0x00;  //Tail index

byte checksum = 0;  //Global variable for checksum calculation

byte state = 0; //Emulator command reading state
bool DME = false; //TS-DOS DME mode flag

byte dataBuffer[256]; //Data buffer for commands
byte fileBuffer[0x80]; //Data buffer for file reading

char refFileName[25] = "";  //Reference file name for emulator
char refFileNameNoDir[25] = ""; //Reference file name for emulator with no ".<>" if directory
char tempRefFileName[25] = ""; //Second reference file name for renaming
char entryName[25] = "";  //Entry name for emulator
int directoryBlock = 0; //Current directory block for directory listing
char directory[60] = "/";
byte directoryDepth = 0;
char tempDirectory[60] = "/";
char dmeLabel[7] = "------";
const byte wakeInterrupt = digitalPinToInterrupt(WAKE_PIN);
#if SLEEP_INHIBIT_MILLIS
unsigned long now = millis();
unsigned long idleSince = now;
#endif

void setup() {
  PINMODE_SD_LED_OUTPUT
  PINMODE_DEBUG_LED_OUTPUT
  //pinMode(WAKE_PIN, INPUT_PULLUP);  // normally need, but not for RX

  attachInterrupt(wakeInterrupt,wakeNow,WAKE_TRIGGER);
  
  DEBUG_BEGIN
  CLIENT.begin(19200);  //Start the main serial port
  CLIENT.flush();

  clearBufferB(dataBuffer); //Clear the data buffer

  #if DEBUG
  while (!CONSOLE);
  #endif
  DEBUG_PRINTL("\r\n-----------[ SD2TPDD setup() ]------------");
  DEBUG_PRINT("SLEEP_MODE: ");
  DEBUG_PRINTL(SLEEP_MODE);

#if !USE_SDIO  
  #if (DISABLE_CHIP_SELECT < 0)
    //DEBUG_PRINTL("Assuming the SD is the only SPI device.");
  #else
    //DEBUG_PRINT("Disabling SPI device on pin ");
    //DEBUG_PRINTL(DISABLE_CHIP_SELECT);
    pinMode(DISABLE_CHIP_SELECT, OUTPUT);
    digitalWrite(DISABLE_CHIP_SELECT, HIGH);
  #endif
  //DEBUG_PRINT("Using SD chip select pin: ");
  //DEBUG_PRINTL(SD_CHIP_SELECT);
#endif  // !USE_SDIO  

  while(!initCard());
}

/*
 * 
 * General misc. routines
 * 
 */

void wakeNow () {
}

void sleepNow() {
#if SLEEP_INHIBIT_MILLIS
    now = millis();
    if ((now-idleSince)<SLEEP_INHIBIT_MILLIS) return;
    idleSince = now;
#endif
    set_sleep_mode(SLEEP_MODE);
    DEBUG_LED_ON
    attachInterrupt(wakeInterrupt,wakeNow,WAKE_TRIGGER);
    sleep_mode();
    detachInterrupt(wakeInterrupt);
    DEBUG_LED_OFF
}

// Input: char[] = "******",  Output: dmeLabel = " ******.<> "
/*
void setLabel(char* s) {
  DEBUG_PRINT("setLabel(): \"");
  DEBUG_PRINT(dmeLabel);
  DEBUG_PRINT("\" -> \"");
  int i = 0;
  dmeLabel[i] = ' ';
  for (i=1;i<7;i++) dmeLabel[i]=s[i-1];
  dmeLabel[i] = '.';
  i++;
  dmeLabel[i] = '<';
  i++;
  dmeLabel[i] = '>';
  i++;
  dmeLabel[i] = ' ';
  dmeLabel[12] = 0;
  DEBUG_PRINT(dmeLabel);
  DEBUG_PRINTL("\"");
}
*/

void setLabel(char* s) {
  DEBUG_PRINT("setLabel(): \"");
  DEBUG_PRINT(dmeLabel);
  DEBUG_PRINT("\" -> \"");
  for (int i=0;i<6;++i) dmeLabel[i]=s[i];
  dmeLabel[6] = 0;
  DEBUG_PRINT(dmeLabel);
  DEBUG_PRINTL("\"");
}

bool initCard () {
  DEBUG_PRINT("Opening SD card...");
  SD_LED_ON
#if USE_SDIO
  if (SD.begin()) {
#else  // USE_SDIO
  //if (SD.begin(SD_CHIP_SELECT,SD_SCK_MHZ(50))) {
  if (SD.begin(SD_CHIP_SELECT)) {
#endif  // USE_SDIO
    DEBUG_PRINTL("OK.");
    SD_LED_OFF
    delay (80);
    SD_LED_ON
    delay (80);
    SD_LED_OFF
    delay (80);
    SD_LED_ON
    delay (80);
  } else {
    DEBUG_PRINTL("No SD card.");
    SD_LED_OFF
    delay(1000);
    return false;
  }

  SD.chvol();

  // TODO - get the FAT volume label and use it inplace of defaultLabel

  // Always do the open & close, even if we aren't doing the printDirectory()
  // It's needed to get the SdFat library to put the sd card to sleep.
    root = SD.open(directory);
    if(root) {
      setLabel(defaultLabel);
    } else {
      setLabel("------");
    }
#if DEBUG
    DEBUG_PRINTL("--- printDirectory(root,0) start ---");
    printDirectory(root,0);
    DEBUG_PRINTL("--- printDirectory(root,0) end ---");
#endif
    root.close();

  SD_LED_OFF
  return true;
}

#if DEBUG
void printDirectory(File dir, int numTabs) {
  char fileName[24] = "";

  SD_LED_ON
  while (true) {
    File entry =  dir.openNextFile();
    if (!entry) break;
    for (uint8_t i = 0; i < numTabs; i++) DEBUG_PRINT('\t');
    entry.getName(fileName,24);
    DEBUG_PRINT(fileName);
    if (entry.isDirectory()) {
      DEBUG_PRINTL("/");
      DEBUG_PRINT("--- printDirectory(");
      DEBUG_PRINT(fileName);
      DEBUG_PRINT(',');
      DEBUG_PRINT(numTabs+1);
      DEBUG_PRINTL(") start ---");
      printDirectory(entry, numTabs + 1);
      DEBUG_PRINT("--- printDirectory(");
      DEBUG_PRINT(fileName);
      DEBUG_PRINT(',');
      DEBUG_PRINT(numTabs+1);
      DEBUG_PRINTL(") end ---");
    } else {
      // files have sizes, directories do not
      DEBUG_PRINT("\t\t");
      DEBUG_PRINTIL(entry.fileSize(), DEC);
    }
    entry.close();
  }
  SD_LED_OFF
}
#endif // DEBUG

void clearBufferB(byte* a){
  for(int i=0;i<sizeof(a);++i) a[i] = 0;
}
void clearBufferC(char* a){
  for(int i=0;i<sizeof(a);++i) a[i] = 0;
}

void directoryAppend(char* c){  //Copy a null-terminated char array to the directory array
  bool terminated = false;
  int i = 0;
  int j = 0;
  
  while(directory[i] != 0x00){  //Jump i to first null character
    i++;
  }

  while(!terminated){
    directory[i++] = c[j++];
    terminated = c[j] == 0x00;
  }
  DEBUG_PRINTL(directory);
}

void upDirectory(){ //Removes the top-most entry in the directoy path
  int j = sizeof(directory);

  while(directory[j] == 0x00){ //Jump to first non-null character
    j--;
  }

  if(directory[j] == '/' && j!= 0x00){  //Strip away the slash character
    directory[j] = 0x00;
  }

  while(directory[j] != '/'){ //Move towards the front of the array until a slash character is encountered...
    directory[j--] = 0x00;  //...set everything along the way to null characters
  }
}

void copyDirectory(){ //Makes a copy of the working directory to a scratchpad
  for(int i=0; i<sizeof(directory); i++){
    tempDirectory[i] = directory[i];
  }
}

/*
 * 
 * TPDD Port misc. routines
 * 
 */

void tpddWrite(char c){  //Outputs char c to TPDD port and adds to the checksum
  checksum += c;
  CLIENT.write(c);
}

void tpddWriteString(char* c){  //Outputs a null-terminated char array c to the TPDD port
  int i = 0;
  while(c[i]!=0){
    checksum += c[i];
    CLIENT.write(c[i]);
    i++;
  }
}

void tpddSendChecksum(){  //Outputs the checksum to the TPDD port and clears the checksum
  CLIENT.write(checksum^0xFF);
  checksum = 0;
}


/*
 * 
 * TPDD Port return routines
 * 
 */
 
void return_normal(byte errorCode){ //Sends a normal return to the TPDD port with error code errorCode
  DEBUG_PRINT("R:Norm ");
  DEBUG_PRINTIL(errorCode, HEX);

  tpddWrite(0x12);  //Return type (normal)
  tpddWrite(0x01);  //Data size (1)
  tpddWrite(errorCode); //Error code
  tpddSendChecksum(); //Checksum
}

void return_reference(){  //Sends a reference return to the TPDD port
  byte term = 6;
  bool terminated = false;
  tpddWrite(0x11);  //Return type (reference)
  tpddWrite(0x1C);  //Data size (1C)

  clearBufferC(tempRefFileName);  // Clear the reference file name buffer

  entry.getName(tempRefFileName,24);  //Save the current file entry's name to the reference file name buffer
  
  if(DME && entry.isDirectory()){ //      !!!Tacks ".<>" on the end of the return reference if we're in DME mode and the reference points to a directory
    for(int i=0; i < 7; i++){ //Find the end of the directory's name by looping through the name buffer
      if(tempRefFileName[i] == 0x00){
        term = i; //and setting a termination index to the offset where the termination is encountered
      }
    }
    tempRefFileName[term++] = '.';  //Tack the expected ".<>" to the end of the name
    tempRefFileName[term++] = '<';
    tempRefFileName[term++] = '>';

    for(int i=term; i<24; i++){ //Fill the rest of the reference name with null characters
      tempRefFileName[i] = 0x00;
    }
    term = 6; //Reset the termination index to prepare for the next check
  }

  

  for(int i=0; i<6; i++){ //      !!!Pads the name of the file out to 6 characters using space characters
    if(term == 6){  //Perform these checks if term hasn't changed
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

  for(int i=0; i<18; i++){  //      !!!Outputs the file extension part of the reference name starting at the offset found above
    tpddWrite(tempRefFileName[i+term]);
  }

  tpddWrite(0x00);  //Attribute, unused
  tpddWrite((byte)((entry.fileSize()&0xFF00)>>8));  //File size most significant byte
  tpddWrite((byte)(entry.fileSize()&0xFF)); //File size least significant byte
  tpddWrite(0x80);  //Free sectors, SD card has more than we'll ever care about
  tpddSendChecksum(); //Checksum

  DEBUG_PRINTL("R:Ref");
}

void return_blank_reference(){  //Sends a blank reference return to the TPDD port
  tpddWrite(0x11);  //Return type (reference)
  tpddWrite(0x1C);  //Data size (1C)

  for(int i=0; i<24; i++){
    tpddWrite(0x00);  //Write the reference file name to the TPDD port
  }

  tpddWrite(0x00);  //Attribute, unused
  tpddWrite(0x00);  //File size most significant byte
  tpddWrite(0x00); //File size least significant byte
  tpddWrite(0x80);  //Free sectors, SD card has more than we'll ever care about
  tpddSendChecksum(); //Checksum

  DEBUG_PRINTL("R:BRef");
}

void return_parent_reference(){
  tpddWrite(0x11);
  tpddWrite(0x1C);

  tpddWriteString("PARENT.<>");
  for(int i=9; i<24; i++){  //Pad the rest of the data field with null characters
    tpddWrite(0x00);
  }

  tpddWrite(0x00);  //Attribute, unused
  tpddWrite(0x00);  //File size most significant byte
  tpddWrite(0x00); //File size least significant byte
  tpddWrite(0x80);  //Free sectors, SD card has more than we'll ever care about
  tpddSendChecksum(); //Checksum
}

/*
 * 
 * TPDD Port command handler routines
 * 
 */

void command_reference(){ //Reference command handler
  byte searchForm = dataBuffer[(byte)(tail+29)];  //The search form byte exists 29 bytes into the command
  byte refIndex = 0;  //Reference file name index
  
  DEBUG_PRINT("SF:");
  DEBUG_PRINTIL(searchForm,HEX);
  
  if(searchForm == 0x00){ //Request entry by name
    for(int i=4; i<28; i++){  //Put the reference file name into a buffer
        if(dataBuffer[(byte)(tail+i)]!=0x20){ //If the char pulled from the command is not a space character (0x20)...
          refFileName[refIndex++]=dataBuffer[(byte)(tail+i)]; //write it into the buffer and increment the index. 
        }
    }
    refFileName[refIndex]=0x00; //Terminate the file name buffer with a null character

    DEBUG_PRINT("Ref: ");
    DEBUG_PRINTL(refFileName);

    if(DME){  //        !!!Strips the ".<>" off of the reference name if we're in DME mode
      if(strstr(refFileName, ".<>") != 0){
        for(int i=0; i<24; i++){  //Copies the reference file name to a scratchpad buffer with no directory extension if the reference is for a directory
          if(refFileName[i] != '.' && refFileName[i] != '<' && refFileName[i] != '>'){
            refFileNameNoDir[i]=refFileName[i];
          }else{
            refFileNameNoDir[i]=0x00; //If the character is part of a directory extension, don't copy it
          } 
        }
      }else{
        for(int i=0; i<24; i++){
          refFileNameNoDir[i]=refFileName[i]; //Copy the reference directly to the scratchpad buffer if it's not a directory reference
        }
      }
    }

    directoryAppend(refFileNameNoDir);  //Add the reference to the directory buffer

    SD_LED_ON
    if(SD.exists(directory)){ //If the file or directory exists on the SD card...
      entry=SD.open(directory); //...open it...
      SD_LED_OFF
      return_reference(); //send a refernce return to the TPDD port with its info...
      entry.close();  //...close the entry.
    }else{  //If the file does not exist...
      SD_LED_OFF
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
    SD_LED_OFF
    return_normal(0x36);  //Send a normal return to the TPDD port with a parameter error
  }
}

void ref_openFirst(){
  directoryBlock = 0; //Set the current directory entry index to 0
  if(DME && directoryDepth>0 && directoryBlock==0){ //Return the "PARENT.<>" reference if we're in DME mode
    return_parent_reference();
  }else{
    ref_openNext();    //otherwise we just return the next reference
  }
}

void ref_openNext(){
  directoryBlock++; //Increment the directory entry index
  SD_LED_ON
    root.rewindDirectory(); //Pull back to the begining of the directory
  for(int i=0; i<directoryBlock-1; i++){  //skip to the current entry offset by the index
    root.openNextFile();
  }

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
  byte rMode = dataBuffer[(byte)(tail+4)];  //The access mode is stored in the 5th byte of the command
  entry.close();

  SD_LED_ON

  if(DME && strcmp(refFileNameNoDir, "PARENT") == 0){ //If DME mode is enabled and the reference is for the "PARENT" directory
    upDirectory();  //The top-most entry in the directory buffer is taken away
    directoryDepth--; //and the directory depth index is decremented
  }else{
    directoryAppend(refFileNameNoDir);  //Push the reference name onto the directory buffer
    if(DME && (int)strstr(refFileName, ".<>") != 0 && !SD.exists(directory)){ //If the reference is for a directory and the directory buffer points to a directory that does not exist
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
  entry.close();  //Close the entry
  SD_LED_OFF
  return_normal(0x00);  //Normal return with no error
}

void command_read(){  //Read a block of data from the currently open entry
  SD_LED_ON
  int bytesRead = entry.read(fileBuffer, 0x80); //Try to pull 128 bytes from the file into the buffer
  SD_LED_OFF
  DEBUG_PRINT("A: ");
  DEBUG_PRINTIL(entry.available(),HEX);

  if(bytesRead > 0){  //Send the read return if there is data to be read
    tpddWrite(0x10);  //Return type
    tpddWrite(bytesRead); //Data length
    for(int i=0; i<bytesRead; i++){
      tpddWrite(fileBuffer[i]);
    }
    tpddSendChecksum();
  }else{
    return_normal(0x3F);  //send a normal return with an end-of-file error if there is no data left to read
  }
}

void command_write(){ //Write a block of data from the command to the currently open entry
  byte commandDataLength = dataBuffer[(byte)(tail+3)];

  SD_LED_ON
  for(int i=0; i<commandDataLength; i++){
      entry.write(dataBuffer[(byte)(tail+4+i)]);
  }
  SD_LED_OFF  
  return_normal(0x00);  //Send a normal return to the TPDD port with no error
}

void command_delete(){  //Delete the currently open entry
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
  return_normal(0x00);
}

void command_status(){  //Drive status
  return_normal(0x00);
}

void command_condition(){ //Not implemented
  return_normal(0x00);
}

void command_rename(){  //Renames the currently open entry
  byte refIndex = 0;  //Temporary index for the reference name
  
  directoryAppend(refFileNameNoDir);  //Push the current reference name onto the directory buffer

  SD_LED_ON
  
  if(entry){entry.close();} //Close any currently open entries
  entry = SD.open(directory); //Open the entry
  if(entry.isDirectory()){  //Append a slash to the end of the directory buffer if the reference is a sub-directory
    directoryAppend("/");
  }
  copyDirectory();  //Copy the directory buffer to the scratchpad directory buffer
  upDirectory();  //Strip the previous directory reference off of the directory buffer
  
  for(int i=4; i<28; i++){  //Loop through the command's data block, which contains the new entry name
      if(dataBuffer[(byte)(tail+i)]!=0x20 && dataBuffer[(byte)(tail+i)]!=0x00){ //If the current character is not a space (0x20) or null character...
        tempRefFileName[refIndex++]=dataBuffer[(byte)(tail+i)]; //...copy the character to the temporary reference name and increment the pointer.
      }
  }
  
  tempRefFileName[refIndex]=0x00; //Terminate the temporary reference name with a null character

  if(DME && entry.isDirectory()){ //      !!!If the entry is a directory, we need to strip the ".<>" off of the new directory name
    if(strstr(tempRefFileName, ".<>") != 0){
      for(int i=0; i<24; i++){
        if(tempRefFileName[i] == '.' || tempRefFileName[i] == '<' || tempRefFileName[i] == '>'){
          tempRefFileName[i]=0x00;
        } 
      }
    }
  }

  directoryAppend(tempRefFileName);
  if(entry.isDirectory()){
    directoryAppend("/");
  }

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

void command_DMEReq(){  //Send the DME return with the root directory's name
  if(DME){
    tpddWrite(0x12);
    tpddWrite(0x0B);
    tpddWrite(' ');
    tpddWriteString(dmeLabel);
    tpddWriteString(".<> ");
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
  byte rType = 0; //Current request type (command type)
  byte rLength = 0; //Current request length (command length)
  byte diff = 0;  //Difference between the head and tail buffer indexes

  state = 0; //0 = waiting for command 1 = waiting for full command 2 = have full command

  sleepNow();
  while(state<2){ //While waiting for a command...
    sleepNow();
    while (CLIENT.available() > 0){  //While there's data to read from the TPDD port...
#if SLEEP_INHIBIT_MILLIS
      idleSince = millis();
#endif
      dataBuffer[head++]=(byte)CLIENT.read();  //...pull the character from the TPDD port and put it into the command buffer, increment the head index...
      if(tail==head){ //...if the tail index equals the head index (a wrap-around has occoured! data will be lost!)
        tail++; //...increment the tail index to prevent the command size from overflowing.
      }
      DEBUG_PRINTI((byte)(head-1),HEX);
      DEBUG_PRINT("-");
      DEBUG_PRINTI(tail,HEX);
      DEBUG_PRINTI((byte)(head-tail),HEX);
      DEBUG_PRINT(":");
      DEBUG_PRINTI(dataBuffer[head-1],HEX);
      DEBUG_PRINT(";");
      DEBUG_PRINTL((dataBuffer[head-1]>=0x20)&&(dataBuffer[head-1]<=0x7E)?(char)dataBuffer[head-1]:' ');
    }

    diff=(byte)(head-tail); //...set the difference between the head and tail index (number of bytes in the buffer)

    if(state == 0){ //...if we're waiting for a command...
      if(diff >= 4){  //...if there are 4 or more characters in the buffer...
        if(dataBuffer[tail]=='Z' && dataBuffer[(byte)(tail+1)]=='Z'){ //...if the buffer's first two characters are 'Z' (a TPDD command)
          rLength = dataBuffer[(byte)(tail+3)]; //...get the command length...
          rType = dataBuffer[(byte)(tail+2)]; //...get the command type...
          state = 1;  //...set the state to "waiting for full command".
        }else if(dataBuffer[tail]=='M' && dataBuffer[(byte)(tail+1)]=='1'){ //If a DME command is received
          DME = true; //set the DME mode flag to true
          tail=tail+2;  //and skip past the command to the DME request command
        }else{  //...if the first two characters are not 'Z'...
          tail=tail+(tail==head?0:1); //...move the tail index forward to the next character, stop if we reach the head index to prevent an overflow.
        }
      }
    }

    if(state == 1){ //...if we're waiting for the full command to come in...
      if(diff>rLength+4){ //...if the amount of data in the buffer satisfies the command length...
          state = 2;  //..set the state to "have full command".
        }
    }
  } 

  DEBUG_PRINTI(tail,HEX); // show the tail index in the buffer where the command was found...
  DEBUG_PRINT("=");
  DEBUG_PRINT("T:"); //...the command type...
  DEBUG_PRINTI(rType, HEX);
  DEBUG_PRINT("|L:");  //...and the command length.
  DEBUG_PRINTI(rLength, HEX);
  DEBUG_PRINTL(DME?'D':'.');
  
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
  
  DEBUG_PRINTI(head,HEX);
  DEBUG_PRINT(":");
  DEBUG_PRINTI(tail,HEX);
  DEBUG_PRINT("->");
  tail = tail+rLength+5;  //Increment the tail index past the previous command
  DEBUG_PRINTIL(tail,HEX);
}
