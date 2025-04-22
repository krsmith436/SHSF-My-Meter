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
#include <WiFi.h> // for Wireless Fidelity (WiFi).
#include <HTTPClient.h> // for Sea Level Pressure.
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
//
//--------------------GLOBAL VARIABLES----------------------//
struct button buttonA;
struct button buttonB;
struct button buttonC;
bool blnLogoTimedOut = false; // flag to indicate the logo has timed out.
bool blnUpdateDisplay = false; // flag to update the display values.
bool blnMetricUnit = false; // flag to indicate metric units for display.
bool blnLogData = false; // flag to indicate data is beinf logged.
uint8_t dsplyMode = 0; // integer to indicate the display mode.
float updateInterval_sec = INTERVAL_WEATHER; // value in seconds to update the display values.
float currentHighValue_mA = 0.0; // value for INA219 Current Sensor.
float seaLevelPressure_hPa = 1013.25; // for BME280 sensor.
bool blnFoundOLED = false; // flag for result of begin statement of sensor.
bool blnFoundBME280 = false; // flag for result of begin statement of sensor.
bool blnFoundSI7021 = false; // flag for result of begin statement of sensor.
bool blnFoundINA219 = false; // flag for result of begin statement of sensor.
bool touch1detected = false; // flag for Touch switch 1 detection.
//
struct tm timeinfo;
bool realTimeUpdate = false;
bool seaLevelPressureUpdate = false;
unsigned long lastMillis;
//
//-------------------------Ticker---------------------------//
Ticker timerLogo;
Ticker timerRefreshDisplay;
Ticker timerRgbOnTime;
Ticker timerTouchHold;
//
void setup() {
  unsigned long timeout = 5000; // Serial() timeout in milliseconds.
  unsigned long startMillis = millis(); // Record the start time for Serial() timeout.
  //
  // Initialze Ticker
  timerLogo.once(4, LogoTimedOut); // Run only once.
  timerRefreshDisplay.attach(2, UpdateDisplay);
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
  }
  else {
    Serial.println(F("OLED Display Module online."));
  }
  //
  // Setup sensors
  setupSensors();
  //
  // Initialze Ticker
  timerLogo.once(4, LogoTimedOut); // Run only once.
  timerRefreshDisplay.attach(2, UpdateDisplay);
  //
  // Setup buttons
  SetupButtons();
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);
  //
  // Setup outputs
  pinMode(LED_PIN, OUTPUT);
  //
  // Setup Touch switch
  // This method based on touchInterruptGetLastStatus() is only available for ESP32 S2 and S3
  touchAttachInterrupt(TOUCH_1, gotTouch1, THRESHOLD);
  //
  // Setup time
  GetWifiData();
  //
  Serial.println(F("Setup is complete.\n"));
}

void loop() {
  if(blnLogoTimedOut) {
    //
    if (touch1detected) {
      touch1detected = false;
      if (touchInterruptGetLastStatus(TOUCH_1)) {
        timerTouchHold.once(2, ToggleLogDataFlag);
      } else {
        timerTouchHold.detach();
      }
    }
    //
    if(!digitalRead(BUTTON_A)) {
      if ((millis() - buttonA.previousMillis) >= buttonA.interval){
        if (dsplyMode == WEATHER) {
          blnMetricUnit = !blnMetricUnit;
        }
        if (dsplyMode == CURRENT) {
          currentHighValue_mA = 0;
        }
        //
        DisplayValues();
        //
        buttonA.previousMillis = millis();
      }
    }
    else if(!digitalRead(BUTTON_B)) {
      if ((millis() - buttonB.previousMillis) >= buttonB.interval){
        if (dsplyMode == CURRENT) {
          if (updateInterval_sec == INTERVAL_SLOW_CURRENT) {
            changeUpdateInterval(INTERVAL_FAST_CURRENT); // value in seconds.
          } else {
            changeUpdateInterval(INTERVAL_SLOW_CURRENT); // value in seconds.
          }
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
      // else if(touchRead(T10)) {

      // }
    }
    //
    if (blnUpdateDisplay) {
      DisplayValues();
      blnUpdateDisplay = false;
    }
    //
    yield(); // allow other processes to run.
  }
}