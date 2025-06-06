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
void handleLogRoot() {
  String html = "<h1><font color=green>Smith Huotari &amp; Santa Fe</font><br>"
                "<font color=orange>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;My Meter</font></h1>"
                "<h1>Log File</h1>"
                "<ul type=circle>"
                "<li><b><a href='/view'>View</a></b></li>"
                "<li><b><a href='/download'>Download</a></b></li>"
                "<li><b><a href='/clear'>Clear</a></b></li>"
                "</ul>";
  server.send(200, "text/html", html);
}
//
//
void handleCredentialsRoot() {
  String html = "<h1><font color=green>Smith Huotari &amp; Santa Fe</font><br>"
                "<font color=orange>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;My Meter</font></h1>"
                "<h1>WiFi Credentials</h1>"
                "<form action='/save' method='POST'>"
                "<b>SSID:</b><input name='ssid'><br>"
                "<b>Password:</b><input name='password' type='password'><br>"
                "<input type='submit'></form>";
  server.send(200, "text/html", html);
}
//
//
void handleSwitch() {
  currentPage = (currentPage == PAGE_CREDENTIALS) ? PAGE_LOG : PAGE_CREDENTIALS;
  server.sendHeader("Location", "/", true);  // redirect to root
  server.send(302, "text/plain", "");
}
//
//
void handleDynamicRoot() {
  if (currentPage == PAGE_LOG) {
    handleLogRoot(); // Log file form.
  } else {
    handleCredentialsRoot(); // WiFi credentials form.
  }
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
void handleSave() {
  String ssid = server.arg("ssid");
  String password = server.arg("password");
  //
  preferences.begin("wifi", false);
  preferences.putString("ssid", ssid);
  preferences.putString("password", password);
  preferences.end();
  //
  server.send(200, "text/html", "Saved! Rebooting...");
  delay(2000);
  ESP.restart();
}
//
// ======= Catch-all redirect =======
void handleRedirect() {
  server.sendHeader("Location", String("http://") + WiFi.softAPIP().toString(), true);
  server.send(302, "text/plain", "");
}
//
// ======= Set up AP + DNS + Web server =======
void setupAP() {
  //
  handleSwitch(); // Switch to Credentials form.
  //
  WiFi.mode(WIFI_AP);
  WiFi.softAP(apSSID);
  IPAddress IP = WiFi.softAPIP();
  Serial.print(F("Access Point IP: "));
  Serial.println(IP);
  //
  // DNS redirects all queries to ESP IP (captive portal trick)
  dns.start(DNS_PORT, "*", IP);
}
//
// ======= Try to connect to saved Wi-Fi =======
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
    Serial.print(F("."));
  }
  return false;
}
//
//
void DisconnectFromWiFi(void) {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
}