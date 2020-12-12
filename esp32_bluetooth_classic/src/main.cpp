#include <Arduino.h>
#include <wire.h>
#include <SPI.h>
#include <BluetoothSerial.h>
#include <DHT.h>

#define DHT_PIN 5
#define DHT_TYPE DHT11

BluetoothSerial SerialBT;
DHT dht(DHT_PIN, DHT_TYPE);
float temperature = 0, humidity = 0;
int lastTransmit = 0;
String receivedString;

void updateDhtData();

void setup()
{
  Serial.begin(9600);
  SerialBT.begin("ESP32-BTClassic");
  pinMode(BUILTIN_LED, OUTPUT);
  dht.begin();
}

void loop()
{
  if (millis() - lastTransmit > 5000)
  {
    updateDhtData();
    String sentence = "Data sensor -> Suhu: " + String(temperature) + " C dan kelembaban: " + String(humidity) + "%";
    Serial.println(sentence);
    SerialBT.println(sentence);
    lastTransmit = millis();
  }

  if (SerialBT.available())
  {
    char receivedChar = SerialBT.read();
    receivedString += receivedChar;

    if (receivedChar == '!')
    {
      if (receivedString == "led on!")
      {
        Serial.println("Turn On Led");
        SerialBT.println("Turn On Led");
        digitalWrite(BUILTIN_LED, HIGH);
      } else if (receivedString == "led off!")
      {
        Serial.println("Turn Off Led");
        SerialBT.println("Turn Off Led");
        digitalWrite(BUILTIN_LED, LOW);
      } else
      {
        Serial.println("Could not recognize command");
        SerialBT.println("Could not recognize command");
      }
      receivedString = "";
      SerialBT.flush();
    }
  }
}

void updateDhtData()
{
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
}