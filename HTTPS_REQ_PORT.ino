const String httpsBasicAuth = "httpsUname:httpsPassword" + "@";
const String url = "https://" + httpsBasicAuth + "nodered.agrisoil.my.id/getngrokport";

uint16_t updatedPort()
{
  Serial.print("Fetching " + url + "... ");

  HTTPClient http;
  http.begin(url);

  int httpResponseCode = http.GET();
  
  if (httpResponseCode > 0) {
    Serial.print("HTTP ");
    Serial.println(httpResponseCode);

    String payload = http.getString();
    Serial.println();
    Serial.println(payload);

    return payload.toInt();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
    Serial.println(":-(");
  }
}