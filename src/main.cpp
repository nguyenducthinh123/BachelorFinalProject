#include <Arduino.h>
#include <Broker.h>
#include "../include/System/System.h"
#include "md5.h"

System _system;
Log _log;

Broker broker;

String exchange_key_topic = "d3101";
String handle_topic = "esp32/" + broker.GetId();

bool isReceived = false;

void CallBack(const char*, byte*, unsigned int);
void ReceivedCallback(JsonDocument);
void RequestCallback(JsonDocument);

class StartClock : public Timer { 
public:

    StartClock() : Timer(3000) { } // cứ 3s gửi id 1 lần

    void on_restart() override {
        if (!isReceived) {
            JsonDocument doc;
            doc["_id"] = broker.GetId();
            
            broker.Send(ToMD5(exchange_key_topic), doc);
        }
    }
} startClock;

void setup() {
    Serial.begin(9600);
    _system.Reset();
    broker.Begin();
    broker.setCallback(CallBack);

    broker.Listen(ToMD5(exchange_key_topic));
    broker.SetAction(ReceivedCallback);
    pinMode(2, OUTPUT);
}

void loop() {
    broker.loop();
    _system.Loop();
}

void CallBack(const char* topic, byte* payload, unsigned int length) {
    JsonDocument doc;
    deserializeJson(doc, payload, length);
    broker.Call(doc);
}

void ReceivedCallback(JsonDocument doc) {

    String res = doc["Response"].as<String>();
    if (res == "received") {
        isReceived = true;
        // broker.Listen(handle_topic.c_str(), RequestCallback); // action bị null sau khi gọi lần 2 ?

        broker.StopListen(exchange_key_topic);
        broker.Listen(handle_topic);
        broker.SetAction(RequestCallback);
    }
}

void RequestCallback(JsonDocument doc) {
    digitalWrite(2, HIGH);
}
