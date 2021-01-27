#include <Arduino.h>
#include <Wifi.h>

const char *ssid = "Semeru";
const char *password = "@K4r4ng4s3M";

String translateEncryptionType(wifi_auth_mode_t encryptionType);
void scanNetworks();
void connectToNetwork();

void setup()
{
  Serial.begin(9600);
  scanNetworks();
  connectToNetwork();
  Serial.println(WiFi.macAddress());
  Serial.println(WiFi.localIP());
}

void loop()
{
  // put your main code here, to run repeatedly:
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