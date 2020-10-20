#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>

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

  // autobrightness active nyalakan led sesuai lux
  if (autoBrightness == 1)
  {
    if (lux <= 250)
    {
      ledOn(4);
    }
    else if (lux >= 250 && lux <= 500)
    {
      ledOn(3);
    }
    else if (lux >= 501 && lux <= 750)
    {
      ledOn(2);
    }
    else if (lux > 751 && lux <= 1000)
    {
      ledOn(1);
    }
    else
    {
      ledOn(0);
    }
  }
  else //autobrightness inactive, nyalakan 4 led
  {
    ledOn(4);
  }
  delay(100);
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