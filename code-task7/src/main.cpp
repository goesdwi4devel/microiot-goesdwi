#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <Wire.h>
#include <EEPROM.h>
#include <SPI.h>
#include <DHT.h>
#include <PubSubClient.h>

#define BH1750_ADDRESS 0x23
#define BH1750_DATALEN 2

#define LEN_ID 3
#define BUTTON_PIN 15

#define LED1 2
#define LED2 4
#define LED3 16
#define LED4 17

#define PUBLISH_INTERVAL 10000
unsigned long lastPublish = 0;

void bh1750Request(int address);
int bh1750GetData(int address);
int ledList[4] = {LED1, LED2, LED3, LED4};

#define EEPROMSIZE_AB 1
#define EEPROMSIZE_WF 64

#define PWM_CHANNEL 0
#define PWM_FREQ 5000
#define PWM_RESOLUTION 8
int dutyCyle = 0;

#define DHT_PIN 5
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);
float temperature = 0, humidity = 0;
String tempString, humidString, luxString;

void updateDhtData();

char readDataAb[EEPROMSIZE_AB], readDataWf[EEPROMSIZE_WF], receiveData[EEPROMSIZE_WF];
int dataIndex = 0;

byte idxSsidPasswd = 0;
char *strings[129];
char *ssid;
char *password;

String translateEncryptionType(wifi_auth_mode_t encryptionType);
void scanNetworks();
void connectToNetwork();

void writeEEPROM(int address, char *data);
void readEEPROM(int address, char *data, int dataSize);

void ledOn(int ledNumber);

byte buff[2];
unsigned short lux = 0;

int autoBrightness = 0;
bool buttonPress = false;

volatile bool interruptState = false;
int totalInterruptCounter = 0;

hw_timer_t *timer = NULL;
portMUX_TYPE gpioIntMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR gpioISR();

String jsonOutput;

String mqttServer = "broker.hivemq.com";
String mqttPort = "1883";
String mqttUser = "";
String mqttPwd = "";
String deviceIdEsp = "2e88cebdd0f5455f9e7370db034b7111";
String pubTopic = String(deviceIdEsp) + "/sensor_data";
String subTopic = String(deviceIdEsp) + "/led_status";

WiFiClient ESPClient;
PubSubClient ESPMqtt(ESPClient);

void connectToMqtt();
void mqttCallback(char *topic, byte *payload, long length);
void publishMessage();
void do_actions(const char *msg);

void setup()
{
  Serial.begin(9600);
  Wire.begin();

  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(LED2, PWM_CHANNEL);

  dht.begin();

  for (int i = 0; i < 4; i++)
  {
    pinMode(ledList[i], OUTPUT);
  }

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUILTIN_LED, OUTPUT);

  attachInterrupt(BUTTON_PIN, &gpioISR, FALLING);

  EEPROM.begin(96);
  delay(100);
  autoBrightness = EEPROM.read(0);

  readEEPROM(32, readDataWf, EEPROMSIZE_WF);

  Serial.println("\r\nReading Auto Brightness status...");
  if (autoBrightness == 1)
  {
    Serial.println("Auto Brightness is On ");
  }
  else
  {
    Serial.println("Auto Brightness is Off");
  }

  char *token = strtok(readDataWf, ";");
  while (token != NULL)
  {
    strings[idxSsidPasswd] = token;
    idxSsidPasswd++;
    token = strtok(NULL, ";");
  }
  ssid = strings[0];
  password = strings[1];

  // disconnect previous wifi connection
  WiFi.disconnect();

  ESPMqtt.setServer(mqttServer.c_str(), mqttPort.toInt());
  ESPMqtt.setCallback(mqttCallback);

  // connect to wifi network
  connectToNetwork();
  delay(1000);

  // connecto to mqtt server
  connectToMqtt();
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    connectToNetwork();
  }
  else
  {
    if (!ESPMqtt.connected())
    {
      connectToMqtt();
    }
  }

  if (Serial.available())
  {
    receiveData[dataIndex] = Serial.read();
    dataIndex++;

    if (receiveData[dataIndex - 1] == '\n')
    {
      String sentence = "Receive data from user: " + String(receiveData);
      Serial.println(sentence);
      writeEEPROM(32, receiveData);
      memset(receiveData, 0, EEPROMSIZE_WF);
      dataIndex = 0;
    }
  }

  bh1750Request(BH1750_ADDRESS);
  delay(200);

  // baca sensor intensitas cahaya
  if (bh1750GetData(BH1750_ADDRESS) == BH1750_DATALEN)
  {
    lux = (((unsigned short)buff[0] << 8 | (unsigned short)buff[1])) / 1.2;
    String sentence = "Nilai intensitas cahaya: " + String(lux) + " lux";
    Serial.println(sentence);
  }

  if (autoBrightness == 1)
  {
    if (lux >= 2500)
    {
      dutyCyle = 0;
    }
    else
    {
      dutyCyle = 255 - (lux / 10);
    }
    ledcWrite(PWM_CHANNEL, dutyCyle);
  }
  else
  {
    ledcWrite(PWM_CHANNEL, dutyCyle);
  }
  delay(100);

  if (millis() - lastPublish > 5000)
  {
    updateDhtData();
    String sentence = "Data sensor -> Suhu: " + String(temperature) + "C dan kelembaban: " + String(humidity) + "%";
    Serial.println(sentence);

    // publish meesage to mqtt broker
    publishMessage();

    lastPublish = millis();
  }

  if (buttonPress)
  {
    buttonPress = !buttonPress;
    autoBrightness = !autoBrightness;
    if (autoBrightness)
    {
      Serial.println("Turning on auto brightness");
    }
    else
    {
      Serial.println("Turning off auto brightness");
    }
    EEPROM.write(0, autoBrightness);
    EEPROM.commit();
  }

  ESPMqtt.loop();
}

void bh1750Request(int address)
{
  Wire.beginTransmission(address);
  Wire.write(0x10); //11x resolution 120ms
  Wire.endTransmission();
}

int bh1750GetData(int address)
{
  int i = 0;
  Wire.beginTransmission(address);
  Wire.requestFrom(address, 2);
  while (Wire.available())
  {
    buff[i] = Wire.read();
    i++;
  }
  Wire.endTransmission();

  return i;
}

void ledOn(int ledNumber)
{
  for (int i = 0; i < 4; i++)
  {
    digitalWrite(ledList[i], LOW);
  }

  for (int i = 0; i < ledNumber; i++)
  {
    digitalWrite(ledList[i], HIGH);
  }
}

void IRAM_ATTR gpioISR()
{
  portENTER_CRITICAL(&gpioIntMux);
  buttonPress = true;
  portEXIT_CRITICAL(&gpioIntMux);
}

void readEEPROM(int address, char *data, int dataSize)
{
  for (int i = 0; i < dataSize; i++)
  {
    data[i] = EEPROM.read(address + i);
  }
}

void writeEEPROM(int address, char *data)
{
  Serial.println("Start writing to EEPROM");
  for (int i = 0; i < EEPROMSIZE_WF; i++)
  {
    EEPROM.write(address + i, data[i]);
  }
  EEPROM.commit();
}

void updateDhtData()
{
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
}

String translateEncryptionType(wifi_auth_mode_t encryptionType)
{
  switch (encryptionType)
  {
  case (WIFI_AUTH_OPEN):
    return "Open";
    break;
  case (WIFI_AUTH_WEP):
    return "WEP";
    break;
  case (WIFI_AUTH_WPA_PSK):
    return "WPA_PSK";
    break;
  case (WIFI_AUTH_WPA2_PSK):
    return "WPA2_PSK";
    break;
  case (WIFI_AUTH_WPA_WPA2_PSK):
    return "WPA_WPA2_PSK";
    break;
  case (WIFI_AUTH_WPA2_ENTERPRISE):
    return "WPA2_ENTERPRISE";
    break;
  default:
    return "Invalid type of WiFi";
    break;
  }
}

void scanNetworks()
{
  int numOfNetworks = WiFi.scanNetworks();

  Serial.print("Number of Networks found: ");
  Serial.println(numOfNetworks);

  for (int i = 0; i < numOfNetworks; i++)
  {
    Serial.print("Network Name: ");
    Serial.println(WiFi.SSID(i));
    Serial.print("Signal Strength: ");
    Serial.println(WiFi.RSSI(i));
    Serial.print("MAC Address: ");
    Serial.println(WiFi.BSSIDstr(i));
    Serial.print("Encryption Type: ");
    String encryptionTypeDescription = translateEncryptionType(WiFi.encryptionType(i));
    Serial.println(encryptionTypeDescription);
    Serial.println("============================");
  }
}

void connectToNetwork()
{
  WiFi.begin(ssid, password);
  delay(500);
  Serial.print("Establishing connection to WiFi ");
  Serial.println(ssid);
  delay(100);

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.print("Connected to WiFi network");
    Serial.println(ssid);
    Serial.print("MAC Addresss: ");
    Serial.println(WiFi.macAddress());
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  }
}

void connectToMqtt()
{
  while (!ESPMqtt.connected())
  {
    Serial.println("ESP > Connecting to MQTT...");

    if (ESPMqtt.connect(deviceIdEsp.c_str(), mqttUser.c_str(), mqttPwd.c_str()))
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

void publishMessage()
{
  char msgToSend[1024] = {0};
  const size_t capacity = JSON_OBJECT_SIZE(5);
  DynamicJsonDocument doc(capacity);

  tempString = String(temperature);
  humidString = String(humidity);
  luxString = String(lux);

  doc["eventName"] = "sensorStatus";
  doc["status"] = "none";
  doc["temp"] = tempString.c_str();
  doc["humid"] = humidString.c_str();
  doc["lux"] = luxString.c_str();

  serializeJson(doc, msgToSend);

  ESPMqtt.publish(pubTopic.c_str(), msgToSend);
  Serial.print("MQTT Payload: ");
  Serial.println(msgToSend);
}

void mqttCallback(char *topic, byte *payload, long length)
{
  char msg[256];

  Serial.println("Message arrived [" + subTopic + "]");
  for (int i = 0; i < length; i++)
  {
    msg[i] = (char)payload[i];
  }
  do_actions(msg);
}

void do_actions(const char *message)
{
  const size_t capacity = JSON_OBJECT_SIZE(2) + 70;
  DynamicJsonDocument doc(capacity);

  deserializeJson(doc, message);

  const char *deviceId = doc["deviceId"];
  const char *ledStatus = doc["ledStatus"];

  if (String(deviceId) != deviceIdEsp)
  {
    Serial.println("Wrong Device ID");
  } else
  {
    if (String(ledStatus) == "ON")
    {
      Serial.println("Turn On Led");
      digitalWrite(BUILTIN_LED, HIGH);
    }
    else if (String(ledStatus) == "OFF")
    {
      Serial.println("Turn Off Led");
      digitalWrite(BUILTIN_LED, LOW);
    }
    else
    {
      Serial.println("Could not recognize Led Status");
    }
  }
}