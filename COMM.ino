void setup_wifi() {
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  if (oled_available) {
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(3, 3);
    display.write("Connecting to ");
    display.println(ssid);
  }

  WiFi.mode(WIFI_STA);
  WiFi.config(staticIP, subnet, gateway);
  WiFi.begin(ssid, password);

  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  //   Serial.print(" .");

  //   display.write(". ");
  //   display.display();
  // }

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  if (oled_available) {
    display.println("\n\nWiFi connected!");
    display.print("\nIP: ");
    display.println(WiFi.localIP());

    display.display();
  }
  delay(1000);
}
