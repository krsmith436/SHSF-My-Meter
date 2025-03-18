//
//
void dsplyValues(void) { // This is the main function.
  if (blnLogoTimedOut) {
    float cellVoltage = maxlipo.cellVoltage();
    if (isnan(cellVoltage)) {
      Serial.println("Failed to read cell voltage, check battery is connected!");
      return;
    }
    if (maxlipo.isActiveAlert()) {
      uint8_t alert_status = maxlipo.getAlertStatus();
      handleBatteryAlert(alert_status);
    }
    Serial.print(F("Batt Voltage: ")); Serial.print(cellVoltage, 3); Serial.println(F(" V"));
    Serial.print(F("Batt Percent: ")); Serial.print(maxlipo.cellPercent(), 1); Serial.println(F(" %"));
    Serial.print(F("Batt ChgRate: ")); Serial.print(maxlipo.chargeRate(), 1); Serial.println(F(" %/hr"));
    Serial.println();
    //
    Serial.print(F("Temperature = ")); Serial.print(bme.readTemperature()); Serial.println(F(" °C"));
    Serial.print(F("Pressure = ")); Serial.print(bme.readPressure() / 100.0F); Serial.println(F(" hPa"));
    Serial.print(F("Approx. Altitude = ")); Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA)); Serial.println(F(" m"));
    Serial.print(F("Humidity = ")); Serial.print(bme.readHumidity()); Serial.println(F(" %"));
    Serial.println();
    //
    // show data on OLED
    display.clearDisplay();
    switch (dsplyMode) {
      case WEATHER:
        display.setCursor(0, ROW_1); display.print((blnMetricUnit) ? bme.readTemperature() : ((bme.readTemperature() * 1.8) + 32), 1); display.print((blnMetricUnit) ? " C" : " F");
        display.setCursor(65, ROW_1); display.print(bme.readPressure() / 100.0F); display.print(F(" hPa"));
        display.setCursor(0, ROW_2); display.print(bme.readHumidity(), 1); display.print(F(" %"));
        display.setCursor(65, ROW_2); display.print(bme.readAltitude(SEALEVELPRESSURE_HPA)); display.print(F(" m"));
        display.setCursor(0, ROW_3); display.print(F("Batt: ")); display.print(cellVoltage, 2); display.print(F(" V"));
        display.setCursor(86, ROW_3); display.print(maxlipo.cellPercent(), 1); display.print(F(" %"));
        break;
      case BATTERY:
        display.setCursor(0, ROW_1); display.print(F("ChgRate: ")); display.print(maxlipo.chargeRate(), 1); display.print(F(" %/hr"));
        display.setCursor(0, ROW_3); display.print(F("Batt: ")); display.print(cellVoltage, 2); display.print(F(" V"));
        display.setCursor(86, ROW_3); display.print(maxlipo.cellPercent(), 1); display.print(F(" %"));
        break;
      case CURRENT:
        display.setCursor(0, ROW_2); display.print(F("Under Construction"));
        break;
      default:
        dsplyMode = 0;
        break;
    }
    display.display();
  }
}
//
//
void handleBatteryAlert(uint8_t status_flags) {
  Serial.print(F("ALERT! flags = 0x"));
  Serial.println(status_flags, HEX);
  //
  if (status_flags & MAX1704X_ALERTFLAG_SOC_CHANGE) {
    Serial.print(F(", SOC Change"));
    maxlipo.clearAlertFlag(MAX1704X_ALERTFLAG_SOC_CHANGE); // clear the alert
  }
  if (status_flags & MAX1704X_ALERTFLAG_SOC_LOW) {
    Serial.print(F(", SOC Low"));
    maxlipo.clearAlertFlag(MAX1704X_ALERTFLAG_SOC_LOW); // clear the alert
  }
  if (status_flags & MAX1704X_ALERTFLAG_VOLTAGE_RESET) {
    Serial.print(F(", Voltage reset"));
    maxlipo.clearAlertFlag(MAX1704X_ALERTFLAG_VOLTAGE_RESET); // clear the alert
  }
  if (status_flags & MAX1704X_ALERTFLAG_VOLTAGE_LOW) {
    Serial.print(F(", Voltage low"));
    maxlipo.clearAlertFlag(MAX1704X_ALERTFLAG_VOLTAGE_LOW); // clear the alert
  }
  if (status_flags & MAX1704X_ALERTFLAG_VOLTAGE_HIGH) {
    Serial.print(F(", Voltage high"));
    maxlipo.clearAlertFlag(MAX1704X_ALERTFLAG_VOLTAGE_HIGH); // clear the alert
  }
  if (status_flags & MAX1704X_ALERTFLAG_RESET_INDICATOR) {
    Serial.print(F(", Reset Indicator"));
    maxlipo.clearAlertFlag(MAX1704X_ALERTFLAG_RESET_INDICATOR); // clear the alert
  }
  Serial.println();
}
//
//
void setupButtons(void) {
  buttonA.previousMillis = 0;
  buttonA.interval = 2000;
  buttonB.previousMillis = 0;
  buttonB.interval = 2000;
  buttonC.previousMillis = 0;
  buttonC.interval = 2000;
}//
//
void LogoTimedOut(void) {
  blnLogoTimedOut = true;
}