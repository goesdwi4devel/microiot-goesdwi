#include <Arduino.h>
#include <ArduinoJson.h>
#include <Wifi.h>
#include <wire.h>
#include <SPI.h>
#include <DHT.h>
#include <PubSubClient.h>

typedef struct sensorData
{
  struct tm timeinfo;
  float temperature = 0;
  float humidity = 0;
} sensorData;

#define DHT_PIN 5
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);
float temperature = 0, humidity = 0;
String tempString, humidString;
sensorData dht11Data;

#define PUBLISH_INTERVAL 10000
unsigned long lastPublish = 0;

const char *ssid = "Semeru";
const char *password = "@K4r4ng4s3M";

String jsonOutput;

String mqttServer = "broker.hivemq.com";
String mqttPort = "1883";
String mqttUser = "";
String mqttPwd = "";
String deviceId = "2e88cebdd0f5455f9e7370db034b7111";
String pubTopic = String(deviceId) + "/sensor_data";
String subTopic = String(deviceId) + "/led_status";

WiFiClient ESPClient;
PubSubClient ESPMqtt(ESPClient);

void connectToNetwork();
void updateDhtData();
void connectToMqtt();
void mqttCallback(char *topic, byte *payload, long length);
void publishMessage();
void do_actions(const char *msg);

void setup()
{
  Serial.begin(9600);
  delay(1000);
  pinMode(BUILTIN_LED, OUTPUT);
  dht.begin();

  // disconnect previouse wifi connection
  WiFi.disconnect();
  ESPMqtt.setServer(mqttServer.c_str(), mqttPort.toInt());
  ESPMqtt.setCallback(mqttCallback);

  // connect to wifi network
  connectToNetwork();
  Serial.println(WiFi.macAddress());
  Serial.println(WiFi.localIP());
  delay(1000);

  // connecto to mqtt server
  connectToMqtt();
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED) {
    connectToNetwork();
  } else {
    if (!ESPMqtt.connected()) {
      connectToMqtt();
    }
  }

  if (millis() - lastPublish > PUBLISH_INTERVAL) {
    updateDhtData();
    Serial.print("Get Sensor Data: ");
    Serial.print("Temperature = ");
    Serial.print(temperature);
    Serial.print(" C & Humidity = ");
    Serial.print(humidity);
    Serial.println(" %");

    publishMessage();

    lastPublish = millis();
  }

  ESPMqtt.loop();
}

void connectToNetwork()
{
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Establishing connection to WiFi..");
    Serial.println(ssid);
  }
  Serial.println("Connected to WiFi network");
}

void updateDhtData()
{
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
}

void connectToMqtt()
{
  while (!ESPMqtt.connected())
  {
    Serial.println("ESP > Connecting to MQTT...");

    if (ESPMqtt.connect(deviceId.c_str(), mqttUser.c_str(), mqttPwd.c_str()))
    {
      Serial.println("Connected to Server");
      //subscribe to the topic
      ESPMqtt.subscribe(subTopic.c_str());
    }
    else
    {
      Serial.print("ERROR > failed with state");
      Serial.print(ESPMqtt.state());
      Serial.print("\r\n");
      delay(2000);
    }
  }
}

void publishMessage() {
  char msgToSend[1024] = {0};
  const size_t capacity = JSON_OBJECT_SIZE(4);
  DynamicJsonDocument doc(capacity);

  tempString = String(temperature);
  humidString = String(humidity);

  doc["eventName"] = "sensorStatus";
  doc["status"] = "none";
  doc["temp"] = tempString.c_str();
  doc["humid"] = humidString.c_str();

  serializeJson(doc, msgToSend);

  ESPMqtt.publish(pubTopic.c_str(), msgToSend);
}

void mqttCallback(char *topic, byte *payload, long length) {
  char msg[256];

  Serial.println("Message arrived [" + subTopic + "]");
  for (int i = 0; i < length; i++) {
    msg[i] = (char)payload[i];
  }
  do_actions(msg);
}


void do_actions(const char *message) {
  const size_t capacity = JSON_OBJECT_SIZE(2) + 30;
  DynamicJsonDocument doc(capacity);

  deserializeJson(doc, message);

  const char *deviceId = doc["deviceId"];
  const char *ledStatus = doc["ledStatus"];

  if (String(ledStatus) == "ON") {
    Serial.println("Turn On Led");
    digitalWrite(BUILTIN_LED, HIGH);
  } else if (String(ledStatus) == "OFF") {
    Serial.println("Turn Off Led");
    digitalWrite(BUILTIN_LED, LOW);
  } else {
    Serial.println("Could not recognize Led Status");
  }
}