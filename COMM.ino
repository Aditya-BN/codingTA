void setup_wifi()
{
  wifiMulti.addAP("ssid_AP_1", "password_AP_1");
  wifiMulti.addAP("ssid_AP_2", "password_AP_2");
  wifiMulti.addAP("ssid_AP_3", "password_AP_3");
  
#ifdef debug> 1
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
#endif

  if (oled_available)
  {
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(3, 3);
    display.write("Connecting to ");
    display.println(ssid);
  }

  // WiFi.mode(WIFI_STA);
  // WiFi.config(staticIP, subnet, gateway);
  // WiFi.begin(ssid, password);

  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  //   Serial.print(" .");

  //   display.write(". ");
  //   display.display();
  // }

  // while (WiFi.waitForConnectResult() != WL_CONNECTED) {
  //   Serial.println("Connection Failed! Rebooting...");
  //   delay(5000);
  //   ESP.restart();
  // }

#ifdef debug> 1
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
#endif

  if (oled_available)
  {
    display.println("\n\nWiFi connected!");
    display.print("\nIP: ");
    display.println(WiFi.localIP());

    display.display();
  }
  delay(1000);
}
