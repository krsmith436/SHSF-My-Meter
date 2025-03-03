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
#include <Ticker.h> // for Ticker callbacks, which can call a function in a predetermined interval.
#include "SHSF-My-Meter.h"
//
//-------------------Object Instantiation-------------------//
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);
Adafruit_MAX17048 maxlipo;
Adafruit_BME280 bme; // I2C
//
//--------------------GLOBAL VARIABLES----------------------//
struct button buttonA;
struct button buttonB;
struct button buttonC;
bool blnLogoTimedOut = false; // flag for indicating the logo has timed out.
//
//-------------------------Ticker---------------------------//
Ticker timerLogo;
Ticker timerRefreshDisplay;
//
void setup() {
  unsigned long timeout = 5000; // Serial() timeout in milliseconds.
  unsigned long startMillis = millis(); // Record the start time
  bool blnOledFault = false;
  unsigned status;
  //
  // Initialize OLED display.
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally.
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
    blnOledFault = true;
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, ROW_1); display.print(F("    Smith Huotari"));
  display.setCursor(0, ROW_2); display.print(F("     & Santa Fe"));
  display.setCursor(0, ROW_3); display.print(F("      My Meter"));
  display.display();
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
  if (blnOledFault) {
    Serial.println(F("SSD1306 display allocation failed"));
  }
  else {
    Serial.println(F("OLED Display Module online."));
  }
  //
  // Initialize Battery Monitor.
  while (!maxlipo.begin()) {
    Serial.println(F("Couldnt find Adafruit MAX17048?\nMake sure a battery is plugged in!"));
    delay(2000);
  }
  Serial.print(F("MAX17048 Battery Monitor online with Chip ID: 0x"));
  Serial.println(maxlipo.getChipID(), HEX);
  //
  // Initialize BME280 Sensor.
  status = bme.begin();
  if (!status) {
    Serial.println(F("Could not find a valid BME280 sensor, check wiring, address, sensor ID!"));
    Serial.print(F("SensorID was: 0x")); Serial.println(bme.sensorID(),16);
    Serial.print(F("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n"));
    Serial.print(F("   ID of 0x56-0x58 represents a BMP 280,\n"));
    Serial.print(F("        ID of 0x60 represents a BME 280.\n"));
    Serial.print(F("        ID of 0x61 represents a BME 680.\n"));
  }
  //
  // Initialze Ticker
  timerLogo.once(4, LogoTimedOut); // Run only once.
  timerRefreshDisplay.attach(2, dsplyValues);
  //
  // Setup buttons
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);
  //
  Serial.print(F("Setup is complete.\n"));
}

void loop() {
  if(!digitalRead(BUTTON_A)) display.print("A");
  if(!digitalRead(BUTTON_B)) display.print("B");
  if(!digitalRead(BUTTON_C)) display.print("C");
  yield();
  display.display();
}