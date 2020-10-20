#include <Arduino.h>

#define BUTTON_PIN 21

int ledStatus = LOW;

volatile bool interruptState = false;
int totalInterruptCounter = 0;

hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR gpioISR() {
  ledStatus = !ledStatus;
  digitalWrite(BUILTIN_LED, ledStatus);
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  // attachInterrupt(BUTTON_PIN, &gpioISR, FALLING);
  attachInterrupt(BUTTON_PIN, &gpioISR, RISING);
}

void loop() {
}