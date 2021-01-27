#include <Arduino.h>
#include <Wifi.h>
#include <wire.h>
#include <SPI.h>
#include <DHT.h>
#include <time.h>

typedef struct sensorData
{
  struct tm timeinfo;
  float temperature = 0;
  float humidity = 0;
} sensorData;

const char *ssid = "Semeru";
const char *password = "@K4r4ng4s3M";
const char *ntpServer = "pool.ntp.org";
const long gmtOffsetSec = 28800;
const int daylightOffsetSec = 0;
sensorData dht11Data;

#define DHT_PIN 5
#define DHT_TYPE DHT11

DHT dht(DHT_PIN, DHT_TYPE);
float temperature = 0, humidity = 0;

String translateEncryptionType(wifi_auth_mode_t encryptionType);
void scanNetworks();
void connectToNetwork();
void updateDhtData();
void updateLocalTime();

void setup()
{
  Serial.begin(9600);
  dht.begin();
  scanNetworks();
  connectToNetwork();
  Serial.println(WiFi.macAddress());
  Serial.println(WiFi.localIP());

  configTime(gmtOffsetSec, daylightOffsetSec, ntpServer);
}

void loop()
{
  updateLocalTime();
  updateDhtData();
  Serial.print("Temperature: ");
  Serial.print(dht11Data.temperature);
  Serial.print(" C and Humidity: ");
  Serial.print(dht11Data.humidity );
  Serial.print(" % pada ");
  Serial.println(&dht11Data.timeinfo, "%A, %B %d %Y %H:%M:%S");
  delay(5000);
}

String translateEncryptionType(wifi_auth_mode_t encryptionType)
{
  switch (encryptionType)
  {
  case (WIFI_AUTH_OPEN):
    return "Open";
    break;
  case (WIFI_AUTH_WEP):
    return "WEP";
    break;
  case (WIFI_AUTH_WPA_PSK):
    return "WPA_PSK";
    break;
  case (WIFI_AUTH_WPA2_PSK):
    return "WPA2_PSK";
    break;
  case (WIFI_AUTH_WPA_WPA2_PSK):
    return "WPA_WPA2_PSK";
    break;
  case (WIFI_AUTH_WPA2_ENTERPRISE):
    return "WPA2_ENTERPRISE";
    break;
  default:
    return "Invalid type of WiFi";
    break;
  }
}

void scanNetworks() {
  int numOfNetworks = WiFi.scanNetworks();

  Serial.print("Number of Networks found: ");
  Serial.println(numOfNetworks);

  for (int i = 0; i < numOfNetworks; i++)
  {
    Serial.print("Network Name: ");
    Serial.println(WiFi.SSID(i));
    Serial.print("Signal Strength: ");
    Serial.println(WiFi.RSSI(i));
    Serial.print("MAC Address: ");
    Serial.println(WiFi.BSSIDstr(i));
    Serial.print("Encryption Type: ");
    String encryptionTypeDescription = translateEncryptionType(WiFi.encryptionType(i));
    Serial.println(encryptionTypeDescription);
    Serial.println("============================");
  }
}


void connectToNetwork() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Establishing connection to WiFi..");
    Serial.println(ssid);
  }
  Serial.println("Connected to WiFi network");
}

void updateDhtData()
{
  dht11Data.temperature = dht.readTemperature();
  dht11Data.humidity = dht.readHumidity();
}

void updateLocalTime()
{
  if (!getLocalTime(&dht11Data.timeinfo))
  {
    Serial.println("Failed to obtain time");
  }

}

