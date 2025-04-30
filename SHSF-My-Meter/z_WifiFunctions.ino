void GetWifiData(void) {
  // 1) Get time from Network Time Protocol (NTP) server.
  // 2) Get sea level pressure from OpenWeatherMap API.
  unsigned long timeout = 5000; // timeout in milliseconds.
  unsigned long startMillis = millis(); // start time for timeout.
  bool connected = true;
  //
  // Read stored SSID and Password.
  preferences.begin("wifi", true); // Open Preferences with read-only
  String ssidString = preferences.getString("ssid", "");
  String passwordString = preferences.getString("password", "");
  preferences.end(); // Close Preferences
  const char* ssid = ssidString.c_str();
  const char* password = passwordString.c_str();
  //
  if (ssid != "" && password != "") {
    //
    WiFi.begin(ssid, password);
    Serial.print(F("Connecting to WiFi"));
    //
    while (WiFi.status() != WL_CONNECTED) {
      // Check if the timeout has been reached
      if (millis() - startMillis > timeout) {
          connected = false;
          break; // Exit the loop after timeout
      }
      delay(500);
      Serial.print(F("."));
    }
    if (!connected) {
      Serial.println(F("\nWiFi NOT connected!"));
    } else {
      Serial.println(F("\nWiFi connected."));
      //
      // Get time.
      GetTime();
      //
      // Get sea level pressure.
      GetSeaLevelPressure();
      //
      // Disconnect Wi-Fi
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
    }
  } else {
    Serial.println(F("No WiFi credentials stored!\n"));
  }
}
//
//
void GetTime(void) {
  //configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1);
  configTzTime(time_zone, ntpServer1);
  if (!getLocalTime(&timeinfo)) {
      realTimeUpdate = false;
  } else {
      realTimeUpdate = true;
  }
  lastMillis = millis(); // Save the reference time
}
//
//
void GetSeaLevelPressure(void) {
  HTTPClient http;
  String url = "http://api.openweathermap.org/data/2.5/weather?lat=" + String(lat) + "&lon=" + String(lon) + "&appid=" + apiKey + "&units=metric";
  //
  http.begin(url);
  int httpCode = http.GET();
  //
  if (httpCode > 0) {
    String payload = http.getString();
    Serial.println(F("Weather JSON received:"));
    //
    // Parse JSON
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, payload);
    //
    if (error) {
      Serial.print(F("JSON parsing failed: "));
      Serial.println(error.c_str());
      http.end();
      return;
    }
    seaLevelPressureUpdate = true;
    //
    seaLevelPressure_hPa = doc["main"]["sea_level"] | doc["main"]["pressure"];
    //
  } else {
    Serial.print(F("Error in HTTP request: "));
    Serial.println(httpCode);
  }
  //
  http.end();
}