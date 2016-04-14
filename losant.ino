/**
 * Example for connecting a Particle Photon to the
 * Losant IoT Platform using MQTT.
 *
 * Copyright (c) 2016 Losant. All rights reserved.
 * https://www.losant.com
 */
 
#include "SparkJson/SparkJson.h"
#include "MQTT/MQTT.h"

#define LOSANT_BROKER "broker.losant.com"
#define LOSANT_DEVICE_ID "my-device-id"
#define LOSANT_ACCESS_KEY "my-access-key"
#define LOSANT_ACCESS_SECRET "my-access-secret"

// Topic used to subscribe to Losant commands.
String MQTT_TOPIC_COMMAND =
    String::format("losant/%s/command", LOSANT_DEVICE_ID);

// Topic used to publish state to Losant.
String MQTT_TOPIC_STATE =
    String::format("losant/%s/state", LOSANT_DEVICE_ID);

// The Photon's onboard LED.
int LED = D7;

// Callback signature for MQTT subscriptions.
void callback(char* topic, byte* payload, unsigned int length);

// MQTT client.
MQTT client(LOSANT_BROKER, 1883, callback);

// Toggles the LED on/off whenever "toggle" command is received.
bool ledValue = false;
void callback(char* topic, byte* payload, unsigned int length) {
    
    // Parse the command payload.
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& command = jsonBuffer.parseObject((char*)payload);

    Serial.println("Command received:");
    command.printTo(Serial);
    Serial.println();
    
    // If the command's name is "toggle", flip the LED.
    if(String(command["name"].asString()).equals(String("toggle"))) {
        ledValue = !ledValue;
        digitalWrite(LED, ledValue ? HIGH : LOW);
        Serial.println("Toggling LED");
    }
}

void setup() {
    Serial.begin(9600);
    while(!Serial) { }
    
    pinMode(LED, OUTPUT);
}

// Connects to the Losant MQTT broker.
void connect() {
    
    Serial.print("Connecting to Losant...");
    
    while(!client.isConnected()) {
        client.connect(
            LOSANT_DEVICE_ID,
            LOSANT_ACCESS_KEY,
            LOSANT_ACCESS_SECRET);
            
        if(client.isConnected()) {
            Serial.println("connected!");
            client.subscribe(MQTT_TOPIC_COMMAND);
        }
        else {
            Serial.print(".");
            delay(500);
        }
    }
}

// Used to only send temperature once a second.
int lastUpdate = millis();

void loop() {
    if (!client.isConnected()) {
        connect();
    }
    
    // Loop the MQTT client.
    client.loop();
        
    int now = millis();
    
    // Publish state every second.
    if(now - lastUpdate > 1000) {
        lastUpdate = now;
        
        // Build the json payload: { "data" : { "temp" : val }}
        StaticJsonBuffer<200> jsonBuffer;
        JsonObject& root = jsonBuffer.createObject();
        JsonObject& state = jsonBuffer.createObject();
        
        // TODO: refer to your specific temperature sensor
        // on how to convert the raw voltage to a temperature.
        int tempRaw = analogRead(A0);
        state["tempF"] = tempRaw;
        state["tempC"] = tempRaw;
        root["data"] = state;
        
        // Get JSON string.
        char buffer[200];
        root.printTo(buffer, sizeof(buffer));

        client.publish(MQTT_TOPIC_STATE, buffer);
    }
}
