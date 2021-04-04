// Teensy 3.5, 3.6, 4.1
// https://www.pjrc.com/store/teensy35.html
// https://www.pjrc.com/store/teensy36.html
// https://www.pjrc.com/store/teensy41.html

// No CD pin and no good alternative way to detect card.
// https://forum.pjrc.com/threads/43422-Teensy-3-6-SD-card-detect
// Might be able to use comment #10 idea.
//   #define CPU_RESTART_ADDR (uint32_t *)0xE000ED0C
//   #define CPU_RESTART_VAL 0x5FA0004
//   #define CPU_RESTART (*CPU_RESTART_ADDR = CPU_RESTART_VAL);
//   ...
//   if (!sd.exists("settings.txt")) { CPU_RESTART }

#if defined(ARDUINO_TEENSY35)
  #define BOARD_TEENSY35
  #define BOARD_NAME "Teensy 3.5"

#elif defined(ARDUINO_TEENSY36)
  #define BOARD_TEENSY36
  #define BOARD_NAME "Teensy 3.6"

#elif defined(ARDUINO_TEENSY41)
  #define BOARD_TEENSY41
  #define BOARD_NAME "Teensy 4.1"
#endif

#define USE_SDIO
#define ENABLE_SLEEP

// #define SD_CD_PIN 37       // hardware mod: https://photos.app.goo.gl/sGP6qJuHd2QBdYoh9
// #define SD_CD_PRESENT LOW  // pin is low when card is inserted

// Main LED: PB5
#define PINMODE_SD_LED_OUTPUT DDRB = DDRB |= 1UL << 5;
#define SD_LED_ON PORTB |= _BV(5);
#define SD_LED_OFF PORTB &= ~_BV(5);

// Main LED: PB5
#define PINMODE_DEBUG_LED_OUTPUT PINMODE_SD_LED_OUTPUT
#define DEBUG_LED_ON SD_LED_ON
#define DEBUG_LED_OFF SD_LED_OFF
