#include <Arduino.h>

#define POT_PIN 4
#define LED1 18
#define LED2 19
#define LED3 21
#define LED4 22

int adcValue = 0;
float voltage = 0;
int ledList[4] = {LED1, LED2, LED3, LED4};

void ledOn(int ledNumber);

void setup()
{
  Serial.begin(9600);
  for (int i = 1; i < 5; i++)
  {
    pinMode(ledList[i], OUTPUT);
  }
}

void loop()
{
  adcValue = analogRead(POT_PIN);
  String printData = "Nilai ADC yang terbaca: " + String(adcValue);
  Serial.println(printData);
  voltage = ((float)adcValue / 4095) * 3.3;
  printData = "Nilai tegangan yang terbaca: " + String(voltage) + " V";
  Serial.println(printData);
  float temperature = ((float)adcValue / 4095) * 100;
  printData = "Nilai termperatur yang terbaca: " + String(temperature) + " C";
  Serial.println(printData);
  delay(1000);

  if (temperature <= 15)
  {
    ledOn(1);
  }
  else if (temperature >= 16 and temperature <= 25)
  {
    ledOn(2);
  }
  else if (temperature >= 25 and temperature <= 30)
  {
    ledOn(3);
  }
  else
  {
    ledOn(4);
  }
  delay(500);
}

void ledOn(int ledNumber)
{
  for (int i = 1; i < 5; i++)
  {
    if (i == ledNumber)
    {
      digitalWrite(ledList[i], HIGH);
    }
    else
    {
      digitalWrite(ledList[i], LOW);
    }
  }
}
