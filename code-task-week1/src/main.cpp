#include <Arduino.h>

#define LED1 14
#define LED2 4
#define LED3 5
#define LED4 16
String readString;

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
}

void loop()
{
  if (Serial.available())
  {
    char data = Serial.read();
    readString += data;
    Serial.print("Karakter yang dikirim: ");
    Serial.println(readString);

    if (readString == "1")
    {
      Serial.println("LED1 On");
      digitalWrite(LED1, HIGH);
      delay(2000);
      digitalWrite(LED1, LOW);
    }
    else if (readString == "2")
    {
      Serial.println("LED2 On");
      digitalWrite(LED2, HIGH);
      delay(2000);
      digitalWrite(LED2, LOW);
    }
    else if (readString == "3")
    {
      Serial.println("LED3 On");
      digitalWrite(LED3, HIGH);
      delay(2000);
      digitalWrite(LED3, LOW);
    } else if (readString == "4")
    {
      Serial.println("LED4 On");
      digitalWrite(LED4, HIGH);
      delay(2000);
      digitalWrite(LED4, LOW);
    } else {
      Serial.println("Masukkan nilai 1 - 4");
    }
    readString = "";
    Serial.println("");
  }
}