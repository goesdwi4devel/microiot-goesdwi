#include <Arduino.h>

#define POT_PIN 4
#define LED1 18
#define LED2 19
#define LED3 21
#define LED4 22

int adcValue = 0;
float voltage = 0;
int temperature = 0;

void setup()
{
  Serial.begin(9600);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
}

void loop()
{
  adcValue = analogRead(POT_PIN);
  String printData = "Nilai ADC yang terbaca: " + String(adcValue);
  Serial.println(printData);
  voltage = ((float)adcValue / 4095) * 3.3;
  printData = "Nilai tegangan yang terbaca: " + String(voltage) + " V";
  Serial.println(printData);
  temperature = ((float)adcValue / 4095) * 100;
  printData = "Nilai termperatur yang terbaca: " + String(temperature) + " C";
  Serial.println(printData);
  delay(1000);

  if (temperature <= 15)
  {
      digitalWrite(LED1, HIGH);
      digitalWrite(LED2, LOW);
      digitalWrite(LED3, LOW);
      digitalWrite(LED4, LOW);
  } else if (temperature >= 16 and temperature <= 25)
  {
      digitalWrite(LED1, LOW);
      digitalWrite(LED2, HIGH);
      digitalWrite(LED3, LOW);
      digitalWrite(LED4, LOW);
  } else if (temperature >=25 and temperature <=30)
  {
      digitalWrite(LED1, LOW);
      digitalWrite(LED2, LOW);
      digitalWrite(LED3, HIGH);
      digitalWrite(LED4, LOW);
  } else
  {
      digitalWrite(LED1, LOW);
      digitalWrite(LED2, LOW);
      digitalWrite(LED3, LOW);
      digitalWrite(LED4, HIGH);
  }
}
