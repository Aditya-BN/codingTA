void setup_wifi()
{
  wifiMulti.addAP("ssid_AP_1", "password_AP_1");
  // wifiMulti.addAP("ssid_AP_2", "password_AP_2");
  // wifiMulti.addAP("ssid_AP_3", "password_AP_3");

  Serial.println("=> Connecting Wifi...");

  if (wifiMulti.run() == WL_CONNECTED)
  {
#ifdef debuglvl > 1
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println(WiFi.SSID());
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
#endif
  }

  if (oled_available)
  {
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(3, 3);
    display.write("Connecting wifi ...");
  }

#ifdef debuglvl> 1
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
