void GetWifiData(void) {
  // 1) Get time from Network Time Protocol (NTP) server.
  // 2) Get sea level pressure from OpenWeatherMap API.
  // Connect Wi-Fi
  if (!ConnectToWiFi()) {
    Serial.println(F("\nWiFi NOT connected!"));
  } else {
    GetTime();
    GetSeaLevelPressure();
    //
    // Disconnect Wi-Fi
    DisconnectFromWiFi();
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
    Serial.println(F("Weather JSON received."));
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
//
//
void handleRoot() {
  String html = "<h2>ESP32 Log Server</h2>"
                "<a href='/view'>View Log</a><br>"
                "<a href='/download'>Download Log</a><br>"
                "<a href='/clear'>Clear Log</a><br>";
  server.send(200, "text/html", html);
}
//
//
void handleView() {
  File file = LittleFS.open(LOG_FILE, "r");
  if (!file) {
    server.send(500, "text/plain", "Failed to open log file");
    return;
  }
  //
  String content;
  while (file.available()) {
    content += file.readStringUntil('\n') + "<br>";
  }
  file.close();
  server.send(200, "text/html", "<h3>Log Contents:</h3>" + content);
}
//
//
void handleDownload() {
  File file = LittleFS.open(LOG_FILE, "r");
  if (!file) {
    server.send(500, "text/plain", "File not found");
    return;
  }
  //
  server.streamFile(file, "text/plain");
  file.close();
}
//
//
void handleClear() {
  LittleFS.remove(LOG_FILE);
  File file = LittleFS.open(LOG_FILE, "w");
  if (file) {
    file.println(LOG_HEADER);
    file.close();
  }
  server.send(200, "text/html", "Log cleared. <a href='/'>Go back</a>");
}
//
//
void DisconnectFromWiFi(void) {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
}
//
//
bool ConnectToWiFi(void) {
  // Read stored SSID and Password.
  preferences.begin("wifi", true); // Open Preferences with read-only
  String ssid = preferences.getString("ssid", "");
  String password = preferences.getString("password", "");
  preferences.end(); // Close Preferences
  //
  if (ssid.isEmpty() || password.isEmpty()) return false;
  //
  WiFi.begin(ssid.c_str(), password.c_str());
  Serial.print("Connecting to WiFi");
  for (int i = 0; i < 20; i++) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nWiFi connected, IP: " + WiFi.localIP().toString());
      return true;
    }
    delay(500);
    Serial.print(".");
  }
  return false;
}