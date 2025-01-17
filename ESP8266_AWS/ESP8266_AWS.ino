#include <ArduinoJson.h>
#include <DHTesp.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <time.h>
#include "Utilities.h"

#define DHTPIN D7
#define LEDPIN D4
#define LDRPIN A0
#define LDR_THRESHOLD 800

WiFiClientSecure espClient;
PubSubClient client(espClient);
DHTesp dht;

BearSSL::X509List caCert(cacert);
BearSSL::X509List clientCert(client_cert);
BearSSL::PrivateKey clientKey(privkey);

bool ledState = false;
int ledOverride = -1;
unsigned long lastPublish = 0;

void syncTime() {
    configTime(TIME_ZONE * 3600, 0, "pool.ntp.org", "time.nist.gov");
    Serial.println("Synchronizing time...");
    while (time(nullptr) < 100000) {
        Serial.print(".");
        delay(500);
    }
    Serial.println("\nTime synchronized.");
}

void setupWiFi() {
    Serial.print("Connecting to WiFi: ");
    Serial.println(WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    Serial.println("\nWiFi connected.");
}

void reconnect() {
    while (!client.connected()) {
        Serial.print("Connecting to AWS...");
        if (client.connect(AWS_THINGNAME)) {
            Serial.println("Connected.");
            client.subscribe(AWS_IOT_TOPIC_SUB);
        } else {
            Serial.print("Failed (rc=");
            Serial.print(client.state());
            Serial.println("). Retrying in 5 seconds...");
            delay(5000);
        }
    }
}

void handleMQTTMessage(char* topic, byte* payload, unsigned int length) {
    StaticJsonDocument<200> jsonDoc;
    if (deserializeJson(jsonDoc, payload, length) == DeserializationError::Ok) {
        if (String(topic) == AWS_IOT_TOPIC_SUB && jsonDoc.containsKey("LED_Override")) {
            ledOverride = jsonDoc["LED_Override"].as<int>();
        }
        Serial.print("Message received on topic ");
        Serial.println(topic);
    } else {
        Serial.println("Failed to parse incoming MQTT message.");
    }
}

void setupMQTT() {
    client.setServer(MQTT_HOST, MQTT_PORT);
    client.setCallback(handleMQTTMessage);
}

void publishData(float temperature, float humidity, int lightIntensity) {
    StaticJsonDocument<200> jsonDoc;
    jsonDoc["Temperature"] = temperature;
    jsonDoc["Humidity"] = humidity;
    jsonDoc["Light"] = lightIntensity;
    jsonDoc["LED_State"] = ledState;

    char payload[256];
    serializeJson(jsonDoc, payload);
    client.publish(AWS_IOT_TOPIC_PUB, payload);
    Serial.println("Data published: ");
    Serial.println(payload);
}

void setup() {
    Serial.begin(9600);
    dht.setup(DHTPIN, DHTesp::DHT22);
    pinMode(LEDPIN, OUTPUT);

    espClient.setTrustAnchors(&caCert);
    espClient.setClientRSACert(&clientCert, &clientKey);

    setupWiFi();
    syncTime();
    setupMQTT();
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    unsigned long now = millis();
    if (now - lastPublish > 1000) {
        lastPublish = now;

        float temperature = dht.getTemperature();
        float humidity = dht.getHumidity();
        int lightIntensity = analogRead(LDRPIN);

        ledState = (ledOverride == -1) ? (lightIntensity < LDR_THRESHOLD) : (ledOverride == 1);
        digitalWrite(LEDPIN, ledState ? HIGH : LOW);

        publishData(temperature, humidity, lightIntensity);
    }
}