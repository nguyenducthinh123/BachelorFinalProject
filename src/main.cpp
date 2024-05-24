#include <Arduino.h>
#include <Broker.h>
#include "../include/System/System.h"
#include "md5.h"

System _system;
Log _log;

Broker broker("Mit Nam", "09082805");

const std::string exchange_key_topic = "d3101";
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
            
            broker.Send(exchange_key_topic.c_str(), doc);
        }
    }
} startClock;

void setup() {
    Serial.begin(9600);
    _system.Reset();
    broker.Begin();
    broker.setCallback(CallBack);

    broker.subscribe(exchange_key_topic.c_str());
    broker.SetAction(ReceivedCallback);
    pinMode(2, OUTPUT);
    Serial.println(ToMD5("D3101").c_str());
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

        broker.unsubscribe(exchange_key_topic.c_str());
        broker.subscribe(handle_topic.c_str());
        broker.SetAction(RequestCallback);
    }
}

void RequestCallback(JsonDocument doc) {
    digitalWrite(2, HIGH);
}
