#include <Arduino.h>
#include <ArduinoJson.h>

const size_t capacity = JSON_OBJECT_SIZE(5) + 70;
DynamicJsonDocument doc(capacity);

const char* json ="{\"device_id\":1,\"status\":\"ON\",\"temp\":\"26.5\",\"humid\":\"54.0\",\"mac\":\"1234567890\"}";


void setup() {
  Serial.begin(9600);

  deserializeJson(doc, json);

  int device_id = doc["device_id"]; // 1
  const char *status = doc["status"]; // "ON"
  const char *temp = doc["temp"]; // "26.5"
  const char *humid = doc["humid"]; // "54.0"
  const char *mac = doc["mac"]; // "204637284"

  Serial.println("Device ID: " + String(device_id));
  Serial.println("Device Status: " + String(status));
  Serial.println("Suhu: " + String(temp) + " C");
  Serial.println("Kelembaban: " + String(humid) + " %");
  Serial.println("Mac Address: " + String(mac));

  doc["device_id"] = 2; // 1
  doc["status"] = "OFF"; // "ON"
  doc["temp"] = "27.5"; // "26.5"
  doc["humid"] = "50.0"; // "54.0"
  doc["mac"] = "abcde123456"; // "204637284"

  String jsonOutput;
  serializeJson(doc, jsonOutput);
  Serial.println("Hasil Serialize: " + jsonOutput);
}

void loop() {
  // put your main code here, to run repeatedly:
}