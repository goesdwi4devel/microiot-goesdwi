#include <Arduino.h>
#include <ArduinoJson.h>
#include <Wifi.h>
#include <HTTPClient.h>

const char *ssid = "Semeru";
const char *password = "@K4r4ng4s3M";

const size_t capacity = JSON_OBJECT_SIZE(5) + 70;
DynamicJsonDocument doc(capacity);

const char* json ="{\"device_id\":1,\"status\":\"ON\",\"temp\":\"26.5\",\"humid\":\"54.0\",\"mac\":\"1234567890\"}";

HTTPClient http;

String jsonOutput;

void connectToNetwork();
void telePrintChatId();
void teleSendMessage(String payload);


void setup() {
  Serial.begin(9600);

  // deserializeJson(doc, json);

  // int device_id = doc["device_id"]; // 1
  // const char *status = doc["status"]; // "ON"
  // const char *temp = doc["temp"]; // "26.5"
  // const char *humid = doc["humid"]; // "54.0"
  // const char *mac = doc["mac"]; // "204637284"

  // Serial.println("Device ID: " + String(device_id));
  // Serial.println("Device Status: " + String(status));
  // Serial.println("Suhu: " + String(temp) + " C");
  // Serial.println("Kelembaban: " + String(humid) + " %");
  // Serial.println("Mac Address: " + String(mac));

  // doc["device_id"] = 2; // 1
  // doc["status"] = "OFF"; // "ON"
  // doc["temp"] = "27.5"; // "26.5"
  // doc["humid"] = "50.0"; // "54.0"
  // doc["mac"] = "abcde123456"; // "204637284"

  // String jsonOutput;
  // serializeJson(doc, jsonOutput);
  // Serial.println("Hasil Serialize: " + jsonOutput);

  connectToNetwork();
  Serial.println(WiFi.macAddress());
  Serial.println(WiFi.localIP());
  delay(1000);

  // telePrintChatId();
  doc["chat_id"] = 1356472769;
  doc["text"] = "Hello World 1";
  serializeJson(doc, jsonOutput);
  teleSendMessage(jsonOutput);
}

void loop() {
  // put your main code here, to run repeatedly:
}

void connectToNetwork() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Establishing connection to WiFi..");
    Serial.println(ssid);
  }
  Serial.println("Connected to WiFi network");
}

void telePrintChatId() {
  http.begin("https://api.telegram.org/bot1383965879:AAEII9ZEdPWYiAyH57JRseQVjMYvZmDBcKM/getUpdates");
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    String response = http.getString(); //get http request response

    Serial.println("Response Code: " + String(httpResponseCode)); // print return code
    Serial.println("=== RESPONSE ===");
    Serial.println(response);
  } else {
    Serial.println("Error on sending GET: " + String(httpResponseCode));
  }

  http.end();
}


void teleSendMessage(String payload) {
  http.begin("https://api.telegram.org/bot1383965879:AAEII9ZEdPWYiAyH57JRseQVjMYvZmDBcKM/sendMessage");
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST(payload);

  if (httpResponseCode > 0) {
    String response = http.getString(); //get http request response

    Serial.println("Response Code: " + String(httpResponseCode)); // print return code
    Serial.println("=== RESPONSE ===");
    Serial.println(response);
  } else {
    Serial.println("Error on sending GET: " + String(httpResponseCode));
  }

  http.end();
}