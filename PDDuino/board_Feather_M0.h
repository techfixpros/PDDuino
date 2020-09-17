// Adafruit Feather M0 Adalogger
// https://learn.adafruit.com/adafruit-feather-m0-adalogger

#define BOARD_FEATHERM0
#define BOARD_NAME "Adafruit Feather M0"

#define SD_CS_PIN 4
#define SD_CD_PIN 7
#define SD_SPI_MHZ 12  // https://github.com/adafruit/ArduinoCore-samd/pull/186
#define ENABLE_SLEEP
#define USE_ALP
#define SLEEP_DELAY 250

// Green LED near card reader: pin 8
#define PINMODE_SD_LED_OUTPUT pinMode(8,OUTPUT);
#define SD_LED_ON digitalWrite(8,HIGH);
#define SD_LED_OFF digitalWrite(8,LOW);

