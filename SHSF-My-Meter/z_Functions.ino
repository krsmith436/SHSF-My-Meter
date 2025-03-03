//
//
void dsplyValues(void) { // This is the main function.
  if (blnLogoTimedOut) {
    uint8_t rh = 13; // row height.
    uint8_t c1 = 7; // column 1 start, Cab.
    uint8_t c2 = 30; // column 2 start, Throttle. (3 chartacters = 33)
    uint8_t c3 = 75; // column 3 start, Direction.
    uint8_t c4 = 104; // column 4 start, Engine Facing.
    uint8_t bh = 19; // bar height, Throttle.
    uint8_t bw = 65; // bar width, Throttle.
    uint8_t bs = 128 - bw; // bar start, Throttle.
    uint8_t iw = 14; // icon width, Direction.
    uint8_t is = bs - iw - 3; // icon start, Direction.
    //
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
    display.setCursor(0, ROW_1); display.print(bme.readTemperature()); display.println(F(" C"));
    display.setCursor(65, ROW_1); display.print(bme.readPressure() / 100.0F); display.println(F(" hPa"));
    display.setCursor(0, ROW_2); display.print(bme.readHumidity()); display.println(F(" %"));
    display.setCursor(65, ROW_2); display.print(bme.readAltitude(SEALEVELPRESSURE_HPA)); display.println(F(" m"));
    display.setCursor(0, ROW_3); display.print(cellVoltage, 1); display.println(F(" V"));
    display.setCursor(65, ROW_3); display.print(maxlipo.cellPercent(), 1); display.print(F(" %"));
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
void LogoTimedOut(void) {
  blnLogoTimedOut = true;
}