#include <Arduino.h>
#include <Wire.h>
#include <w25q64.hpp>

#define BH1750_ADDRESS 0x23
#define BH1750_DATALEN 2

#define CS_PIN 5
#define SCK_PIN 18
#define MISO_PIN 19
#define MOSI_PIN 23
#define GET_CHIP_ID 0x9F
#define LEN_ID 4
#define BUTTON_PIN 15

byte chipId[4];
unsigned char writePage[256] = "";
unsigned char readPage[256] = "";
w25q64 spiChip;

#define LED1 2
#define LED2 4
#define LED3 16
#define LED4 17

void bh1750Request(int address);
int bh1750GetData(int address);
int ledList[4] = {LED1, LED2, LED3, LED4};

void ledOn(int ledNumber);
void ledOff();

byte buff[2];
unsigned short lux = 0;

void writeMem(String str2Write);
String readMem();
String autoBrightness;

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  Wire.begin();

  for (int i = 0; i < 4; i++)
  {
    pinMode(ledList[i], OUTPUT);
  }
  ledOff();

  spiChip.begin();
  spiChip.getId(chipId);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // baca chip id
  Serial.print("CHIP ID is: ");
  for (int i = 0; i < LEN_ID; i++)
  {
    Serial.print(chipId[i], HEX);
  }
  Serial.println();

  //membaca data dari chip w25q64
  Serial.println("Reading Auto Brightness status...");
  autoBrightness = readMem();
  if (autoBrightness == "1")
  {
    Serial.println("Auto Brightness is On ");
  } else
  {
    Serial.println("Auto Brightness is Off");
  }
}

void loop()
{
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
  if (autoBrightness == "1")
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
      ledOff();
    }
  }
  else //autobrightness inactive, nyalakan 4 led
  {
    ledOn(4);
  }
  delay(100);

  if (digitalRead(BUTTON_PIN) == LOW)
  {
    if (autoBrightness == "1")
    {
      autoBrightness = "0";
      Serial.println("Turning off auto brightness");
      writeMem(autoBrightness);
    }
    else
    {
      autoBrightness = "1";
      Serial.println("Turning on auto brightness");
      writeMem(autoBrightness);
    }
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
  ledOff();
  for (int i = 0; i < ledNumber; i++)
  {
    digitalWrite(ledList[i], HIGH);
  }
}

void ledOff()
{
  for (int i = 0; i < 4; i++)
  {
    digitalWrite(ledList[i], LOW);
  }
}

void writeMem(String str2Write)
{
  //Menulis data ke chip w25q64
  if (str2Write == "1")
  {
    memcpy(writePage, "1", sizeof("1"));
  }
  else
  {
    memcpy(writePage, "0", sizeof("0"));
  }
  spiChip.erasePageSector(0xFFFF);
  spiChip.pageWrite(writePage, 0xFFFF);
  delay(200);
}

String readMem()
{
  //membaca data dari chip w25q64
  String autoBrightness = "";

  spiChip.readPages(readPage, 0xFFFF, 1);
  for (int i = 0; i < 256; i++)
  {
    if (readPage[i] > 8 && readPage[i] < 127)
    {
      autoBrightness += (char)readPage[i];
    }
  }
  delay(200);
  return autoBrightness;
}