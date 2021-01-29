#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include <SPI.h>
#include <DHT.h>
#include <BluetoothSerial.h>
#include <WiFi.h>

#define BH1750_ADDRESS 0x23
#define BH1750_DATALEN 2

#define LEN_ID 4
#define BUTTON_PIN 15

#define LED1 2
#define LED2 4
#define LED3 16
#define LED4 17

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
int lastTransmit = 0;

BluetoothSerial SerialBT;
String receivedString;
bool serialBTChgAutobrightStatus = false, serialBTChgAutobrightValue = false;

void updateDhtData();

char readDataAb[EEPROMSIZE_AB], readDataWf[EEPROMSIZE_WF], receiveData[EEPROMSIZE_WF];
int dataIndex = 0;

byte idxSsidPasswd = 0;
char *strings[129];
char *ssid;
char *password;
String defaultGatewayMode = "WiFi";

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

void setup()
{
  Serial.begin(9600);
  Wire.begin();

  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(LED1, PWM_CHANNEL);

  dht.begin();

  for (int i = 0; i < 4; i++)
  {
    pinMode(ledList[i], OUTPUT);
  }

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  attachInterrupt(BUTTON_PIN, &gpioISR, FALLING);

  EEPROM.begin(96);
  delay(100);
  autoBrightness = EEPROM.read(0);

  readEEPROM(32, readDataWf, EEPROMSIZE_WF);

  Serial.println("Reading Auto Brightness status...");
  if (autoBrightness == 1)
  {
    Serial.println("Auto Brightness is On ");
  }
  else
  {
    Serial.println("Auto Brightness is Off");
  }

  Serial.println("Check Gateway Mode");
  // split ssid passwd
  char *token = strtok(readDataWf, ";");
  while (token != NULL)
  {
    strings[idxSsidPasswd] = token;
    idxSsidPasswd++;
    token = strtok(NULL, ";");
  }
  ssid = strings[0];
  password = strings[1];

  if (String(ssid) == "")
  {
    defaultGatewayMode = "Bluetooth";
    Serial.println("Enter Bluetooth Mode");
    SerialBT.begin("ESP32-BTClassic");
  }
}

void loop()
{
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

  if (defaultGatewayMode == "WiFi" && WiFi.status() != WL_CONNECTED)
  {
    String sentence = "SSID: " + String(ssid);
    Serial.println(sentence);
    sentence = "Password: " + String(password);
    Serial.println(sentence);
    connectToNetwork();
  }
  else
  {
    char receivedChar = SerialBT.read();
    receivedString += receivedChar;

    if (receivedChar == '\n')
    {
      if (receivedString == "TEMP\r\n")
      {
        SerialBT.println(String(temperature));
      }
      else if (receivedString == "HUMID\r\n")
      {
        SerialBT.println(String(humidity));
      }
      else if (receivedString == "LUX\r\n")
      {
        SerialBT.println(String(lux));
      }
      else if (receivedString == "AUTOBRIGHT,ON\r\n")
      {
        SerialBT.println("ACK");
        serialBTChgAutobrightStatus = 1;
        serialBTChgAutobrightValue = 1;
      }
      else if (receivedString == "AUTOBRIGHT,OFF\r\n")
      {
        SerialBT.println("ACK");
        serialBTChgAutobrightStatus = 1;
        serialBTChgAutobrightValue = 0;
      }
      else
      {
        SerialBT.println("Could not recognize command");
      }
      receivedString = "";
      SerialBT.flush();
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

  if (millis() - lastTransmit > 5000)
  {
    updateDhtData();
    String sentence = "Data sensor -> Suhu: " + String(temperature) + "C dan kelembaban: " + String(humidity) + "%";
    Serial.println(sentence);
    lastTransmit = millis();
  }

  if (serialBTChgAutobrightStatus)
  {
    if (autoBrightness != serialBTChgAutobrightValue)
    {
      buttonPress = 1;
    }
    serialBTChgAutobrightStatus = 0;
  }

  if (buttonPress)
  {

    for (int i = 0; i < EEPROMSIZE_WF; i++)
    {
      receiveData[i] = 0;
    }

    Serial.println("Clearing SSID and Password");
    writeEEPROM(32, receiveData);
    memset(receiveData, 0, EEPROMSIZE_WF);
  }
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
  Serial.println("Establishing connection to WiFi..");
  Serial.println(ssid);
  delay(100);

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Connected to WiFi network");
    Serial.print("MAC Addresss: ");
    Serial.println(WiFi.macAddress());
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  }
}