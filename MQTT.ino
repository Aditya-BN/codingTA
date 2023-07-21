void MqttConnect()
{
  if (!client.connected())
  {
    if (millis() - lastReconnectAttempt > 5000)
    {
      lastReconnectAttempt = millis();

      // Attempt to reconnect
      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  }
  else
  {
    // Client connected
    client.loop();
  }
}

bool reconnect()
{
  // Create a random client ID
  String clientId = "Cooling tower";
  if (client.connect(clientId.c_str())) {
    client.subscribe("agrisoil/fromserver");
  }
  return client.connected();
}


// callback buat kalo ada pesan masuk dari mqtt nya
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
}