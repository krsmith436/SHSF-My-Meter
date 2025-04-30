void DisplayValues(void) { // This is the main function.
  //
  display.clearDisplay();
  //
  switch (dsplyMode) {
    case WEATHER:
      rgbLedWrite(RGB_BUILTIN, 0, RGB_LED_BRIGHTNESS, 0);  // Green
      timerRgbOnTime.once(RGB_ON_TIME, RgbLedOff);
      if (updateInterval_sec != INTERVAL_WEATHER){
        ChangeUpdateInterval(INTERVAL_WEATHER); // value in seconds.
      }
      ReadWeatherSensor();
      break;
    case BATTERY:
      rgbLedWrite(RGB_BUILTIN, RGB_LED_BRIGHTNESS, 0, 0);  // Red
      timerRgbOnTime.once(RGB_ON_TIME, RgbLedOff);
      if (updateInterval_sec != INTERVAL_BATTERY){
        ChangeUpdateInterval(INTERVAL_BATTERY); // value in seconds.
      }
      ReadBatteryMonitor();
      break;
    case CURRENT:
      rgbLedWrite(RGB_BUILTIN, 0, 0, RGB_LED_BRIGHTNESS);  // Blue
      timerRgbOnTime.once(RGB_ON_TIME, RgbLedOff);
      if ((updateInterval_sec != INTERVAL_SLOW_CURRENT) && (updateInterval_sec != INTERVAL_FAST_CURRENT)){
        ChangeUpdateInterval(INTERVAL_SLOW_CURRENT); // value in seconds.
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
void DisplayTime(void) {
  time_t adjustedTime = 0;
  unsigned long elapsedMillis = millis() - lastMillis;
  //
  if (!realTimeUpdate) {
    adjustedTime = (elapsedMillis / 1000); // Adjust using millis(), starting at zero.
  } else {
    adjustedTime = mktime(&timeinfo) + (elapsedMillis / 1000); // Adjust using millis(), starting at real time.
  }
  struct tm *updatedTime = localtime(&adjustedTime);
  //
  if (!realTimeUpdate) {
    display.print(F("ZERO: "));
  } else {
    display.print(F("RTC: "));
  }
  display.printf("%02d:%02d:%02d", updatedTime->tm_hour, updatedTime->tm_min, updatedTime->tm_sec);
}
//
//
void ReadWeatherSensor(void) {
  float si7021Temperature = 0;
  float si7021Humidity = 0;
  //
  display.setCursor(COL_1, ROW_3);
  if(!blnDisplaySeaLevelPressure) {
    DisplayTime();
  } else {
    display.print(F("SLP: ")); display.print((blnMetricUnit) ? seaLevelPressure_hPa : (seaLevelPressure_hPa * 0.02953F), 2); display.print((blnMetricUnit) ? " hPa" : " inHg");
  }
  //
  // if (!blnFoundBME280) {
  //   Serial.println(F("No BME280 Sensor Data!"));
  //   //
  // } else {
  //   Serial.print(F("BME280 Temperature = ")); Serial.print(bme.readTemperature()); Serial.println(F(" °C"));
  //   Serial.print(F("BME280 Pressure = ")); Serial.print(bme.readPressure() / 100.0F); Serial.println(F(" hPa"));
  //   Serial.print(F("BME280 Approx. Altitude = ")); Serial.print(bme.readAltitude(seaLevelPressure_hPa)); Serial.println(F(" m"));
  //   Serial.println("***");
  // }
  display.setCursor(COL_2, ROW_1);
  if (!blnFoundBME280) {
    display.print(F("-----"));
  } else {
    display.print((seaLevelPressureUpdate) ? "" : "~"); display.print((blnMetricUnit) ? bme.readAltitude(seaLevelPressure_hPa) : (bme.readAltitude(seaLevelPressure_hPa) * 3.28), 0);
  }
  display.print((blnMetricUnit) ? " m" : " ft");
  //
  display.setCursor(COL_2, ROW_2);
  if (!blnFoundBME280) {
    display.print(F("-----"));
  } else {
    display.print((blnMetricUnit) ? bme.readPressure() / 100.0F : (bme.readPressure() * 0.0002953F), 2);
  }
  display.print((blnMetricUnit) ? " hPa" : " inHg");
  //
  if (!blnFoundSI7021) {
    // Serial.println(F("No Si7021 Sensor Data!"));
    //
  } else {
    si7021Temperature = si7021.readTemperature();
    si7021Humidity = si7021.readHumidity();
    //
    // Serial.print(F("Si7021 Temperature = ")); Serial.print((blnMetricUnit) ? si7021Temperature : ((si7021Temperature * 1.8F) + 32.0F), 1); Serial.println((blnMetricUnit) ? " C" : " F");
    // Serial.print(F("Si7021 Humidity = ")); Serial.print(si7021Humidity); Serial.println(F(" %"));
  }
  display.setCursor(COL_1, ROW_1);
  if (!blnFoundSI7021) {
    display.print(F("-----"));
  } else {
    display.print((blnMetricUnit) ? si7021Temperature : ((si7021Temperature * 1.8F) + 32.0F), 1);
  }
  display.print((blnMetricUnit) ? " C" : " F");
  //
  display.setCursor(COL_1, ROW_2);
  if (!blnFoundSI7021) {
    display.print(F("-----"));
  } else {
  display.print(si7021Humidity, 1);
  }
  display.print(F(" %"));
  //
  Serial.println();
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
    // Serial.println(F("No Current Sensor Data!"));
    display.setCursor(COL_1, ROW_2); display.print(F("INA219 not found!"));
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
    // Serial.print(F("Shunt Voltage [mV]: ")); Serial.println(shuntVoltage_mV);
    // Serial.print(F("Bus Voltage [V]: ")); Serial.println(busVoltage_V);
    // Serial.print(F("Load Voltage [V]: ")); Serial.println(loadVoltage_V);
    // Serial.print(F("Current[mA]: ")); Serial.println(current_mA);
    // Serial.print(F("Max. Current[mA]: ")); Serial.println(currentHighValue_mA);
    // Serial.print(F("Bus Power [mW]: ")); Serial.println(power_mW);
    // Serial.print(F("Update [Sec]: ")); Serial.println(updateInterval_sec, 3);
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
    display.setCursor(COL_1, ROW_1); display.print(busVoltage_V); display.print(" V");
    display.setCursor(80, ROW_1); display.print("OK = "); display.print((!ina219_overflow)?"Yes":"NO");
    display.setCursor(COL_1, ROW_2); display.print(current_mA); display.print(" mA");
    display.setCursor(80, ROW_2); display.print(updateInterval_sec * 1000.0F, 0); display.print(" ms");
    display.setCursor(COL_1, ROW_3); display.print(currentHighValue_mA); display.print(" max");
  }
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
    display.setCursor(COL_1, ROW_3); display.print(F("ALERT! flags = 0x")); display.print(alert_status, HEX);
  }
  // Serial.print(F("Batt Voltage: ")); Serial.print(cellVoltage, 3); Serial.println(F(" V"));
  // Serial.print(F("Batt Percent: ")); Serial.print(cellCharge, 1); Serial.println(F(" %"));
  // Serial.print(F("Batt ChgRate: ")); Serial.print(cellChargeRate, 1); Serial.println(F(" %/hr"));
  // Serial.print(F("Update Interval: ")); Serial.print(updateInterval_sec, 3); Serial.println(F(" sec"));
  // Serial.println();
  //
  //  show data on OLED
  display.setCursor(COL_1, ROW_1); display.print(F("Batt: ")); display.print(cellVoltage, 2); display.print(F(" V"));
  display.setCursor(80, ROW_1); display.print(F("@ ")); display.print(cellCharge, 0); display.print(F(" %"));
  display.setCursor(COL_1, ROW_2); display.print(F("ChgRate: ")); display.print(cellChargeRate, 1); display.print(F(" %/hr"));
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
void ChangeUpdateInterval(float newInterval) {
  timerRefreshDisplay.detach(); // Stop the current ticker
  updateInterval_sec = newInterval;
  timerRefreshDisplay.attach(updateInterval_sec, UpdateDisplay); // Reattach with new interval
}
//
//
void ToggleLogDataFlag(void) {
  blnLogData = !blnLogData;
  digitalWrite(LED_PIN, blnLogData);
}
//
//
void GotTouch1(void) {touch1detected = true;}
//
//
void RgbLedOff(void) {rgbLedWrite(RGB_BUILTIN, 0, 0, 0);}
//
//
void LogoTimedOut(void) {blnLogoTimedOut = true;}
//
//
void UpdateDisplay(void) {blnUpdateDisplay = true;}