#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#include <WiFi.h>
#include <WiFiUdp.h>
#include <WiFiMulti.h>
#include "./secret.h"
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <PubSubClient.h>
#include <HTTPClient.h>

#include <TimeLib.h>

#include <Wire.h>
#include <ADS1X15.h>
#include <TinyGPSPlus.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define debuglvl 2

void displayInfo();

ADS1115 ADS0(0x48);
ADS1115 ADS1(0x49);

//**********************WIFI**************************//
WiFiMulti wifiMulti;
WiFiClient espClient;

//************************* MQTT ****************************//
const char *mqtt_server = "tcp://0.tcp.ap.ngrok.io";
uint16_t mqtt_port = 15352;

PubSubClient client(espClient);
// const String Sen_ID = "Agrisoil_1";
long lastReconnectAttempt = 0;

void callback(char *topic, byte *payload, unsigned int length);

//************************** OLED ****************************//
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_ADDR 0x3C

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)

bool oled_available = false;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//************************** GPS ******************************//
// TaskHandle_t Task1;
TinyGPSPlus gps;

//*********************** SLEEPY BOY **************************//
#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 10          /* Time ESP32 will go to sleep (in seconds) */

RTC_DATA_ATTR int bootCount = 0;

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // disable brownout detector

  //********************** turn on relay **************************
  pinMode(18, OUTPUT);
  pinMode(19, OUTPUT);
  digitalWrite(18, HIGH);
  digitalWrite(19, HIGH);

  //*********************** Serial Comm ****************************
  Serial.begin(115200);

  //************************** OLED ************************************
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  { // Address 0x3D for 128x64
    Serial.println(F("Can't find OLED I2C"));
  }
  else
  {
    oled_available = true;
  }
  display.display();
  display.clearDisplay();

  display.setTextSize(2);         // set text size
  display.setTextColor(WHITE);    // set text color
  display.setCursor(2, 2);       // set position to display (x,y)
  display.println("Membaca \n sensor...."); // set text
  display.display();              // display on OLED

  //*********************** ADS ********************************
  ADS0.begin();
  ADS1.begin();

  ADS0.setGain(0);
  ADS1.setGain(0);

  int16_t npk_1ADC, npk_2ADC, moistADC, phADC;

  // Baca sekaligus dibuat rerata;
  for (int8_t i = 0; i < 10; i++)
  {
    npk_1ADC += ADS0.readADC(0);
    npk_2ADC += ADS0.readADC(1);

    moistADC += ADS1.readADC(0);
    phADC += ADS1.readADC(1);
    delay(500);
  }

  npk_1ADC = npk_1ADC / 10;
  npk_2ADC = npk_2ADC / 10;
  moistADC = moistADC / 10;
  phADC = phADC / 10;

  float vf0 = ADS0.toVoltage(1); // voltage factor
  float vf1 = ADS1.toVoltage(1); // voltage factor

  float npk_1Volt = npk_1ADC * vf0;
  float npk_2Volt = npk_2ADC * vf0;
  float moistVolt = moistADC * vf1;
  float phVolt = phADC * vf1;

  // code below there migth be looks like "magic"
  // cuz filled with goddamit magic number.
  // but it isn't fully true, cuz it came from calibration.
  // Raw value compared to a "real" value from real produt.
  float Nval = (-8.1731 * npk_1ADC) + 46510;
  Nval = Nval < 0 ? 0 : Nval;
  Nval = Nval > 700 ? 700 : Nval;

  float Pval = (-0.8921 * npk_1ADC) + 5112;
  Pval = Pval < 0 ? 0 : Pval;
  Pval = Pval > 30 ? 30 : Pval;

  float Kval = (21.2 * npk_1ADC) - 120331;
  Kval = Kval < 0 ? 0 : Kval;
  Kval = Kval > 170 ? 170 : Kval;

  // ADC says 4680 when submerged into water,
  // and 11940 when submerged into nothing.
  // so it must be (something - 4680)/(11940 - 4680)
  float moistval = (moistADC * 4680) / (11940 - 4680) * 100;

  // idk, just idk. I just lost 3 braincell tried to understand this.
  // https://depoinovasi.com/produk-975-sensor-ph-tanah-support-arduino.html
  float phval = (-0.0136 * phVolt) + 7.5773;


// delay(2000);
  display.clearDisplay();
  display.setTextSize(1);         // set text size
  display.setTextColor(WHITE);    // set text color
  display.setCursor(2, 2);       // set position to display (x,y)
  display.print("N: "); // set text
  display.println(Nval);
  display.print("P: "); // set text
  display.println(Pval);
  display.print("K: "); // set text
  display.println(Kval);
  display.print("Moist: "); // set text
  display.println(moistval);

  display.display();              // display on display

  //*********************** so wake me up when.... *******************

  // Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  // Print the wakeup reason for ESP32
  print_wakeup_reason();

  //*********************** Wifi & MQTT ****************************
  // setup_wifi();
  wifiMulti.addAP(ssid_AP_1, password_AP_1);
  wifiMulti.addAP(ssid_AP_2, password_AP_2);
  wifiMulti.addAP(ssid_AP_3, password_AP_3);

  Serial.println("Connecting Wifi...");
  if (wifiMulti.run() == WL_CONNECTED)
  {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  MqttConnect();


//************************ UPLOAD DATA ************************
    char moistString[8];
    dtostrf(moistval, 1, 2, moistString);
    char phString[8];
    dtostrf(phval, 1, 2, phString);
    char nitString[8];
    dtostrf(Nval, 1, 2, nitString);
    char phosString[8];
    dtostrf(Pval, 1, 2, phosString);
    char kalString[8];
    dtostrf(Kval, 1, 2, kalString);

    // Serial.print("Humidity: ");
    // Serial.println(humString);
    client.publish("agrisoil/val/moist", moistString);
    client.publish("agrisoil/val/ph", phString);
    client.publish("agrisoil/val/nit", nitString);
    client.publish("agrisoil/val/phos", phosString);
    client.publish("agrisoil/val/kal", kalString);


    // RAW DATA
    dtostrf(moistADC, 1, 2, moistString);
    dtostrf(phADC, 1, 2, phString);
    char npk_1String[8];
    dtostrf(npk_1ADC, 1, 2, npk_1String);
    char npk_2String[8];
    dtostrf(npk_2ADC, 1, 2, npk_2String);

    client.publish("agrisoil/raw/moist", moistString);
    client.publish("agrisoil/raw/ph", phString);
    client.publish("agrisoil/raw/npk1", npk_1String);
    client.publish("agrisoil/raw/npk2", npk_2String);

  //*********************** GPS *******************************
  

  uint16_t skrg = millis();
  while (millis() - skrg >= 5000)
  {
    while (Serial.available() > 0)
    if (gps.encode(Serial.read()))
      displayInfo();

    // if (gps.charsProcessed() < 10)
    // {
    //   Serial.println(F("No GPS detected: check wiring."));
    // }
  }

  //******************** this will makes u wakes up again.
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");

  digitalWrite(18, LOW);
  digitalWrite(19, LOW);

  //******************** Hidup tak segan, mati tak mau.
  //******************** mending turu aja
  display.setCursor(2, 50);       // set position to display (x,y)
  display.println("Hibernasi..."); // set text
  delay(1000);

  Serial.println("Going to sleep now");
  Serial.flush();

  esp_deep_sleep_start();
  // seriously, don't put any code below there,
  // anything below that poin will be sent to oblivion just like my.. nvm.
}

void loop()
{
  // put your main code here, to run repeatedly:
}

void displayInfo()
{
  Serial.print(F("Location: ")); 
  if (gps.location.isValid())
  {
    // Serial.print(gps.location.lat(), 6);
    // Serial.print(F(","));
    // Serial.print(gps.location.lng(), 6);
    char latString[8];
    dtostrf(gps.location.lat(), 9, 6, latString);
    char lngString[8];
    dtostrf(gps.location.lng(), 9, 6, lngString);

    client.publish("agrisoil/loc", latString);
    client.publish("agrisoil/loc", lngString);
  }
  else
  {
    Serial.print(F("INVALID"));
  }
}