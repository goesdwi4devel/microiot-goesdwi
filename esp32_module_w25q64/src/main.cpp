#include <Arduino.h>
// #include <SPI.h>
#include <w25q64.hpp>

#define CS_PIN 5
#define SCK_PIN 18
#define MISO_PIN 19
#define MOSI_PIN 23

#define GET_CHIP_ID 0x9F
#define LEN_ID 4
byte chipId[4];

unsigned char writePage[256] = "";
unsigned char readPage[256] = "";
w25q64 spiChip;

void chipInit();
void chipGetId();

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  spiChip.begin();
  spiChip.getId(chipId);

  Serial.print("CHIP ID: ");

  for (int i = 0; i < LEN_ID; i++)
  {
    Serial.print(chipId[i], HEX);
  }

  Serial.println();

  //Menulis data ke chip w25q64
  memcpy(writePage, "Nusantech Academy 1", sizeof("Nusantech Academy 1"));
  spiChip.erasePageSector(0xFFFF);
  spiChip.pageWrite(writePage, 0xFFFF);
  Serial.println("Writing is done");
  delay(1000);

  //membaca data dari chip w25q64
  Serial.println("Start Reading");
  spiChip.readPages(readPage, 0xFFFF, 1);
  for (int i = 0; i < 256; i++)
  {
    if (readPage[i] > 8 && readPage [i] < 127)
    {
      Serial.print((char)readPage[i]);
    }

  }

}

void loop()
{
  // put your main code here, to run repeatedly:
}

void chipInit()
{
  pinMode(CS_PIN, OUTPUT);
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);
  digitalWrite(CS_PIN, HIGH); // seting chip dalam kondisi HIGH atau tidak komunikasi
}

void chipGetId()
{
  digitalWrite(CS_PIN, LOW); // untuk setin LOW agar chip siap terima data
  SPI.transfer(GET_CHIP_ID);

  // Receive ID
  for (int i = 0; i < LEN_ID; i++)
  {
    chipId[i] = SPI.transfer(0);
  }
  digitalWrite(CS_PIN, HIGH);

  //Print chipID
  Serial.print("CHIP ID: ");
  for (int i = 0; i < LEN_ID; i++)
  {
    Serial.print(chipId[i], HEX);
  }
  Serial.println();

}