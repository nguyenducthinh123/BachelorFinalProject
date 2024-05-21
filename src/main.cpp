#include <Arduino.h>
#include <Broker.h>
#include "../include/System/System.h"
#include "Md5.h"

System _system;
Log _log;

Broker broker;

class AckClock : public Timer { 
public:

    AckClock() : Timer(5000) { } // chu kì bằng 5s

    void on_restart() override {
        JsonDocument doc;
        doc["_id"] = WiFi.macAddress();
        std::string topic = "d3101";
        std::string data_hex;
        md5 hash;
        hash.update(topic.begin(), topic.end());
        hash.hex_digest(data_hex);
        broker.Send(data_hex.c_str(), doc);
    }
} ackClock;

void Start(const char*, byte*, unsigned int);
void ControlLed(const char*, byte*, unsigned int);

int pinLeds[] = { 2, 4 };

void setup() {
    Serial.begin(9600);

    _system.Reset();

     for (int i = 0; i < 2; i++) { 
        pinMode(pinLeds[i], OUTPUT);
    }

    pinMode(5, INPUT);

    broker.Begin();

    // JsonDocument doc;
    // doc["_id"] = WiFi.macAddress();
    // doc["name"] = "Nguyễn Đức Thịnh";
    // broker.Send("esp32/start", doc);
    broker.Listen("esp32/start", Start);
}

void loop() {
    broker.loop();
    _system.Loop();
    if (digitalRead(5)) broker.Listen("esp32/control-led", ControlLed);
    delay(500);
}

void ControlLed(const char* topic, byte*  payload, unsigned int length) { 
   JsonDocument doc;
   deserializeJson(doc, payload, length);

   
    String led1 = doc["State"].as<String>();
    String id = doc["Id"].as<String>();

    //String led2 = doc["led2"].as<String>();

    if (led1 == "on") digitalWrite(2, HIGH);
    else digitalWrite(2, LOW);

    Serial.println(id);

    // if (led2 == "on") digitalWrite(4, HIGH);
    // else digitalWrite(4, LOW);
}

void Start(const char* topic, byte* payload, unsigned int length) {
    JsonDocument doc;
    deserializeJson(doc, payload, length);

    String _id = doc["_id"].as<String>();
    Serial.println(_id);
}

