// board_*.h - User-defined options and board-specific details
// board_Defaults.h supplies default values not supplied by some other board_*.h file

// Always supply the default. It will only be used if DEBUG
#if !defined(CONSOLE)
#define CONSOLE SERIAL_PORT_MONITOR		// where to send DEBUG messages, if enabled. (Serial)
#endif

// CLIENT must always exist
#if !defined(CLIENT)
#define CLIENT SERIAL_PORT_HARDWARE_OPEN	// what serial port is the TPDD client connected to (Serial1)
#endif

// Supply the default if undefined
// Disable if explicitly defined -1
#if !defined(DTR_PIN)
#define DTR_PIN 5			// pin to output DTR, github.com/bkw777/MounT uses pin 5
#elif DTR_PIN < 0
#undef DTR_PIN
#endif

// Supply the default if undefined
// Disable if explicitly defined -1
#if !defined(DSR_PIN)
#define DSR_PIN 6			// pin to input DTR, github.com/bkw777/MounT uses pin 6
#elif DSR_PIN < 0
#undef DSR_PIN
#endif

// Allow these to be undefined
//#define SD_CS_PIN SS			// sd card reader chip-select pin #, sometimes automatic
//#define SD_CD_PIN 7			// sd card reader card-detect pin #, interrupt & restart if card ejected
//#define DISABLE_CS 10			// Disable other SPI device on this pin, usully none, assume SD is only SPI device
//#define SD_SPI_MHZ 12			// override default SPI clock for SD card reader (Adafruit SAMD needs it)
//#define USE_SDIO			// sd card reader is connected by SDIO instead of SPI (Teensy 3.5/3.6/4.1)
//#define ENABLE_SLEEP			// hardware sleep while idle
//#define USE_ALP			// Use ArduinoLowPower instead of avr/sleep.h, SAMD only (Feather M0)

// Always supply the default. Only used if ENABLE_SLEEP.
#if !defined(SLEEP_DELAY)
#define SLEEP_DELAY 0			// Delay in ms before sleeping
#endif

// Always supply the default. Only used if ENABLE_SLEEP.
#if !defined(CLIENT_RX_PIN)
#define CLIENT_RX_PIN 0			// CLIENT RX pin#, interrupt is attached to wake from sleep
#endif

// Activity light for the sd card reader.
// Supply the default if undefined.
// Disable by explicitly defining empty in board_*.h
#if !defined(PINMODE_SD_LED_OUTPUT)
#define PINMODE_SD_LED_OUTPUT pinMode(LED_BUILTIN,OUTPUT);
#endif
// same as above
#if !defined(SD_LED_ON)
#define SD_LED_ON digitalWrite(LED_BUILTIN,HIGH);
#endif
// same as above
#if !defined(SD_LED_OFF)
#define SD_LED_OFF digitalWrite(LED_BUILTIN,LOW);
#endif

// LED for debugging some events without the serial console.
// ( sleep_mode(), wait-for-sd in setup() )
// Always supply the default. It will only be used if DEBUG_LED
#if !defined(PINMODE_DEBUG_LED_OUTPUT)
#define PINMODE_DEBUG_LED_OUTPUT pinMode(LED_BUILTIN,OUTPUT);
#endif
// same as above
#if !defined(DEBUG_LED_ON)
#define DEBUG_LED_ON digitalWrite(LED_BUILTIN,HIGH);
#endif
// same as above
#if !defined(DEBUG_LED_OFF)
#define DEBUG_LED_OFF digitalWrite(LED_BUILTIN,LOW);
#endif
