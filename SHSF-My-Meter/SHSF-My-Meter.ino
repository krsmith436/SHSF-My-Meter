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
bool blnLogoTimedOut = false; // flag for indicating the logo has timed out.
bool blnUpdateDisplay = false; // flag to update the display values.
bool blnMetricUnit = false; // flag for indicating metric units for display.
uint8_t dsplyMode = 0; // integer for display mode.
float updateIntervalSeconds = INTERVAL_WEATHER; // value in seconds to update the display values.
float currentHighValue_mA = 0.0; // value for INA219 Current Sensor.
bool blnFoundOLED = false; // flag for result of begin statement of sensor.
bool blnFoundBME280 = false; // flag for result of begin statement of sensor.
bool blnFoundSI7021 = false; // flag for result of begin statement of sensor.
bool blnFoundINA219 = false; // flag for result of begin statement of sensor.
//
const char *ssid = "Be_My_Guest";
const char *password = "MyArduinoNetwork";
const char *ntpServer1 = "pool.ntp.org";
const char *ntpServer2 = "time.nist.gov";
const long gmtOffset_sec = -18000;  // Adjust for your timezone
const int daylightOffset_sec = 3600;
const char *time_zone = "EST5EDT,M3.2.0,M11.1.0";  // TimeZone rule for America_Detroit
struct tm timeinfo;
bool realTimeUpdate = false;
unsigned long lastMillis;
//
//-------------------------Ticker---------------------------//
Ticker timerLogo;
Ticker timerRefreshDisplay;
Ticker timerRgbOnTime;
//
void setup() {
  unsigned long timeout = 5000; // Serial() timeout in milliseconds.
  unsigned long startMillis = millis(); // Record the start time for Serial() timeout.
  unsigned status;
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
  else {
    blnFoundBME280 = true;
    /*
    // default settings for BME280 senesor.
    Serial.println("-- Default Test --");
    Serial.println("normal mode, 16x oversampling for all, filter off,");
    Serial.println("0.5ms standby period");
    */
    // gaming settings for BME280 senesor.
    Serial.println(F("BME280 Sensor online, Gaming Scenario."));
    Serial.println(F("  normal mode, 4x pressure / 1x temperature / 0x humidity oversampling,"));
    Serial.println(F("  humidity off, 0.5ms standby period, filter 16x"));
    bme.setSampling(Adafruit_BME280::MODE_NORMAL,
                    Adafruit_BME280::SAMPLING_X1,   // temperature
                    Adafruit_BME280::SAMPLING_X4,   // pressure
                    Adafruit_BME280::SAMPLING_NONE, // humidity
                    Adafruit_BME280::FILTER_X16,
                    Adafruit_BME280::STANDBY_MS_0_5 );
                      
    // Suggested rate is 83Hz
    // 1 + (2 * T_ovs) + (2 * P_ovs + 0.5)
    // T_ovs = 1
    // P_ovs = 4
    // = 11.5ms + 0.5ms standby
    // delayTime = 12;
  }
  //
  // Initialize Si7021 Sensor.
  if (!si7021.begin()) {
    Serial.println(F("Si7021 sensor not found!"));
  } else {
    blnFoundSI7021 = true;
    //
    Serial.print(F("Found model "));
    switch(si7021.getModel()) {
      case SI_Engineering_Samples:
        Serial.print(F("SI engineering samples")); break;
      case SI_7013:
        Serial.print(F("Si7013")); break;
      case SI_7020:
        Serial.print(F("Si7020")); break;
      case SI_7021:
        Serial.print(F("Si7021")); break;
      case SI_UNKNOWN:
      default:
        Serial.print(F("Unknown"));
    }
    Serial.print(F(" Rev("));
    Serial.print(si7021.getRevision());
    Serial.print(F(")"));
    Serial.print(F(" Serial #")); Serial.print(si7021.sernum_a, HEX); Serial.println(si7021.sernum_b, HEX);
  }
  //
  // Initialize INA219 Current Sensor - Triggered Mode.
 if(!ina219.init()){
    Serial.println(F("INA219 Sensor not found!"));
  } else {
    blnFoundINA219 = true;
    //
    Serial.println(F("INA219 Sensor online."));
    /* Set ADC Mode for Bus and ShuntVoltage
    * Mode *            * Res / Samples *       * Conversion Time *
    BIT_MODE_9        9 Bit Resolution             84 µs
    BIT_MODE_10       10 Bit Resolution            148 µs  
    BIT_MODE_11       11 Bit Resolution            276 µs
    BIT_MODE_12       12 Bit Resolution            532 µs  (DEFAULT)
    SAMPLE_MODE_2     Mean Value 2 samples         1.06 ms
    SAMPLE_MODE_4     Mean Value 4 samples         2.13 ms
    SAMPLE_MODE_8     Mean Value 8 samples         4.26 ms
    SAMPLE_MODE_16    Mean Value 16 samples        8.51 ms     
    SAMPLE_MODE_32    Mean Value 32 samples        17.02 ms
    SAMPLE_MODE_64    Mean Value 64 samples        34.05 ms
    SAMPLE_MODE_128   Mean Value 128 samples       68.10 ms
    */
    // ina219.setADCMode(BIT_MODE_12); // choose mode and uncomment for change of default
    
    /* Set measure mode
    POWER_DOWN - INA219 switched off
    TRIGGERED  - measurement on demand
    ADC_OFF    - Analog/Digital Converter switched off
    CONTINUOUS  - Continuous measurements (DEFAULT)
    */
    ina219.setMeasureMode(TRIGGERED); // Triggered measurements for this example
    
    /* Set PGain
    * Gain *  * Shunt Voltage Range *   * Max Current *
    PG_40       40 mV                    0.4 A
    PG_80       80 mV                    0.8 A
    PG_160      160 mV                   1.6 A
    PG_320      320 mV                   3.2 A (DEFAULT)
    */
    ina219.setPGain(PG_160); // choose gain and uncomment for change of default
    
    /* Set Bus Voltage Range
    BRNG_16   -> 16 V
    BRNG_32   -> 32 V (DEFAULT)
    */
    ina219.setBusRange(BRNG_16); // choose range and uncomment for change of default

    /* If the current values delivered by the INA219 differ by a constant factor
      from values obtained with calibrated equipment you can define a correction factor.
      Correction factor = current delivered from calibrated equipment / current delivered by INA219
    */
    // ina219.setCorrectionFactor(0.98); // insert your correction factor if necessary

    /* If you experience a shunt voltage offset, that means you detect a shunt voltage which is not 
      zero, although the current should be zero, you can apply a correction. For this, uncomment the 
      following function and apply the offset you have detected.   
    */
    // ina219.setShuntVoltOffset_mV(0.0); // insert the shunt voltage (millivolts) you detect at zero current
  }
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
  updateTimeFromNTP();
  //
  Serial.println(F("Setup is complete.\n"));
}

void loop() {
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
        if (updateIntervalSeconds == INTERVAL_SLOW_CURRENT) {
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
  }
  //
  if (blnUpdateDisplay && blnLogoTimedOut) {
    DisplayValues();
    blnUpdateDisplay = false;
  }
  //
  yield(); // allow other processes to run.
}