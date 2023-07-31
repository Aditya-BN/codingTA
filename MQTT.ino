void sendData(void *pvParameters)
{
  
  Serial.print(">>>> Void wifi jalan di: ");
Serial.println(xPortGetCoreID());
  //*********************** Wifi & MQTT ****************************
  mqttConnected = false;
  // setup_wifi();
  // wifiMulti.addAP(ssid_AP_1, password_AP_1);
  // wifiMulti.addAP(ssid_AP_2, password_AP_2);
  // wifiMulti.addAP(ssid_AP_3, password_AP_3);

  // Serial.println("Connecting Wifi...");
  // if (wifiMulti.run() == WL_CONNECTED)
  // {
  //   Serial.println("");
  //   Serial.println("WiFi connected");
  //   Serial.println("IP address: ");
  //   Serial.println(WiFi.localIP());
  // }

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  for (;;)
  {
    // mqttConnected = true;
    // MqttConnect();
    // client.loop();
    // Serial.println("Di sini jalan");
    if (!client.connected())
    {
        reconnect();
    }
    client.loop();
  }
}

// void MqttConnect()
// {
//   if (!client.connected())
//   {
//     if (millis() - lastReconnectAttempt > 5000)
//     {
//       lastReconnectAttempt = millis();

//       // Attempt to reconnect
//       if (reconnect())
//       {
//         lastReconnectAttempt = 0;
//       }
//     }
//   }
//   else
//   {
//     // Client connected
//     Serial.print("mqtt ok");
//     client.loop();
//   }
// }

void reconnect()
{
    // Loop until we're reconnected
    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (client.connect("ESP8266Client"))
        {
            Serial.println("connected");
            // Subscribe
            client.subscribe("esp32/output");
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 2 seconds");
            // Wait 5 seconds before retrying
            delay(2000);
        }
    }
}
// bool reconnect()
// {
//   String clientId = "Agrisoil_1";
//   if (client.connect(clientId.c_str()))
//   {
//     client.subscribe("agrisoil/fromserver");
//   }
//   return client.connected();
// }

// callback buat kalo ada pesan masuk dari mqtt nya
void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
}