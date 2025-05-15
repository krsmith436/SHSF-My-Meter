/***************************************************************************
  Sketch Name: SHSF_My_Meter
  Written By: Kevin R. Smith
  Created: 2025-Mar-01

  This sketch is a multi-meter for the Smith, Huotari & Santa Fe (SHSF) HO scale model railroad.
  The meter is implemented using a Adafruit ESP32-S2 Feather with BME280 and Adafruit OLED FeatherWing.
  Included is a INA219 Current Sensor to provide a Volt/Current meter.

  Parameters Displayed:
    Ambient Temperature
    Humidity
    Barametric Pressure
    Altitude
    Battery Voltage
    Battery Charge
    Battery Charge Rate
    External Voltage
    External Current
****************************************************************************
*/
//-----------------Calling libraries needed to run-----------//
#include <Wire.h>
#include "Adafruit_MAX1704X.h" // for Battery Monitor.
#include <Adafruit_SSD1306.h> // for OLED FeatherWing display.
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h> // for Temperature/Humidity/Pressure Sensor.
#include "Adafruit_Si7021.h" // for Temperature/Humidity Sensor.
#include <INA219_WE.h> // for INA219 Current Sensor.
#include <Ticker.h> // for Ticker callbacks, which can call a function in a predetermined interval.
#include <Preferences.h> // wrapper for EEPROM library used to store WiFi ssid and password.
#include <WiFi.h> // to connect to your Wireless Fidelity (WiFi) network.
#include <HTTPClient.h> // for Sea Level Pressure.
#include <WebServer.h> //(ESP32 built-in) to create a lightweight HTTP server
#include <LittleFS.h> // to serve and manage the log file.
#include <ArduinoJson.h> // for Sea Level Pressure.
#include "time.h"
#include "SHSF-My-Meter.h"
//
//-------------------Object Instantiation-------------------//
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);
Adafruit_MAX17048 maxlipo;
Adafruit_BME280 bme; // I2C
Adafruit_Si7021 si7021 = Adafruit_Si7021();
INA219_WE ina219 = INA219_WE(INA219_I2C_ADDR);
Preferences preferences;
WebServer server(80);
//
//--------------------GLOBAL VARIABLES----------------------//
struct button buttonA = {0, 1500};
struct button buttonB = {0, 2000};
struct button buttonC = {0, 800};
bool blnLogoTimedOut = false; // flag to indicate the logo has timed out.
bool blnSetUpdateDisplayFlag = false; // flag to update the display values.
bool blnMetricUnit = false; // flag to indicate metric units for display.
bool blnDisplaySeaLevelPressure = false; // flag to display sea level pressure.
bool blnLogData = false; // flag to indicate data is being logged.
bool blnAppendDataToLog = false; // flag to indicate data is to be written (appended) to log file.
uint8_t dsplyMode = 0; // integer to indicate the display mode.
float updateInterval_sec = INTERVAL_WEATHER; // value in seconds to update the display values.
float currentHighValue_mA = 0.0; // value for INA219 Current Sensor.
float seaLevelPressure_hPa = 1013.25; // for BME280 sensor.
bool blnFoundOLED = false; // flag for result of begin statement of sensor.
bool blnFoundBME280 = false; // flag for result of begin statement of sensor.
bool blnFoundSI7021 = false; // flag for result of begin statement of sensor.
bool blnFoundINA219 = false; // flag for result of begin statement of sensor.
bool blnTouch1detected = false; // flag for Touch switch 1 detection.
bool blnFoundLittleFS = false; // flag for result of begin statement of file system.
//
struct tm timeinfo;
bool realTimeUpdate = false;
bool seaLevelPressureUpdate = false;
unsigned long lastMillis;
char timeStr[9]; // "HH:MM:SS" = 8 chars + 1 null terminator
//
//-------------------------Ticker---------------------------//
Ticker timerLogo;
Ticker timerRefreshDisplay;
Ticker timerRgbOnTime;
Ticker timerTouchHold;
Ticker timerAppendLog;
//
void setup() {
  unsigned long timeout = 5000; // Serial() timeout in milliseconds.
  unsigned long startMillis = millis(); // Record the start time for Serial() timeout.
  //
  // Initialze Ticker
  timerLogo.once(4, LogoTimedOut); // Run only once.
  timerRefreshDisplay.attach(2, SetUpdateDisplayFlag);
  //
  // Initialize OLED display.
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally.
  if (display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
    blnFoundOLED = true;
    //
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0, ROW_1); display.print(F("    Smith Huotari"));
    display.setCursor(0, ROW_2); display.print(F("     & Santa Fe"));
    display.setCursor(0, ROW_3); display.print(F("      My Meter"));
    display.display();
  }
  //
  // Initialize Serial().
  Serial.begin(115200);
  while (!Serial) {
    // Check if the timeout has been reached
    if (millis() - startMillis > timeout) {
        break; // Exit the loop after timeout
    }
    delay(10);
  }
  //
  Serial.println(F("SH&SF - My Meter"));
  Serial.println(F("Starting setup."));
  if (!blnFoundOLED) {
    Serial.println(F("SSD1306 display allocation failed"));
  } else {
    Serial.println(F("OLED Display Module online."));
  }
  //
  // Setup log file
  if (!SetupLittleFS()) {
    blnFoundLittleFS = false;
  } else {
    blnFoundLittleFS = true;
  }
  //
  // Setup sensors
  SetupSensors();
  //
  // Setup buttons
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);
  //
  // Setup outputs
  pinMode(LED_PIN, OUTPUT);
  //
  // Setup Touch switch
  // This method based on touchInterruptGetLastStatus() is only available for ESP32 S2 and S3
  touchAttachInterrupt(TOUCH_1, GotTouch1, THRESHOLD);
  //
  // Setup time and Sea Level Pressure
  GetWifiData();
  //
  // Web server routes
  server.on("/", handleRoot);
  server.on("/view", handleView);
  server.on("/download", handleDownload);
  server.on("/clear", handleClear);
  server.begin();
  //
  Serial.println(F("Setup is complete.\n"));
}

void loop() {
  if(blnLogoTimedOut) {
    //
    server.handleClient(); // Web server.
    //
    if (blnTouch1detected) {
      blnTouch1detected = false;
      if (touchInterruptGetLastStatus(TOUCH_1)) {
        timerTouchHold.once(2, ToggleLogDataFlag);
      } else {
        timerTouchHold.detach();
      }
    }
    //
    if(!digitalRead(BUTTON_A)) {
      if ((millis() - buttonA.previousMillis) >= buttonA.interval){
        switch (dsplyMode) {
          case WEATHER:
            blnMetricUnit = !blnMetricUnit;
            break;
          case CURRENT:
            currentHighValue_mA = 0;
            break;
          case WIFI:
            display.invertDisplay(true);
            display.display();
            if (WiFi.status() != WL_CONNECTED) {ConnectToWiFi();}
            break;
        }
        //
        DisplayValues();
        //
        buttonA.previousMillis = millis();
      }
    }
    else if(!digitalRead(BUTTON_B)) {
      if ((millis() - buttonB.previousMillis) >= buttonB.interval){
        switch (dsplyMode) {
          case WEATHER:
            blnDisplaySeaLevelPressure = !blnDisplaySeaLevelPressure;
            break;
          case CURRENT:
            if (updateInterval_sec == INTERVAL_SLOW_CURRENT) {
              ChangeUpdateInterval(INTERVAL_FAST_CURRENT); // value in seconds.
            } else {
              ChangeUpdateInterval(INTERVAL_SLOW_CURRENT); // value in seconds.
            }
            break;
          case WIFI:
            if (WiFi.status() == WL_CONNECTED) {DisconnectFromWiFi();}
            break;
        }
        //
        DisplayValues();
        //
        buttonB.previousMillis = millis();
      }
    }
    else if(!digitalRead(BUTTON_C)) {
      if ((millis() - buttonC.previousMillis) >= buttonC.interval){
        dsplyMode = (dsplyMode >= NUM_MODES) ? 0: (dsplyMode + 1);
        //
        DisplayValues();
        //
        buttonC.previousMillis = millis();
      }
    }
    //
    if (blnSetUpdateDisplayFlag) {
      blnSetUpdateDisplayFlag = false;
      DisplayValues();
    }
    //
    if (blnAppendDataToLog) {
      blnAppendDataToLog = false;
      AppendLogDataToFile();
    }
    //
    yield(); // allow other processes to run.
  }
}