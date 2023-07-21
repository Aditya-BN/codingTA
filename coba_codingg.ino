#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <PubSubClient.h>
#include <HTTPClient.h>

#include <TimeLib.h>

#include <Wire.h>
#include "ADS1X15.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

ADS1115 ADS0(0x48);
ADS1115 ADS1(0x49);

int16_t val0[4] = {0, 0, 0, 0};
int16_t val1[4] = {0, 0, 0, 0};
int idx = 0;
uint32_t lastTime = 0;

//**********************WIFI**************************//
//**************hotspot laptop************************//
const char *ssid = "Base Camp X Tensi";
const char *password = "janganlupa1";
IPAddress staticIP(111, 11, 11, 32);
IPAddress gateway(11, 11, 11, 1);
IPAddress subnet(255, 255, 255, 0);

WiFiClient espClient;
PubSubClient client(espClient);
const String Sen_ID = "Agrisoil_1";
long lastReconnectAttempt = 0;

//************************* MQTT ****************************//
// const char* mqtt_server = "11.11.11.55";
const char *mqtt_server = "tcp://0.tcp.ap.ngrok.io";
uint16_t mqtt_port = 15352;

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
  Serial.println(ADS0.isConnected());
  Serial.println(ADS1.isConnected());

  ADS0.setDataRate(4); // 0 = slow   4 = medium   7 = fast but more noise
  ADS1.setDataRate(4);

  idx = 0;
  ADS_request_all();

  //*********************** wake up baby wake up *******************

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
  setup_wifi();
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