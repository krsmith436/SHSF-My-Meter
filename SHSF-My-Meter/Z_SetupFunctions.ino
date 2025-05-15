bool SetupLittleFS(void) {
  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS Mount Failed.");
    return false;
  }
  // Ensure log file exists
  if (!LittleFS.exists(LOG_FILE)) {
    File file = LittleFS.open(LOG_FILE, "w");
    if (file) {
      file.println(LOG_HEADER);
      file.close();
    } else {
      return false;
    }
  }
  Serial.println("LittleFS online.");
  return true;
}
//
//
void SetupSensors(void) {
  unsigned status;
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
  }
  else {
    blnFoundBME280 = true;
    // gaming settings for BME280 senesor.
    Serial.println(F("BME280 Sensor online, Gaming Scenario."));
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
    Serial.println(F("Si7021 Sensor online."));
    // Serial.print(F(" Rev("));
    // Serial.print(si7021.getRevision());
    // Serial.print(F(")"));
    // Serial.print(F(" Serial #")); Serial.print(si7021.sernum_a, HEX); Serial.println(si7021.sernum_b, HEX);
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
}