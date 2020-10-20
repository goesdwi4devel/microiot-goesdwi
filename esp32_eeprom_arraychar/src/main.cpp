#include <Arduino.h>
#include <EEPROM.h>

#define EEPROM_SIZE 32

char readData[EEPROM_SIZE], receiveData[EEPROM_SIZE];
int dataIndex = 0;

void readEEPROM(int address, char * data);
void writeEEPROM(int address, char * data);

void setup() {
  Serial.begin(9600);

  EEPROM.begin(EEPROM_SIZE);
  delay(100);

  readEEPROM(0, readData);
  Serial.print("Isi EEPROM: ");
  Serial.println(readData);
}

void loop() {
  if (Serial.available()) {
    receiveData[dataIndex] = Serial.read();
    dataIndex++;

    if (receiveData[dataIndex - 1] == '\n')
    {
      dataIndex = 0;
      writeEEPROM(0, receiveData);
      memset(receiveData, 0, EEPROM_SIZE);
    }
  }
}

void readEEPROM(int address, char * data)
{
  for (int i = 0; i < EEPROM_SIZE; i++)
  {
    data[i] = EEPROM.read(address + i);
  }
}

void writeEEPROM(int address, char * data)
{
  Serial.println("Start writing to EEPROM");
  for (int i = 0; i < EEPROM_SIZE; i++)
  {
    EEPROM.write(address + i, data[i]);
  }
  EEPROM.commit();
}