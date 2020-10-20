#include <Arduino.h>
#include <EEPROM.h>

#define EEPROM_SIZE 1
#define BUTTON_PIN 15

unsigned char ledStatus = LOW;
bool changeLedStatus = false;
portMUX_TYPE gpioIntMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR gpioISR() {
  portENTER_CRITICAL(&gpioIntMux);
  changeLedStatus = true;
  portEXIT_CRITICAL(&gpioIntMux);
}

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUILTIN_LED, OUTPUT);
  attachInterrupt(BUTTON_PIN, &gpioISR, RISING);

  EEPROM.begin(EEPROM_SIZE);
  delay(100);
  ledStatus = EEPROM.read(0);
  digitalWrite(BUILTIN_LED, ledStatus);
}

void loop() {
  if (changeLedStatus)
  {
    portENTER_CRITICAL(&gpioIntMux);
    changeLedStatus = false;
    portEXIT_CRITICAL(&gpioIntMux);

    ledStatus = !ledStatus;
    EEPROM.write(0, ledStatus);
    EEPROM.commit();

    digitalWrite(BUILTIN_LED, ledStatus);
  }
}