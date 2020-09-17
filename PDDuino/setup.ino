void setup() {

  // leds
  PINMODE_SD_LED_OUTPUT
  PINMODE_DEBUG_LED_OUTPUT
  SD_LED_OFF
  DEBUG_LED_OFF
  digitalWrite(LED_BUILTIN,LOW); // in case the macros didn't

  // interrupt pins
  // pinMode(CLIENT_RX_PIN, INPUT_PULLUP);  // don't do on a HardwareSerial uart pin
#if defined(SD_CD_PIN)
  pinMode(SD_CD_PIN,INPUT_PULLUP);  // card-detect
#endif // SD_CD_PIN

  // DSR/DTR
#if defined(DTR_PIN)
  pinMode(DTR_PIN, OUTPUT);
  digitalWrite(DTR_PIN,HIGH);  // tell client we're not ready
#endif // DTR_PIN
#if defined(DSR_PIN)
  pinMode(DSR_PIN, INPUT_PULLUP);
#endif // DSR_PIN

// if debug console enabled, blink led and wait for console to be attached before proceeding
#if DEBUG && defined(CONSOLE)
  CONSOLE.begin(115200);
    while(!CONSOLE){
      DEBUG_LED_ON
      delay(20);
      DEBUG_LED_OFF
      delay(800);
    }
  CONSOLE.flush();
#endif

  DEBUG_PRINTL(F("init()"));

  // connect the client serial port
  CLIENT.begin(19200);
  CLIENT.flush();

  // clear the main data buffer
  for(byte i=0x00;i<FILE_BUFFER_SZ;++i) dataBuffer[i] = 0x00;

  // info dump
  DEBUG_PRINTL(F("\r\n-----------[ " SKETCH_NAME " " SKETCH_VERSION " ]------------"));
  DEBUG_PRINTL(F("BOARD_NAME: " BOARD_NAME));

  DEBUG_PRINT(F("DSR_PIN: "));
#if defined(DSR_PIN)
  DEBUG_PRINT(DSR_PIN);
#endif // DSR_PIN
  DEBUG_PRINTL("");

  DEBUG_PRINT("DTR_PIN: ");
#if defined(DTR_PIN)
  DEBUG_PRINT(DTR_PIN);
#endif // DTR_PIN
  DEBUG_PRINTL("");

  DEBUG_PRINT("LOADER_FILE: ");
#if defined(LOADER_FILE)
  DEBUG_PRINT(LOADER_FILE);
#endif // LOADER_FILE
  DEBUG_PRINTL("");

  DEBUG_PRINT(F("sendLoader(): "));
#if defined(DSR_PIN) && defined(LOADER_FILE)
  DEBUG_PRINTL(F("enabled"));
#else
  DEBUG_PRINTL(F("disabled"));
#endif // DSR_PIN && LOADER_FILE

  DEBUG_PRINT(F("Card-Detect pin: "));
#if defined(SD_CD_PIN)
  DEBUG_PRINT(SD_CD_PIN);
  DEBUG_PRINT(F(" INT: "));
  DEBUG_PRINT(digitalPinToInterrupt(SD_CD_PIN));
  DEBUG_PRINT(F(" state: "));
  DEBUG_PRINTL(digitalRead(SD_CD_PIN));
#else
  DEBUG_PRINTL(F("not enabled"));
#endif // SD_CD_PIN

#if defined(USE_SDIO)
 #if DEBUG > 2
 DEBUG_PRINTL(F("Using SDIO"));
 #endif // DEBUG
#else
 #if defined(DISABLE_CS)
  #if DEBUG > 2
   DEBUG_PRINTL(F("Using SPI"));
   DEBUG_PRINT(F("Disabling SPI device on pin: "));
   DEBUG_PRINTL(DISABLE_CS);
  #endif // DEBUG
   pinMode(DISABLE_CS, OUTPUT);
   digitalWrite(DISABLE_CS, HIGH);
 #else
   #if DEBUG > 2
   DEBUG_PRINTL(F("Assuming the SD is the only SPI device."));  
   #endif // DEBUG
 #endif // DISABLE_CS
 #if DEBUG > 2
  DEBUG_PRINT(F("Using SD chip select pin: "));
  #if defined(SD_CS_PIN)
  DEBUG_PRINT(SD_CS_PIN);
  #endif  // SD_CS_PIN
  DEBUG_PRINTL("");
 #endif  // DEBUG
#endif  // !USE_SDIO

// open the SD card
  initCard();

// If the card is ejected, immediately restart, so that we:
//   go off-line (DTR_PIN)
//   wait for a card to re-appear
//   open the new card
//   finally go back on-line.
// This comes after initCard(), so that while ejected, we wait in initCard() rather than in a boot loop.
#if defined(SD_CD_PIN)
  attachInterrupt(cdInterrupt, restart, LOW);
#endif

// tell client we're open for business
#if defined(DTR_PIN)
  digitalWrite(DTR_PIN,LOW);
#endif // DTR_PIN

// before proceeding to the TPDD main loop, see if client wants to bootstrap instead
#if defined(DSR_PIN) && defined(LOADER_FILE)
  if(!digitalRead(DSR_PIN)) {
    DEBUG_PRINTL(F("Client is asserting DSR. Doing sendLoader()."));
    sendLoader();
  } else {
    DEBUG_PRINTL(F("Client is not asserting DSR. Doing loop()."));
  }
#endif // DSR_PIN && LOADER_FILE

}
