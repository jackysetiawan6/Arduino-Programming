#include <ArduinoJson.h>
#include <DHTesp.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <time.h>

#define TIME_ZONE +7 
#define DHTPIN D7
#define LED D4
#define LDR A0
#define AWS_IOT_TOPIC "sensor_group_03"
#define THINGNAME "iot_assignment"

unsigned long lastMillis = 0, previousMillis = 0;
const long interval = 5000;
const char WIFI_SSID[] = "Yang Pertama";
const char WIFI_PASSWORD[] = "MaafTapiItuBukanPunyaku";
const char MQTT_HOST[] = "a1kwmoq0xfo7wp-ats.iot.us-east-1.amazonaws.com";

static const char cacert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)EOF";

static const char client_cert[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
MIIDWjCCAkKgAwIBAgIVAJP9xabB/1ZonWzGePlqFuIU0tM3MA0GCSqGSIb3DQEB
CwUAME0xSzBJBgNVBAsMQkFtYXpvbiBXZWIgU2VydmljZXMgTz1BbWF6b24uY29t
IEluYy4gTD1TZWF0dGxlIFNUPVdhc2hpbmd0b24gQz1VUzAeFw0yNDEyMTUxNjE5
MTNaFw00OTEyMzEyMzU5NTlaMB4xHDAaBgNVBAMME0FXUyBJb1QgQ2VydGlmaWNh
dGUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCcSE9o9qQ4V7799pU1
wDYjk+R/+iEBvONuuyXzlnVh664uaHYiaQFKv94NaHCTV6p/swnCM8BHdiT506Km
P28+gtAGd7EwbYT4aMd8ximgP902D6EVvyHufhLlFFDNs6yThJ2yCp1k0n256FqQ
u7ESPbW0cQT2OmE2UPRXWmpV1oKvAK4X75+yoMzM/ysmtqHZU5RGGvgA1wXtdXD5
LMRl2TQtFPETdk3Vkug10ODeFFJ6qhVo+3bhzwz2MtBCk050/9f2MARvlxY0krhC
xcoWdaqzIDEbBT7zp/RJCmiHbOe+btAuTz9UWe6sIcQxcO6XMrKZO70RLTig9p0Y
batlAgMBAAGjYDBeMB8GA1UdIwQYMBaAFBvAi7vOkUlz/HAOEb9tn8mYVylXMB0G
A1UdDgQWBBQuY2pVP7ZD8c8iYjqn4u3qWglZBjAMBgNVHRMBAf8EAjAAMA4GA1Ud
DwEB/wQEAwIHgDANBgkqhkiG9w0BAQsFAAOCAQEAtyd5AA3U56SLcS4uUefM3IhT
o+KwFyEck4vMYvufQo9bvX3L16+3CYmTxedklpxJIduH1AdMK1hBGBU9Jq8aFbjK
r3a+pB2lorPC0bnRkFoy2Fr7U8JI875XMBu4WwlMS5ecagxvhsQS3XW6ZQVUkpR9
JZGFeG9TFEJsYZqD23yhhhJHzxC/gcADl0ijvFqTP9KjVgMaefosDzespbxW3LRm
e7T7oNVWLGAmhjW4LoGxLUcIJL32izJALBaVbNGuC+21gaLVTzVg0E4h42ICMzHV
FEDDIkDPprTB2udbHf+JYKVfI1nU4d21IL3fdRgRgTxQxUGjx0pdCBQwzqk/xg==
-----END CERTIFICATE-----
)KEY";

static const char privkey[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
MIIEowIBAAKCAQEAnEhPaPakOFe+/faVNcA2I5Pkf/ohAbzjbrsl85Z1YeuuLmh2
ImkBSr/eDWhwk1eqf7MJwjPAR3Yk+dOipj9vPoLQBnexMG2E+GjHfMYpoD/dNg+h
Fb8h7n4S5RRQzbOsk4SdsgqdZNJ9uehakLuxEj21tHEE9jphNlD0V1pqVdaCrwCu
F++fsqDMzP8rJrah2VOURhr4ANcF7XVw+SzEZdk0LRTxE3ZN1ZLoNdDg3hRSeqoV
aPt24c8M9jLQQpNOdP/X9jAEb5cWNJK4QsXKFnWqsyAxGwU+86f0SQpoh2znvm7Q
Lk8/VFnurCHEMXDulzKymTu9ES04oPadGG2rZQIDAQABAoIBAQCR8omOWX9FStSH
cYcoukcU9KFYzquSZubyWv33PJSEjMeOeWmRewSE3Seu19ECGMWWGgpL/W5cjj27
vWPQ476s3xcMYyzrL+wU3C0oHhtzlxCr0u65U1SJ190nZ9CKOXg6D4gBZB5WsKhP
PGmPbmjPwyd0ynM6y4xLYFC/XyWc4JBzgU/7RbqotuzbIJz4rEBJCS0GAe15fwUP
nyYPflBeo0yJCEwPTOiRrHWrlWBysQSZW2+NAQQi9yT2wu0c8CueqeQ6oGEf+cvo
8nYJ0pAalIb1Y6bzsOKvhahDXldUthS7FLoxpTd16yqi4TL4/ILmgy5buz8H0oVJ
31iWYnHlAoGBAMkaLMbGSD64Gi9bFAkziMD6ylWvbgce/EG7JyzEakH+waCDUzuy
X4gC5i1JNdqcrrBCDB19a6WhWhNei2bVVzWgOnD1P6g7Pg3aB8ITtTtHUpW1g167
zJUtHy5AD1Q2xYrgtqqqGEedQePFMupwTqDNWtdmv091iHenT48ymbULAoGBAMbx
81B8FMjSu37sQU5oYE5x5xO34Ps5e3AhztFbManp5NSQ9jPJAsNE3XZp+OhDtUkZ
z4KKYWEcZ3kigKRXjuwo1byU5QR3/x6eq7V4dTEyivjfzarGsBOPoR9gHDVZwPn0
Z6jxVXwjUS1lv3S1sBWmphTZYJajBQy/yxutUodPAoGATw6/M5WXqTBu+tbvVX6w
8EynZnAE7T+9I4oTqIG+1Br2u8dk3T8OV0XsfzkctsiJ1Dji0mUQAJkh9CNi4hZU
pnxK/UUstVI03vUrv5xHzvoG+VvWPVPO5YZLj78XR6AQoLwek5Ey4lRsJsLLdWaZ
QpX4dF2sL+ygrYlUuZLG4mkCgYAGgJXxRXzwa2LRbStdQrATIEnrLRFfZKuSKKi3
0wE2inx+Lha97o32j+OlISWPgdcFAqDzpU41fFYtcWE+/dWgBl2mAj/R+5INECaQ
kdVSvFIqfDYepMvQd9tOSTfizoIuKTzSGsEug4D9uNufFukgbAf1mn07ryxVFFex
GeMepQKBgAGKl71rgmFcOUXkN+BvTXdEgNqHwdgqNbkfDUsEgPQ6RTRhU0LEp7jV
vfe93WWfT0MK+vulushErfvVzqP1CgSBjtZjxnLvvwYitbpgjJuZGj+cPsRCQbXP
nC5oVSDG5TUVdXEN1VoTRRe6GqqkqsiRIrw0Be04ANacy2qWmXBh
-----END RSA PRIVATE KEY-----
)KEY";

DHTesp dht;
WiFiClientSecure espClient;
PubSubClient client(espClient);

time_t now;
time_t nowish = 1510592825;

BearSSL::X509List cert(cacert);
BearSSL::X509List client_crt(client_cert);
BearSSL::PrivateKey key(privkey);

void NTPConnect(void)
{
  Serial.print("Setting time using SNTP");
  configTime(TIME_ZONE * 3600, 0 * 3600, "pool.ntp.org", "time.nist.gov");
  now = time(nullptr);
  while (now < nowish) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("done!");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}

void messageReceived(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Received [");
  Serial.print(topic);
  Serial.print("]: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void connectAWS()
{
  delay(1000);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println(String("Attempting to connect to SSID: ") + String(WIFI_SSID));
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  NTPConnect();
  espClient.setTrustAnchors(&cert);
  espClient.setClientRSACert(&client_crt, &key);
  client.setServer(MQTT_HOST, 8883);
  client.setCallback(messageReceived);
  Serial.println("Connecting to AWS IOT");
  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(1000);
  }
  if (!client.connected()) {
    Serial.println("AWS IoT Timeout!");
    return;
  }
  client.subscribe(AWS_IOT_TOPIC);
  Serial.println("AWS IoT Connected!");
}

void publishMessage(int LDRValue, float temperature, float humidity)
{
  StaticJsonDocument<200> doc;
  doc["Light"] = LDRValue;
  doc["Temperature"] = String(temperature, 2) + "°C";
  doc["Humidity"] = String(humidity, 1) + "%";
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);
  client.publish(AWS_IOT_TOPIC, jsonBuffer);
}

void setup()
{
  pinMode(LED, OUTPUT);
  Serial.begin(9600);
  connectAWS();
  dht.setup(DHTPIN, DHTesp::DHT22);
}

void printStatus(int LDRValue, float temperature, float humidity) {
  Serial.print("LDR Value: ");
  Serial.print(LDRValue);
  Serial.print(" with Temperature: ");
  Serial.print(String(temperature, 2) + "°C");
  Serial.print("°C and Humidity: ");
  Serial.print(String(humidity, 1) + "%");
  Serial.println("%");
}

void setLamp(float temperature, int LDRValue) {
  if (LDRValue < 800) {
    digitalWrite(LED, HIGH);
  } else {
    digitalWrite(LED, LOW);
  }
}

void loop()
{
  TempAndHumidity data = dht.getTempAndHumidity();
  int LDRValue = analogRead(LDR);
  if (isnan(data.temperature) || isnan(data.humidity)) {
    Serial.println("Failed to read from DHT sensor! Check connections.");
    return;
  }
  float temperature = data.temperature;
  float humidity = data.humidity;
  printStatus(LDRValue, temperature, humidity);
  setLamp(temperature, LDRValue);
  delay(1000);
  now = time(nullptr);
  if (!client.connected()) {
    connectAWS();
  } else {
    client.loop();
    if (millis() - lastMillis > 1000) {
      lastMillis = millis();
      publishMessage(LDRValue, temperature, humidity);
    }
  }
}