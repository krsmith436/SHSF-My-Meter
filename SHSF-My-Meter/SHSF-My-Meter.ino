#include <Wire.h>
#include "Adafruit_MAX1704X.h"
#include <Adafruit_SSD1306.h> // for OLED display

#define OLED_I2C_ADDR 0x3C
#define OLED_WIDTH 128 // OLED display width, in pixels
#define OLED_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino reset pin)
//
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);

Adafruit_MAX17048 maxlipo;

void setup() {
  unsigned long timeout = 5000; // Timeout in milliseconds (5 seconds)
  unsigned long startMillis = millis(); // Record the start time
  Serial.begin(115200);
  while (!Serial) {
    // Check if the timeout has been reached
    if (millis() - startMillis > timeout) {
        Serial.println("Timeout reached, exiting loop.");
        break; // Exit the loop after timeout
    }
    //
    delay(10);    // wait until serial monitor opens
  }
  Serial.println(F("\nAdafruit MAX17048 simple demo"));

  while (!maxlipo.begin()) {
    Serial.println(F("Couldnt find Adafruit MAX17048?\nMake sure a battery is plugged in!"));
    delay(2000);
  }
  Serial.print(F("Found MAX17048"));
  Serial.print(F(" with Chip ID: 0x")); 
  Serial.println(maxlipo.getChipID(), HEX);
// initialize OLED display
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR))
  {
    Serial.println(F("SSD1306 display allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  Serial.println(F("OLED Display Module online."));
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.display();
//
}

void loop() {
  float cellVoltage = maxlipo.cellVoltage();
  if (isnan(cellVoltage)) {
    Serial.println("Failed to read cell voltage, check battery is connected!");
    delay(2000);
    return;
  }
  Serial.print(F("Batt Voltage: ")); Serial.print(cellVoltage, 3); Serial.println(" V");
  Serial.print(F("Batt Percent: ")); Serial.print(maxlipo.cellPercent(), 1); Serial.println(" %");
  Serial.print(F("Batt ChgRate: ")); Serial.print(maxlipo.chargeRate(), 1); Serial.println(" %/hr");
  Serial.println();
  //
  if (maxlipo.isActiveAlert()) {
    uint8_t status_flags = maxlipo.getAlertStatus();
    Serial.print(F("ALERT! flags = 0x"));
    Serial.print(status_flags, HEX);
    
    if (status_flags & MAX1704X_ALERTFLAG_SOC_CHANGE) {
      Serial.print(", SOC Change");
      maxlipo.clearAlertFlag(MAX1704X_ALERTFLAG_SOC_CHANGE); // clear the alert
    }
    if (status_flags & MAX1704X_ALERTFLAG_SOC_LOW) {
      Serial.print(", SOC Low");
      maxlipo.clearAlertFlag(MAX1704X_ALERTFLAG_SOC_LOW); // clear the alert
    }
    if (status_flags & MAX1704X_ALERTFLAG_VOLTAGE_RESET) {
      Serial.print(", Voltage reset");
      maxlipo.clearAlertFlag(MAX1704X_ALERTFLAG_VOLTAGE_RESET); // clear the alert
    }
    if (status_flags & MAX1704X_ALERTFLAG_VOLTAGE_LOW) {
      Serial.print(", Voltage low");
      maxlipo.clearAlertFlag(MAX1704X_ALERTFLAG_VOLTAGE_LOW); // clear the alert
    }
    if (status_flags & MAX1704X_ALERTFLAG_VOLTAGE_HIGH) {
      Serial.print(", Voltage high");
      maxlipo.clearAlertFlag(MAX1704X_ALERTFLAG_VOLTAGE_HIGH); // clear the alert
    }
    if (status_flags & MAX1704X_ALERTFLAG_RESET_INDICATOR) {
      Serial.print(", Reset Indicator");
      maxlipo.clearAlertFlag(MAX1704X_ALERTFLAG_RESET_INDICATOR); // clear the alert
    }
    Serial.println();
  }
  //
  // show data on OLED
  display.clearDisplay();

  display.setCursor(0, 0);
  display.print(cellVoltage, 3);
  display.print(" V");

  display.setCursor(0, 10);
  display.print(maxlipo.cellPercent(), 1);
  display.print(" %");

  display.setCursor(0, 20);
  display.print(maxlipo.chargeRate(), 1);
  display.print(" %/hr");

  display.display();
  //
  delay(2000);  // dont query too often!
}