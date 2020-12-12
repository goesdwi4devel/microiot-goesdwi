#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include <SPI.h>
#include <DHT.h>
#include <BluetoothSerial.h>

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
char *ssidChr;
char *passwordChr;

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

  SerialBT.begin("ESP32-BTClassic");

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

  // split ssid passwd
  char *token = strtok(readDataWf, ";");
  while (token != NULL)
  {
    strings[idxSsidPasswd] = token;
    idxSsidPasswd++;
    token = strtok(NULL, ";");
  }
  ssidChr = strings[0];
  passwordChr = strings[1];

  String sentence = "SSID: " + String(ssidChr);
  Serial.println(sentence);
  sentence = "Password: " + String(passwordChr);
  Serial.println(sentence);
}

void loop()
{
  if (SerialBT.available())
  {
    char receivedChar = SerialBT.read();
    receivedString += receivedChar;

    if (receivedChar == '\n')
    {
      if (receivedString == "TEMP\r\n")
      {
        SerialBT.println(String(temperature));
      } else if (receivedString == "HUMID\r\n")
      {
        SerialBT.println(String(humidity));
      } else if (receivedString == "LUX\r\n")
      {
        SerialBT.println(String(lux));
      } else if (receivedString == "AUTOBRIGHT,ON\r\n")
      {
        SerialBT.println("ACK");
        serialBTChgAutobrightStatus = 1;
        serialBTChgAutobrightValue = 1;
      } else if (receivedString == "AUTOBRIGHT,OFF\r\n")
      {
        SerialBT.println("ACK");
        serialBTChgAutobrightStatus = 1;
        serialBTChgAutobrightValue = 0;
      } else
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
    } else
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