#include <AuthClient.h>
#include <MicroGear.h>
#include <MQTTClient.h>
#include <SHA1.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <MicroGear.h>

const char* ssid     = "ESPERT-002";
const char* password = "espertap";

#define APPID   "HelloCMMC"
#define KEY     "BZYTbAa9ItnMyeW"
#define SECRET  "h4aeKNOFIlSatTS8ADNk3Ft3O"
#define ALIAS   "testled"
#define LEDPin 16

WiFiClient client;
AuthClient *authclient;

int timer = 0;
MicroGear microgear(client);

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
    Serial.println("connect...");
  }
  else
  {
    Serial.println("connection lost, reconnect...");
    microgear.connect(APPID);
  }
  delay(1000);

  //  if (microgear.connected()) {
  //    Serial.println("connected");
  //    microgear.loop();
  //    if (timer >= 1000) {
  //      Serial.println("Publish...");
  //      microgear.chat("mygear", "HelloCMMC");
  //      timer = 0;
  //    }
  //    else timer += 100;
  //  }
  //  else {
  //    Serial.println("connection lost, reconnect...");
  //    if (timer >= 5000) {
  //      microgear.connect(APPID);
  //      timer = 0;
  //    }
  //    else timer += 100;
  //  }
  //  delay(100);
}
