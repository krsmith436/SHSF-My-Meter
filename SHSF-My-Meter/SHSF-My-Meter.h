// all # define options should be defined here,  this keeps them together in a nice way.
//
// for display modes
const uint8_t WEATHER = 0;
const uint8_t CURRENT = 1;
const uint8_t BATTERY = 2;
const uint8_t NUM_MODES = 2; // number of modes, starting at zero.
//
// for display interval, IN Seconds
const float INTERVAL_WEATHER = 2; // value in seconds.
const float INTERVAL_BATTERY = 2; // value in seconds.
const float INTERVAL_FAST_CURRENT = 0.001; // value in seconds.
const float INTERVAL_SLOW_CURRENT = 1; // value in seconds.
//
// for RGB LED
const float RGB_LED_BRIGHTNESS = 25;
const float RGB_ON_TIME = 0.03; // value in seconds.
//
// for OLED
#define OLED_WIDTH 128 // OLED display width, in pixels
#define OLED_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define ROW_1 0
#define ROW_2 12
#define ROW_3 24
//
// for BME280
#define SEALEVELPRESSURE_HPA (1013.25)
//
// for communications
const unsigned long COM_BAUD_RATE = 115200;
//
struct button {
  unsigned long previousMillis; // time since last button press.
  unsigned long interval; // interval at which to look for new press (de-bounce).
};
//--------------------- I2C Addresses ----------------------
// MAX17048 Battery Monitor (0x0c), cannot change.
// BME280 Temp/Hum/Pres Sensor (0x77), cannot change.
// Si7021 Temp/Hum Sensor (0x40). cannot change.
#define OLED_I2C_ADDR 0x3C // Address 0x3C for 128x32.
#define INA219_I2C_ADDR 0x41 // can change to 0x41, 0x44 or 0x45.
//
//-------------------- Pin assignments ---------------------
#define BUTTON_A  9
#define BUTTON_B  6
#define BUTTON_C  5