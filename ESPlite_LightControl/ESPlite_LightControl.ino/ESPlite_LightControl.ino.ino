#include <Arduino.h>
#include <ArduinoJson.h>
#include <MicroGear.h>
#include <ESP8266WiFi.h>
#include <ESPert.h>
#include "DHT.h"

const char* ssid     = "@ESPertAP_001";  // Change your ssid wifi : ESPERT-002
const char* password = "espertap";  // Change your password wifi : espertap

// NETPIE.io : man_LightControl
#define APPID   "HelloCMMC"             // Change your appID
#define KEY     "BZYTbAa9ItnMyeW"       // Change your Key
#define SECRET  "h4aeKNOFIlSatTS8ADNk3Ft3O" // Change your SECRET
#define ALIAS   "smartuniversity"       // Change your name

#define LEDPin 16
#define DHTPIN 12
#define DHTTYPE DHT22


ESPert espert;
WiFiClient client;
MicroGear microgear(client);
DHT dht(DHTPIN, DHTTYPE);

void init_hardware();
void init_wifi();

/******************* microgear loop ***********************************/
void onFoundgear(char *attribute, uint8_t* msg, unsigned int msglen) {
  Serial.print("Found new member --> ");
  for (int i = 0; i < msglen; i++)
    Serial.print((char)msg[i]);
  Serial.println();
}

void onLostgear(char *attribute, uint8_t* msg, unsigned int msglen) {
  Serial.print("Lost member --> ");
  for (int i = 0; i < msglen; i++)
    Serial.print((char)msg[i]);
  Serial.println();
}

void onConnected(char *attribute, uint8_t* msg, unsigned int msglen) {
  Serial.println("Connected to NETPIE...");
  microgear.setName("mygear");
}

void onMsghandler(char *topic, uint8_t* msg, unsigned int msglen) {
  Serial.print("Incoming message --> ");
  Serial.print(topic);
  Serial.print(" : ");
  char strState[msglen];
  for (int i = 0; i < msglen; i++)
  {
    strState[i] = (char)msg[i];
    Serial.print((char)msg[i]);
  }

  Serial.println();

  String stateStr = String(strState).substring(0, msglen);

  Serial.print("Topic is ");
  Serial.println(topic);

  if (stateStr == "ON")
  {
    digitalWrite(LEDPin, LOW);
  }
  else if (stateStr == "OFF")
  {
    digitalWrite(LEDPin, HIGH);
  }
}

void init_hardware() {
  microgear.on(MESSAGE, onMsghandler);
  microgear.on(PRESENT, onFoundgear);
  microgear.on(ABSENT, onLostgear);
  microgear.on(CONNECTED, onConnected);

  Serial.begin(115200);
  espert.init();
  espert.oled.init();
  delay(2000);

  dht.begin();
  pinMode(LEDPin, OUTPUT);
  digitalWrite(LEDPin, LOW);
}

void init_wifi() {
  if (WiFi.begin(ssid, password)) {
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  microgear.resetToken();
  microgear.init(KEY, SECRET, ALIAS);
  microgear.connect(APPID);
}

/************************* loop ***********************************/
void setup() {
  init_hardware();
  init_wifi();
}

void loop() {
  if (microgear.connected())
  {
    microgear.loop();

    // อ่านค่าจากเซ็นเซอร์ DHt22
    float tempread = dht.readTemperature();
    float humidread = dht.readHumidity();

    // แปลงค่า float ให้เป็น String
    char buffer[50];
    String temp = dtostrf(tempread, 4, 2, buffer);
    String humid = dtostrf(humidread, 4, 2, buffer);

    // ส่งค่าไปยัง netpie
    microgear.chat("smartuniversity/Temperature", temp.c_str());
    microgear.chat("smartuniversity/Humidity", humid.c_str());

    espert.oled.clear();
    espert.oled.setTextSize(1);
    espert.oled.setTextColor(ESPERT_WHITE);
    espert.oled.setCursor(0, 24);

    espert.oled.println("Hello NETPIE");
    espert.oled.print("Temperature = ");
    espert.oled.println(dht.readTemperature());
    espert.oled.print("Humidity = ");
    espert.oled.println(dht.readHumidity());
    espert.oled.update();

    Serial.println("connect...");
    Serial.print("Temperature = ");
    Serial.print(tempread);
    Serial.print("\t");
    Serial.print("Humidity = ");
    Serial.println(humidread);
    delay(1000);
  }
  else
  {
    Serial.println("connection lost, reconnect...");
    microgear.connect(APPID);
  }
}
