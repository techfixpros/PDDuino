// Adafruit Feather 32u4 Adalogger
// https://learn.adafruit.com/adafruit-feather-32u4-adalogger

#define BOARD_FEATHER32U4
#define BOARD_NAME "Adafruit Feather 32u4"

#define SD_CS_PIN 4
#define SD_CD_PIN 7
#define ENABLE_SLEEP
#define SLEEP_DELAY 5000	// this board needs a few seconds before sleeping

// Green LED near card reader: PB4 / pin 8
#define PINMODE_SD_LED_OUTPUT DDRB = DDRB |= 1UL << 4;
#define SD_LED_ON PORTB |= _BV(4);
#define SD_LED_OFF PORTB &= ~_BV(4);

// Main LED: PC7 / pin 13
#define PINMODE_DEBUG_LED_OUTPUT DDRC = DDRC |= 1UL << 7;
#define DEBUG_LED_ON PORTC |= _BV(7);
#define DEBUG_LED_OFF PORTC &= ~_BV(7);
