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
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define debug 2

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

//*********************** SLEEPY BOY **************************//
#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 5           /* Time ESP32 will go to sleep (in seconds) */

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

  npk_1ADC = npk_1ADC/10;
  npk_2ADC = npk_2ADC/10;
  moistADC = moistADC/10;
  phADC = phADC/10;

  float vf0 = ADS0.toVoltage(1);  // voltage factor
  float vf1 = ADS1.toVoltage(1);  // voltage factor

  float npk_1Volt = npk_1ADC * vf0;
  float npk_2Volt = npk_2ADC * vf0;
  float moistVolt = moistADC * vf1;
  float phVolt = phADC * vf1;

  // code below there migth be looks like "magic"
  // cuz filled with goddamit magic number.
  // but it isn't fully true, cuz it came from calibration.
  // Raw value compared to a "real" value from real produt.
  float Nval = (-8.1731 * npk_1ADC) + 46510;
  float Pval = (-0.8921 * npk_1ADC) + 5112;
  float Kval = (21.2 * npk_1ADC) - 120331;

  // ADC says 4680 when submerged into water,
  // and 11940 when submerged into nothing.
  // so it must be (something - 4680)/(11940 - 4680)
  float moistval = (moistADC * 4680)/(11940 - 4680)*100;

  //idk, just idk. I just lost 3 braincell tried to understand this.
  //https://depoinovasi.com/produk-975-sensor-ph-tanah-support-arduino.html 
  float phval = (-0.0136 * phVolt) + 7.5773;


  //*********************** so wake me up when.... *******************

  // Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  // Print the wakeup reason for ESP32
  print_wakeup_reason();

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

  //******************** this will makes u wakes up again.
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");

  //******************** Hidup tak segan, mati tak mau.
  //******************** mending turu aja
  Serial.println("Going to sleep now");
  Serial.flush();

  esp_deep_sleep_start();
  // seriously, don't put any code below there,
  // anything below that poin will be sent to oblivion just like my hopes and dreams.
}

void loop()
{
  // put your main code here, to run repeatedly:
}