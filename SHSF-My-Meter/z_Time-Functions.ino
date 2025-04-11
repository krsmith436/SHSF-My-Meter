void dsplyTime() {
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
    Serial.print(F("ZERO: "));
    display.print(F("ZERO: "));
  } else {
    Serial.print(F("RTC: "));
    display.print(F("RTC: "));
  }
  Serial.printf("%02d:%02d:%02d\n", updatedTime->tm_hour, updatedTime->tm_min, updatedTime->tm_sec);
  display.printf("%02d:%02d:%02d", updatedTime->tm_hour, updatedTime->tm_min, updatedTime->tm_sec);
}
//
//
void updateTimeFromNTP() {
  // Get time from Network Time Protocol (NTP) server
  unsigned long timeout = 5000; // timeout in milliseconds.
  unsigned long startMillis = millis(); // start time for timeout.
  bool connected = true;
  //
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  //
  while (WiFi.status() != WL_CONNECTED) {
    // Check if the timeout has been reached
    if (millis() - startMillis > timeout) {
        connected = false;
        break; // Exit the loop after timeout
    }
    delay(500);
    Serial.print(".");
  }
  if (!connected) {
    Serial.println("\nWiFi NOT connected!");
  } else {
    Serial.println("\nWiFi connected.");
    //
    // Get time from NTP
    Serial.println("Getting time from NTP.");
    //configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1);
    configTzTime(time_zone, ntpServer1);
    if (!getLocalTime(&timeinfo)) {
        realTimeUpdate = false;
        Serial.println("Failed to obtain time");
    } else {
        realTimeUpdate = true;
        Serial.println("Time updated from NTP.");
    }
    //
    // Disconnect Wi-Fi
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    if (!realTimeUpdate) {
        Serial.println("WiFi disconnected. Time starts at zero.");
    } else {
        Serial.println("WiFi disconnected. Running on internal RTC.");
    }
  }
  lastMillis = millis(); // Save the reference time
}