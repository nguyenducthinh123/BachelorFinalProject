#pragma once
#include <PubSubClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>

#define pin_mqtt 26

typedef void(*Callback)(const char*, byte*, unsigned int);
typedef void(*Action)(JsonDocument);
const int BufferSize = (2 << 10); // 2 KB

class Broker : public PubSubClient, public WiFiClient {
    const char* ssid;
    const char* password;
    
    WiFiClient espClient;
    Action action;    

public:
    Broker(const char* Ssid = "Duc Thinh", const char* Password = "05082011") : PubSubClient(espClient) {
        ssid = Ssid;
        password = Password;
        setBufferSize(BufferSize);
    }

    void Begin() {
        ConnectWifi();

        PubSubClient::setServer("broker.emqx.io", 1883);
        ConnectMqtt();
    }

    void ConnectWifi() {
        // Connecting to a WiFi network
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.println("Connecting to WiFi..");
        }
        Serial.println("Connected to the Wi-Fi network");
    }

    void ConnectMqtt() {
        // Connecting to a mqtt broker
        while (!PubSubClient::connected()) {
            String client_id = "esp32-client-";
            client_id += (WiFi.macAddress());
            Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
            if (PubSubClient::connect(client_id.c_str())) {
                Serial.println("Public EMQX MQTT broker connected");
                digitalWrite(pin_mqtt, HIGH);
            } else {
                Serial.print("failed with state ");
                Serial.print(PubSubClient::state());
                delay(2000);
            }
         }
    }

    Broker& SetAction(Action action) {
        this->action = action;
        return *this;
    }

    void Send(String topic, JsonDocument doc) {
        Send(topic.c_str(), doc);
    }

    void Send(const char* topic, JsonDocument doc) {
        char buffer[BufferSize];
        serializeJson(doc, buffer);
        publish(topic, buffer);
    }

    void Listen(String topic) {
        subscribe(topic.c_str());
    }

    void StopListen(String topic) {
        unsubscribe(topic.c_str());
    }

    void Call(JsonDocument doc) {
        if (action) {
            action(doc);
        }
    }

    String GetId() {
       return WiFi.macAddress();
    }

};