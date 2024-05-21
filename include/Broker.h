#pragma once
#include <PubSubClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>

//typedef void(*far_proc)(JsonDocument);

typedef void(*Callback)(const char*, byte*, unsigned int);

class Broker : public PubSubClient, public WiFiClient {
    const char* ssid;
    const char* password;
    
    WiFiClient espClient;
    const char* last_topic;    

public:
    Broker(const char* Ssid = "Wifi nha tro 4G", const char* Password = "43219876") : PubSubClient(espClient) {
        ssid = Ssid;
        password = Password;
    }

    void Begin() {
        // Connecting to a WiFi network
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.println("Connecting to WiFi..");
        }
        Serial.println("Connected to the Wi-Fi network");

        // Connecting to a mqtt broker
        PubSubClient::setServer("broker.emqx.io", 1883);
        
        while (!PubSubClient::connected()) {
            String client_id = "esp32-client-";
            client_id += (WiFi.macAddress());
            Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
            if (PubSubClient::connect(client_id.c_str())) {
                Serial.println("Public EMQX MQTT broker connected");
            } else {
                Serial.print("failed with state ");
                Serial.print(PubSubClient::state());
                delay(2000);
            }
         }
    }

    void Send(const char* topic, JsonDocument doc) {
        char buffer[256];
        serializeJson(doc, buffer);
        publish(topic, buffer);
    }

    void Listen(const char* topic, Callback received_callback) { 
        setCallback(received_callback);
        if (last_topic) {
            unsubscribe(last_topic);
        }
        last_topic = topic;
        if (topic) subscribe(topic);
    }

};