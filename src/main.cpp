#include <Arduino.h>
#include <Broker.h>
#include "../include/System/System.h"
#include "md5.h"

#define light1 2
#define light2 4
#define light3 5
#define light4 18
#define light5 27
#define door1 19
#define door2 23
#define fan1 12
#define fan2 13

int pinLights[] = { light1, light2, light3, light4, light5 };
int pinDoors[] = { door1, door2 };
int pinFans[] = { fan1, fan2 };
 
System _system;
Log _log;

Broker broker("Duc Thinh", "05082011");

String handle_topic = "d3101";
String token = ToMD5(handle_topic); // dùng token làm topic 
String building_topic = handle_topic.substring(0, 2);
String building_token = ToMD5(building_topic); // dùng làm topic lập lịch

JsonDocument RespDoc;

// khởi tạo RespDoc mặc định khi chưa có dữ liệu
void InitResponseDoc(JsonDocument& doc);

void CallBack(const char*, byte*, unsigned int);
void HandleCallback(JsonDocument);
void ScheduleCallback(JsonDocument);

class KeepAliveClock : public Timer {
public:
    KeepAliveClock() : Timer(60000) { } // 60s gửi keep alive 1 lần

    void on_restart() override {
        JsonDocument doc;
        doc["Type"] = "keep-alive";

        broker.Send(token, doc);
    }
} keepAliveClock;

void setup() {
    Serial.begin(9600);
    InitResponseDoc(RespDoc);
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

    broker.Listen(token);
    broker.Listen(building_token);
}

void loop() {
    broker.loop();
    _system.Loop();
}

void CallBack(const char* topic, byte* payload, unsigned int length) {
    JsonDocument doc;
    deserializeJson(doc, payload, length);

    if (strcmp(topic, building_token.c_str()) == 0) {
        ScheduleCallback(doc);
    }
    else {
        HandleCallback(doc);
    } 
}

// void ReceivedCallback(JsonDocument doc) {

//     String res = doc["Response"];
//     if (res == "received") {
//         isReceived = true;
//         // broker.Listen(handle_topic.c_str(), RequestCallback); // action bị null sau khi gọi lần 2 ?

//         broker.StopListen(exchange_key_topic);
//         broker.Listen(handle_topic);
//         broker.SetAction(RequestCallback);
//     }
// }

void HandleCallback(JsonDocument doc) {
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
    // if (type == "ack-control" || type == "response" || type == "keep-alive") return;
    
    if (type == "control") {
        JsonArray Lights = doc["Devices"]["Lights"];
        JsonArray Fans = doc["Devices"]["Fans"];
        JsonArray Doors = doc["Devices"]["Doors"];

        JsonArray CnLights = RespDoc["Devices"]["Lights"]; // ref nên thay đổi ở array sẽ thay đổi trong doc
        JsonArray CnFans = RespDoc["Devices"]["Fans"];
        JsonArray CnDoors = RespDoc["Devices"]["Doors"];

        for (JsonVariant items : Lights) {
            int id_esp = items["id_esp"];
            --id_esp;
            String status = items["status"];
            if (status == "on") {
                digitalWrite(pinLights[id_esp], HIGH);
                CnLights[id_esp]["status"] = "on";
            }
            else {
                digitalWrite(pinLights[id_esp], LOW);
                CnLights[id_esp]["status"] = "off";
            }
        }

        for (JsonVariant items : Fans) {
            int id_esp = items["id_esp"];
            --id_esp;
            String status = items["status"];
            if (status == "on") {
                digitalWrite(pinFans[id_esp], HIGH);
                CnFans[id_esp]["status"] = "on";
            }
            else {
                digitalWrite(pinFans[id_esp], LOW);
                CnFans[id_esp]["status"] = "off";
            }
        }

        for (JsonVariant items : Doors) {
            int id_esp = items["id_esp"];
            --id_esp;
            String status = items["status"];
            if (status == "on") {
                digitalWrite(pinDoors[id_esp], HIGH);
                CnDoors[id_esp]["status"] = "on";
            }
            else {
                digitalWrite(pinDoors[id_esp], LOW);
                CnDoors[id_esp]["status"] = "off";
            }
        }

        // send ack-control
        JsonDocument ack_doc;
        ack_doc["Type"] = "ack-control";
        ack_doc["Response"] = "control-success";
        broker.Send(token, ack_doc);
    }
    else if (type == "request-infor") {
        broker.Send(token, RespDoc);
    }
}

void ScheduleCallback(JsonDocument doc) {
    String type = doc["Type"];
    String _token = doc["Token"];
    if (type == "schedule" && _token == token) {
        JsonArray devices = doc["Devices"];    
        String status = doc["Status"];
        // đèn
        if (status == "on") {
            for (int i = 0; i < devices[0]; i++) {
                digitalWrite(pinLights[i], HIGH);
            }
        }
        else {
            for (int i = 0; i < devices[0]; i++) {
                digitalWrite(pinLights[i], LOW);
            }
        }
        // quạt
        if (status == "on") {
            for (int i = 0; i < devices[1]; i++) {
                digitalWrite(pinFans[i], HIGH);
            }
        }
        else {
            for (int i = 0; i < devices[1]; i++) {
                digitalWrite(pinFans[i], LOW);
            }
        }
        // cửa
        if (status == "on") {
            for (int i = 0; i < devices[2]; i++) {
                digitalWrite(pinDoors[i], HIGH);
            }
        }
        else {
            for (int i = 0; i < devices[2]; i++) {
                digitalWrite(pinDoors[i], LOW);
            }
        }

        // send ack-schedule
        JsonDocument ack_doc;
        ack_doc["Type"] = "ack-schedule";
        ack_doc["Token"] = token;
        broker.Send(building_token, ack_doc);
    }
}

void InitResponseDoc(JsonDocument& doc) {
    doc["Type"] = "response";

    JsonObject devices = doc.createNestedObject("Devices");

    JsonArray lights = devices.createNestedArray("Lights");
    int i = 0;
    for (auto& x : pinLights) {
        JsonObject light = lights.createNestedObject();
        light["id_esp"] = ++i;
        light["status"] = "off";
    }

    JsonArray fans = devices.createNestedArray("Fans");
    int j = 0;
    for (auto& x : pinFans) {
        JsonObject fan = fans.createNestedObject();
        fan["id_esp"] = ++j;
        fan["status"] = "off";
    }

    JsonArray doors = devices.createNestedArray("Doors");
    int k = 0;
    for (auto& x : pinDoors) {
        JsonObject door = doors.createNestedObject();
        door["id_esp"] = ++k;
        door["status"] = "off";
    }
}