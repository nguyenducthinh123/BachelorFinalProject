#include <Arduino.h>
#include <Broker.h>
#include "../include/System/System.h"
#include "md5.h"

#define light1 2
#define light2 4
#define light3 5
#define light4 18
#define door1 19
#define door2 23
#define fan1 12
#define fan2 13

int pinLights[] = { light1, light2, light3, light4 };
int pinDoors[] = { door1, door2 };
int pinFans[] = { fan1, fan2 };
 
System _system;
Log _log;

Broker broker("Tang 4 nha 37", "0912177195tqd");

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

class KeepAliveClock : public Timer {
public:
    KeepAliveClock() : Timer(60000) { } // 60s gửi keep alive 1 lần

    void on_restart() override {
        if (isReceived) {
            JsonDocument doc;
            doc["Type"] = "keep-alive";

            broker.Send(handle_topic, doc);
        }
    }
} keepAliveClock;

void setup() {
    Serial.begin(9600);
    _system.Reset();

    for (auto& x : pinLights) {
        pinMode(x, OUTPUT);
    }

    for (auto& x : pinDoors) {
        pinMode(x, OUTPUT);
    }

    for (auto& x : pinFans) {
        pinMode(x, OUTPUT);
    }
    
    broker.Begin();
    broker.setCallback(CallBack);

    broker.Listen(ToMD5(exchange_key_topic));
    broker.SetAction(ReceivedCallback);
}

void loop() {
    broker.loop();
    _system.Loop();
}

void CallBack(const char* topic, byte* payload, unsigned int length) {
    JsonDocument doc;
    deserializeJson(doc, payload, length);
    // int len = measureJson(doc);
    // _log << len << endl;
    broker.Call(doc);
}

void ReceivedCallback(JsonDocument doc) {

    String res = doc["Response"];
    if (res == "received") {
        isReceived = true;
        // broker.Listen(handle_topic.c_str(), RequestCallback); // action bị null sau khi gọi lần 2 ?

        broker.StopListen(exchange_key_topic);
        broker.Listen(handle_topic);
        broker.SetAction(RequestCallback);
    }
}

void RequestCallback(JsonDocument doc) {
    // JsonArray device = doc["Devices"];
    // if (!device) return;
    // int i = 0;
    // for (JsonVariant item : device) {
    //     if (item["Power"].as<String>() == "on") digitalWrite(pins[i], HIGH);
    //     else digitalWrite(pins[i], LOW);
    //     ++i;
    // }
    // JsonDocument doc_response;
    // doc_response["Response"] = "Success Control";
    // broker.StopListen(handle_topic);
    // broker.Send(handle_topic, doc_response);
    // broker.Listen(handle_topic);

    String type = doc["Type"];
    if (type == "ack-control" || type == "response" || type == "keep-alive") return;
    
    if (type == "control") {
        JsonArray Lights = doc["Devices"]["Lights"];
        JsonArray Fans = doc["Devices"]["Fans"];
        JsonArray Doors = doc["Devices"]["Doors"];

        for (JsonVariant items : Lights) {
            int id_esp = items["id_esp"];
            --id_esp;
            String power = items["power"];
            if (power == "on") {
                digitalWrite(pinLights[id_esp], HIGH);
            }
            else digitalWrite(pinLights[id_esp], LOW);
        }

        for (JsonVariant items : Fans) {
            int id_esp = items["id_esp"];
            --id_esp;
            String power = items["power"];
            if (power == "on") {
                digitalWrite(pinFans[id_esp], HIGH);
            }
            else digitalWrite(pinFans[id_esp], LOW);
        }

        for (JsonVariant items : Doors) {
            int id_esp = items["id_esp"];
            --id_esp;
            String power = items["power"];
            if (power == "on") {
                digitalWrite(pinDoors[id_esp], HIGH);
            }
            else digitalWrite(pinDoors[id_esp], LOW);
        }

        // send ack-control
        JsonDocument ack_doc;
        ack_doc["Type"] = "ack-control";
        ack_doc["Response"] = "control-success";
        broker.Send(handle_topic, ack_doc);
    }
}
