#include <AuthClient.h>
#include <MicroGear.h>
#include <MQTTClient.h>
#include <SHA1.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <MicroGear.h>
#include "DHT.h"

const char* ssid     = "ESPERT-002";
const char* password = "espertap";

#define APPID   "HelloCMMC"
#define KEY     "BZYTbAa9ItnMyeW"
#define SECRET  "h4aeKNOFIlSatTS8ADNk3Ft3O"
#define ALIAS   "smartfarm"
#define LEDPin 16
#define DHTPIN 12
#define DHTTYPE DHT22

WiFiClient client;
AuthClient *authclient;

int timer = 0;
MicroGear microgear(client);

DHT dht(DHTPIN, DHTTYPE);

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
  //  microgear.subscribe("/mygear_slave");
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

  //=========== ช่วงประมวลผลคำสั่ง =============

  if (stateStr == "ON")
  {
    digitalWrite(LEDPin, LOW);
    //    microgear.chat("testledstatus", "ON");        //==== คำสั่ง chat เพื่อบอกส่งค่าสถานะไปยังหลอด LED บน NETPIE Freeboard
  }
  else if (stateStr == "OFF")
  {
    digitalWrite(LEDPin, HIGH);
    //    microgear.chat("testledstatus", "OFF");       //==== คำสั่ง chat เพื่อบอกส่งค่าสถานะไปยังหลอด LED บน NETPIE Freeboard
  }

  //=========== ช่วงประมวลผลคำสั่ง  =============
}

void setup() {
  /* Event listener */
  microgear.on(MESSAGE, onMsghandler);
  microgear.on(PRESENT, onFoundgear);
  microgear.on(ABSENT, onLostgear);
  microgear.on(CONNECTED, onConnected);

  Serial.begin(115200);
  Serial.println("Starting...");

  pinMode(LEDPin, OUTPUT);
  digitalWrite(16, LOW);

  dht.begin();

  if (WiFi.begin(ssid, password)) {

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
  }

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  //uncomment the line below if you want to reset token -->
  microgear.resetToken();
  microgear.init(KEY, SECRET, ALIAS);
  microgear.connect(APPID);
}

void loop() {
  if (microgear.connected())
  {
    microgear.loop();
 // อ่านค่าจากเซ็นเซอร์ DHt22
    float tempread = dht.readTemperature();
    float humidread = dht.readHumidity();
// ประกาศตัวแปรเพื่อเก็บค่าสำหรับส่งไปยัง netpie
    char temp[10];
    char humid[10];
// แยกจำนวนเลขทศนิยมออก เพื่อเก็บไว้ในอาเรย์
    int tempread_decimal = (tempread - (int)tempread) * 100;
    int humidread_decimal = (humidread - (int)humidread) * 100;
// บันทึกต่าในอาเรย์ในส่วนของจำนวนจริง และทศนิม
    sprintf(temp, "%d.%d", (int)tempread, tempread_decimal);
    sprintf(humid, "%d.%d", (int)humidread, humidread_decimal);
// ส่งค่าไปยัง netpie
    microgear.chat("smartfarm/Temperature", temp);
    microgear.chat("smartfarm/Humidity", humid);
    Serial.println("connect...");
    Serial.print("Temperature = ");
    Serial.print(tempread);
    Serial.print("\t");
    Serial.print("Humidity = ");
    Serial.println(humidread);
  }
  else
  {
    Serial.println("connection lost, reconnect...");
    microgear.connect(APPID);
  }
  delay(1000);
}
