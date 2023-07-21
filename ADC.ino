void ADS_request_all()
{
  if (ADS0.isConnected()) ADS0.requestADC(idx);
  if (ADS1.isConnected()) ADS1.requestADC(idx);
}

bool ADS_read_all()
{
  if (ADS0.isConnected() && ADS0.isBusy()) return true;
  if (ADS1.isConnected() && ADS1.isBusy()) return true;
  //  if (ADS2.isConnected() && ADS2.isBusy()) return true;
  //  if (ADS3.isConnected() && ADS3.isBusy()) return true;

  if (ADS0.isConnected()) val0[idx] = ADS0.getValue();
  if (ADS1.isConnected()) val1[idx] = ADS1.getValue();
  //  if (ADS2.isConnected()) val2[idx] = ADS2.getValue();
  //  if (ADS3.isConnected()) val3[idx] = ADS3.getValue();
  idx++;
  if (idx < 4)
  {
    ADS_request_all();
    return true;
  }
  idx = 0;
  return false;
}

void ADS_print_all()
{
  uint32_t now = millis();
  Serial.println(now - lastTime);
  lastTime = now;

  // PRINT ALL VALUES OF ADC0
  for (int i = 0; i < 4; i++)
  {
    Serial.print(val0[i]);
    Serial.print("\t");
  }
  // PRINT ALL VALUES OF ADC1
  for (int i = 0; i < 4; i++)
  {
    Serial.print(val1[i]);
    Serial.print("\t");
  }
  Serial.println();
}