#include <Arduino.h>
#include <wire.h>
#include <SPI.h>
#include <DHT.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#define SERVICE_UUID "776310b7-092f-49dc-b7a0-b7a03a1f810d"
#define TX_CHARACTERISTIC_UUID "d858f0bc-ecd4-4436-9a3d-802557d0fb2d"
#define RX_CHARACTERISTIC_UUID "825ba01d-88a7-4300-b5b4-4780f38bf261"

#define LED_PIN 2
#define PWM_CHANNEL 0
#define PWM_FREQ 5000
#define PWM_RESOLUTION 8

#define DHT_PIN 5
#define DHT_TYPE DHT11

DHT dht(DHT_PIN, DHT_TYPE);
float temperature = 0, humidity = 0;
int lastTransmit = 0;
String receivedString;

BLECharacteristic *pTxCharacteristic, *pRxCharacteristic;

class BLECallback : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string rxValue = pCharacteristic->getValue();

    if (rxValue.length() > 0)
    {
      Serial.println("--------------------");
      Serial.print("Received String: ");

      for (int i = 0; i < rxValue.length(); i++)
      {
        Serial.print(rxValue[i]);
      }
      Serial.println();
      Serial.println("--------------------");
    }

    if (rxValue == "1")
    {
      Serial.println("Turn On Led");
      digitalWrite(BUILTIN_LED, HIGH);
    }
    else if (rxValue == "0")
    {
      Serial.println("Turn Off Led");
      digitalWrite(BUILTIN_LED, LOW);
    }
    else
    {
      Serial.println("Could not recognize command");
    }
  }
};

void updateDhtData();

void setup()
{
  Serial.begin(9600);
  pinMode(BUILTIN_LED, OUTPUT);
  dht.begin();

  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(LED_PIN, PWM_CHANNEL);

  BLEDevice::init("ESP32-BLE");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  // sensor data characteristic
  pTxCharacteristic = pService->createCharacteristic(TX_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE);
  pTxCharacteristic->addDescriptor(new BLE2902());

  // receive command characteristic
  pRxCharacteristic = pService->createCharacteristic(RX_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE);
  pRxCharacteristic->addDescriptor(new BLE2902());
  pRxCharacteristic->setValue("0");
  pRxCharacteristic->setCallbacks(new BLECallback());

  pService->start();

  pServer->getAdvertising()->setMinPreferred(0x06);
  pServer->getAdvertising()->setMaxPreferred(0x12);
  pServer->getAdvertising()->start();
  Serial.println("Charactristic Defined");
}

void loop()
{
  if (millis() - lastTransmit > 5000)
  {
    updateDhtData();
    String sentence = String(temperature) + ";" + String(humidity) + ";";
    Serial.println(sentence);

    pTxCharacteristic->setValue(sentence.c_str());
    pTxCharacteristic->notify();

    lastTransmit = millis();
  }

  for (int dutyCyle = 0;  dutyCyle <= 255; dutyCyle++)
  {
    ledcWrite(PWM_CHANNEL, dutyCyle);
    delay(15);
  }
  for (int dutyCyle = 255;  dutyCyle >= 0; dutyCyle--)
  {
    ledcWrite(PWM_CHANNEL, dutyCyle);
    delay(15);
  }


}

void updateDhtData()
{
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
}