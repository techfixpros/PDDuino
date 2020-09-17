/* reboot */
// This hangs at boot if debug console is enabled. It seems to break the CONSOLE port on the way down,
// and then on the way back up, it sits in setup() waiting for the CONSOLE port.
// The fix is probaby to reorganize the sketch to avoid needing to restart:
// https://forum.arduino.cc/index.php?topic=381083.0
//void setup() { // leave empty }
//void loop()
//{
//  // ... initialize application here
//  while (// test application exit condition here )
//  {
//    // application loop code here
//  }
//}
void(* restart) (void) = 0;

#if DEBUG
void printDirectory(File dir, byte numTabs);
void printDirectory(File dir, byte numTabs) {
  char fileName[FILENAME_SZ] = "";

  SD_LED_ON
  while (true) {
    entry = dir.openNextFile();
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

#if defined(ENABLE_SLEEP)
void wakeNow () {}

void sleepNow() {
 #if defined(SLEEP_DELAY)  // no-op until SLEEP_DELAY expires
  now = millis();
  if ((now-idleSince)<SLEEP_DELAY) return;
  idleSince = now;
 #endif // SLEEP_DELAY
 //DEBUG_PRINT(F("sleep..."));
 #if defined(DEBUG_SLEEP)
  DEBUG_LED_ON
 #endif
 #if defined(USE_ALP)
  LowPower.attachInterruptWakeup(CLIENT_RX_PIN, wakeNow,LOW);
  #if DEBUG
  LowPower.idle();  // .idle() .sleep() .deepSleep()
  #else
  LowPower.sleep();  // .idle() .sleep() .deepSleep()
  #endif
 #else
  #if DEBUG
  set_sleep_mode(SLEEP_MODE_IDLE);  // power down breaks usb serial connection
  #else
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // lightest to deepest: _IDLE _ADC _PWR_SAVE _STANDBY _PWR_DOWN
  #endif // DEBUG
  attachInterrupt(rxInterrupt,wakeNow,LOW); // uart RX/TX rest high
  sleep_mode();
  detachInterrupt(rxInterrupt);
 #endif // USE_ALP
  //DEBUG_PRINTL(F("wake"));
 #if defined(DEBUG_SLEEP)
  DEBUG_LED_OFF
 #endif
}
#endif // ENABLE_SLEEP

void initCard () {
  while(true){
    DEBUG_PRINT(F("Opening SD card..."));
    SD_LED_ON
#if defined(USE_SDIO)
 #if DEBUG > 2
    DEBUG_PRINT(F("(SDIO)"));
 #endif
    if (SD.begin()) {
#else
 #if DEBUG > 2
    DEBUG_PRINT(F("(SPI)"));
 #endif
 #if defined(SD_CS_PIN) && defined(SD_SPI_MHZ)
    if (SD.begin(SD_CS_PIN,SD_SCK_MHZ(SD_SPI_MHZ))) {
 #elif defined(SD_CS_PIN)
    if (SD.begin(SD_CS_PIN)) {
 #else
    if (SD.begin()) {
 #endif
#endif  // USE_SDIO
      DEBUG_PRINTL(F("OK."));
#if SD_LED
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
#if SD_LED
      SD_LED_OFF
      delay(1000);
#endif
    }
  }

  SD.chvol();

  // TODO - get the FAT volume label and use it in place of rootLabel
  // ...non-trivial

  // Always do this open() & close(), even if we aren't doing the printDirectory()
  // It's needed to get the SdFat library to put the sd card to sleep.
  root = SD.open(directory);
#if DEBUG
  DEBUG_PRINTL(F("--- printDirectory(root,0) start ---"));
  printDirectory(root,0);
  DEBUG_PRINTL(F("--- printDirectory(root,0) end ---"));
#endif
  root.close();

  SD_LED_OFF
}
