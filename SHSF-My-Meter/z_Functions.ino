void DisplayValues(void) { // This is the main function.
  //
  display.clearDisplay();
  //
  switch (dsplyMode) {
    case WEATHER:
      rgbLedWrite(RGB_BUILTIN, 0, RGB_LED_BRIGHTNESS, 0);  // Green
      timerRgbOnTime.once(RGB_ON_TIME, RgbLedOff);
      if (updateIntervalSeconds != INTERVAL_WEATHER){
        changeUpdateInterval(INTERVAL_WEATHER); // value in seconds.
      }
      ReadWeatherSensor();
      break;
    case BATTERY:
      rgbLedWrite(RGB_BUILTIN, RGB_LED_BRIGHTNESS, 0, 0);  // Red
      timerRgbOnTime.once(RGB_ON_TIME, RgbLedOff);
      if (updateIntervalSeconds != INTERVAL_BATTERY){
        changeUpdateInterval(INTERVAL_BATTERY); // value in seconds.
      }
      ReadBatteryMonitor();
      break;
    case CURRENT:
      rgbLedWrite(RGB_BUILTIN, 0, 0, RGB_LED_BRIGHTNESS);  // Blue
      timerRgbOnTime.once(RGB_ON_TIME, RgbLedOff);
      if ((updateIntervalSeconds != INTERVAL_SLOW_CURRENT) && (updateIntervalSeconds != INTERVAL_FAST_CURRENT)){
        changeUpdateInterval(INTERVAL_SLOW_CURRENT); // value in seconds.
      }
      ReadCurrentSensor();
      break;
    default:
      dsplyMode = 0;
      break;
  }
  display.display();
}
//
//
void changeUpdateInterval(float newInterval) {
  timerRefreshDisplay.detach(); // Stop the current ticker
  updateIntervalSeconds = newInterval;
  timerRefreshDisplay.attach(updateIntervalSeconds, UpdateDisplay); // Reattach with new interval
}
//
//
void ReadWeatherSensor(void) {
  float si7021Temperature = 0;
  float si7021Humidity = 0;
  //
  display.setCursor(0, ROW_3);
  dsplyTime();
  //
  if (!blnFoundBME280) {
    Serial.println(F("No BME280 Sensor Data!"));
    //
    display.setCursor(0, ROW_2); display.print(F("BME280 not found!"));
  } else {
    Serial.print(F("BME280 Temperature = ")); Serial.print(bme.readTemperature()); Serial.println(F(" °C"));
    Serial.print(F("BME280 Pressure = ")); Serial.print(bme.readPressure() / 100.0F); Serial.println(F(" hPa"));
    Serial.print(F("BME280 Approx. Altitude = ")); Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA)); Serial.println(F(" m"));
    Serial.println("***");
    //
    display.setCursor(0, ROW_2); display.print((blnMetricUnit) ? bme.readAltitude(SEALEVELPRESSURE_HPA) : (bme.readAltitude(SEALEVELPRESSURE_HPA) * 3.28), 1); display.print((blnMetricUnit) ? " m" : " ft");
    display.setCursor(65, ROW_2); display.print(bme.readPressure() / 100.0F); display.print(F(" hPa"));
  }
  if (!blnFoundSI7021) {
    Serial.println(F("No Si7021 Sensor Data!"));
    //
    display.setCursor(0, ROW_1); display.print(F("Si7021 not found!"));
  } else {
    si7021Temperature = si7021.readTemperature();
    si7021Humidity = si7021.readHumidity();
    //
    Serial.print(F("Si7021 Temperature = ")); Serial.print((blnMetricUnit) ? si7021Temperature : ((si7021Temperature * 1.8) + 32), 1); Serial.println((blnMetricUnit) ? " C" : " F");
    Serial.print(F("Si7021 Humidity = ")); Serial.print(si7021Humidity); Serial.println(F(" %"));
    //
    display.setCursor(0, ROW_1); display.print((blnMetricUnit) ? si7021Temperature : ((si7021Temperature * 1.8) + 32), 1); display.print((blnMetricUnit) ? " C" : " F");
    display.setCursor(65, ROW_1); display.print(si7021Humidity, 1); display.print(F(" %"));
  }
  Serial.print(F("Update Interval = ")); Serial.print(updateIntervalSeconds, 3); Serial.println(F(" sec"));
  Serial.println();
}
//
//
void ReadBatteryMonitor(void) {
  float cellVoltage = maxlipo.cellVoltage();
  float cellCharge = maxlipo.cellPercent();
  float cellChargeRate = maxlipo.chargeRate();
  //
  if (isnan(cellVoltage)) {
    Serial.println("Failed to read cell voltage, check battery is connected!");
    return;
  }
  if (maxlipo.isActiveAlert()) {
    uint8_t alert_status = maxlipo.getAlertStatus();
    HandleBatteryAlert(alert_status);
  }
  Serial.print(F("Batt Voltage: ")); Serial.print(cellVoltage, 3); Serial.println(F(" V"));
  Serial.print(F("Batt Percent: ")); Serial.print(cellCharge, 1); Serial.println(F(" %"));
  Serial.print(F("Batt ChgRate: ")); Serial.print(cellChargeRate, 1); Serial.println(F(" %/hr"));
  Serial.print(F("Update Interval: ")); Serial.print(updateIntervalSeconds, 3); Serial.println(F(" sec"));
  Serial.println();
  //
  //  show data on OLED
  display.setCursor(0, ROW_1); display.print(F("   Batt: ")); display.print(cellVoltage, 2); display.print(F(" V"));
  display.setCursor(0, ROW_2); display.print(F(" Charge: ")); display.print(cellCharge, 1); display.print(F(" %"));
  display.setCursor(0, ROW_3); display.print(F("ChgRate: ")); display.print(cellChargeRate, 1); display.print(F(" %/hr"));
}
//
//
void ReadCurrentSensor(void) {
  float shuntVoltage_mV = 0.0;
  float loadVoltage_V = 0.0;
  float busVoltage_V = 0.0;
  float current_mA = 0.0;
  float power_mW = 0.0; 
  bool ina219_overflow = false;
  //
  if (!blnFoundINA219) {
    Serial.println(F("No Current Sensor Data!"));
    display.setCursor(0, ROW_2); display.print(F("INA219 not found!"));
  } else {
    ina219.startSingleMeasurement(); // triggers single-shot measurement and waits until completed
    shuntVoltage_mV = ina219.getShuntVoltage_mV();
    busVoltage_V = ina219.getBusVoltage_V();
    current_mA = ina219.getCurrent_mA();
    if(abs(current_mA) > abs(currentHighValue_mA)) {
      currentHighValue_mA = current_mA;
    }
    power_mW = ina219.getBusPower();
    loadVoltage_V  = busVoltage_V - (shuntVoltage_mV/1000);
    ina219_overflow = ina219.getOverflow();
    //
    Serial.print(F("Shunt Voltage [mV]: ")); Serial.println(shuntVoltage_mV);
    Serial.print(F("Bus Voltage [V]: ")); Serial.println(busVoltage_V);
    Serial.print(F("Load Voltage [V]: ")); Serial.println(loadVoltage_V);
    Serial.print(F("Current[mA]: ")); Serial.println(current_mA);
    Serial.print(F("Max. Current[mA]: ")); Serial.println(currentHighValue_mA);
    Serial.print(F("Bus Power [mW]: ")); Serial.println(power_mW);
    Serial.print(F("Update [Sec]: ")); Serial.println(updateIntervalSeconds, 3);
    //
    if(!ina219_overflow){
      Serial.println(F("Values OK - no overflow"));
      display.invertDisplay(false);
    }
    else{
      Serial.println(F("Overflow! Choose higher PGAIN"));
      display.invertDisplay(true);
    }
    Serial.println();
    //
    // show data on OLED
    display.setCursor(0, ROW_1); display.print(busVoltage_V); display.print(" V");
    display.setCursor(80, ROW_1); display.print("OK = "); display.print((!ina219_overflow)?"Yes":"NO");
    display.setCursor(0, ROW_2); display.print(current_mA); display.print(" mA");
    display.setCursor(80, ROW_2); display.print(updateIntervalSeconds * 1000.0F, 0); display.print(" ms");
    display.setCursor(0, ROW_3); display.print(currentHighValue_mA); display.print(" max");
  }
}
// 
//
void HandleBatteryAlert(uint8_t status_flags) {
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
void SetupButtons(void) {
  buttonA.previousMillis = 0;
  buttonA.interval = 1500;
  buttonB.previousMillis = 0;
  buttonB.interval = 2000;
  buttonC.previousMillis = 0;
  buttonC.interval = 1000;
}
//
//
void RgbLedOff(void) {rgbLedWrite(RGB_BUILTIN, 0, 0, 0);}
//
//
void LogoTimedOut(void) {blnLogoTimedOut = true;}
//
//
void UpdateDisplay(void) {blnUpdateDisplay = true;}