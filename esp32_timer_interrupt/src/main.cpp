#include <Arduino.h>

// deklarasi variable
volatile bool interruptState =false;
int totalInterruptCounter = 0;
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onTimer() {
  portENTER_CRITICAL(&timerMux);
  interruptState = true;
  portEXIT_CRITICAL(&timerMux);
}

void setup() {
  Serial.begin(9600);

  // timer initialisasi
  timer = timerBegin(0, 8000, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 10000, true);
  timerAlarmEnable(timer);
  // perhitungan nya x detik =  1 / ((80Mhz / 8000) / 10000) = 1 detik
  //  1 / ((80Mhz / 8000) / 20000) = 2 detik
  //  1 / ((80Mhz / 8000) / 50000) = 5 detik
}

void loop() {
  if (interruptState)
  {
    portENTER_CRITICAL(&timerMux);
    interruptState = false;
    portEXIT_CRITICAL(&timerMux);

    totalInterruptCounter++;
    String sentence = "Total Interrupt Counter: " + String(totalInterruptCounter);
    Serial.println(sentence);
  }
}