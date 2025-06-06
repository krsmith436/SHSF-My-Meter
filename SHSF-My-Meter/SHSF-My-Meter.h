// for data logging
const char* LOG_FILE = "/datalog.txt";
const char LOG_HEADER[36] = "Time,Temperature(degF),Altitude(ft)"; // = 35 chars + 1 null terminator
const float INTERVAL_LOG = 60; // value in seconds.
//
// for time information
const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
const long gmtOffset_sec = -18000;  // Adjust for your timezone
const int daylightOffset_sec = 3600;
const char* time_zone = "EST5EDT,M3.2.0,M11.1.0";  // TimeZone rule for America_Detroit.
//
// DNS Server
const byte DNS_PORT = 53;
const char *apSSID = "SHSF_MyMeter_Setup";
//
// for OpenWeatherMap
const char* apiKey = "17c1605d232037456e84471f772b3e3a"; // OpenWeatherMap API key "esp32-my-meter".
const char* lat = "42.5610";  // Wolverine Lake, MI latitude
const char* lon = "-83.4751"; // Wolverine Lake, MI longitude
//
// for display modes
const uint8_t WEATHER = 0;
const uint8_t CURRENT = 1;
const uint8_t BATTERY = 2;
const uint8_t WIFI = 3;
const uint8_t NUM_MODES = 3; // number of modes minus one.
//
// for display interval, in Seconds
const float INTERVAL_WEATHER = 2; // value in seconds.
const float INTERVAL_BATTERY = 2; // value in seconds.
const float INTERVAL_FAST_CURRENT = 0.001; // value in seconds.
const float INTERVAL_SLOW_CURRENT = 1; // value in seconds.
//
// for RGB LED
const float RGB_LED_BRIGHTNESS = 25;
const float RGB_ON_TIME = 0.03; // value in seconds.
//
// for Red LED
const float RED_OFF_TIME = 0.05; // value in seconds.
//
// for Touch switch
const int THRESHOLD = 1500;  // ESP32S2
//
// for OLED
const uint8_t OLED_WIDTH = 128; // OLED display width, in pixels
const uint8_t OLED_HEIGHT = 32; // OLED display height, in pixels
const int8_t OLED_RESET = -1; // Reset pin # (or -1 if sharing Arduino reset pin)
const int16_t ROW_1 = 0;
const int16_t ROW_2 = 12;
const int16_t ROW_3 = 24;
const int16_t COL_1 = 0;
const int16_t COL_2 = 60;
//
// for communications
const unsigned long COM_BAUD_RATE = 115200;
//
// for HTTP server
enum PageMode {
  PAGE_CREDENTIALS,
  PAGE_LOG
};
//
struct button {
  unsigned long previousMillis; // time since last button press.
  unsigned long interval; // interval at which to look for new press (de-bounce).
};
//--------------------- I2C Addresses ----------------------
// MAX17048 Battery Monitor (0x0c), cannot change.
// BME280 Temp/Hum/Pres Sensor (0x77), cannot change.
// Si7021 Temp/Hum Sensor (0x40). cannot change.
const uint8_t OLED_I2C_ADDR = 0x3C; // Address 0x3C for 128x32.
const uint8_t  INA219_I2C_ADDR = 0x41; // can change to 0x41, 0x44 or 0x45.
//
//-------------------- Pin assignments ---------------------
const uint8_t BUTTON_A  = 9;
const uint8_t BUTTON_B  = 6;
const uint8_t BUTTON_C  = 5;
const uint8_t TOUCH_1 = T12;
const int LED_PIN = LED_BUILTIN;  // the number of the LED pin