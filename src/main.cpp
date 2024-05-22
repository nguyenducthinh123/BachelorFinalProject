#include <Arduino.h>
#include <Broker.h>
#include "../include/System/System.h"
#include "Md5.h"

System _system;
Log _log;

Broker broker;

const std::string exchange_key_topic = "d3101";
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
            
            broker.Send(md5::ToMd5(exchange_key_topic).c_str(), doc);
        }
    }
} startClock;

void setup() {
    Serial.begin(9600);
    _system.Reset();
    broker.Begin();
    broker.setCallback(CallBack);

    broker.Listen(md5::ToMd5(exchange_key_topic).c_str(), ReceivedCallback);
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

    String res = doc["response"].as<String>();
    if (res == "received") {
        isReceived = true;
        String handle_topic = "esp32/" + broker.GetId();
        //broker.Listen(handle_topic.c_str(), RequestCallback);
        broker.unsubscribe("c7e65e2086e66190ea95701621c34d3c");
        broker.subscribe(handle_topic.c_str());
        broker.SetAction(RequestCallback);
    }
}

void RequestCallback(JsonDocument doc) {
    String res = doc["response"].as<String>();
    Serial.println(res);
}
