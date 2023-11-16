#include <WiFiManager.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <PubSubClient.h>
#include <EEPROM.h>

#define COLLECT_POS_ADDR 0
#define STORE_POS_ADDR 4

int Collect_pos = 0;
int Store_pos = 0;
int BeaconRSSI;
char current_location = "";
WiFiClient espClient;
PubSubClient client(espClient);

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
    if (advertisedDevice.getAddress() == BLEAddress("fd:d3:6f:39:97:b8"))
    {
      BeaconRSSI = advertisedDevice.getRSSI();
    }
  }
};

void connectMQTT()
{
  while (!client.connected())
  {
    Serial.println("Attempting MQTT connection...");
    if (client.connect("BLE_Client"))
    {
      Serial.println("Connected to MQTT broker");
      client.subscribe("uok/iot/wah20/setLocation");
    }
    else
    {
      Serial.print("Failed to connect to MQTT, rc=");
      Serial.print(client.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void setupWiFi()
{
  WiFiManager wm;
  wm.resetSettings();
  bool res;
  res = wm.autoConnect("BLEsensorSetUp", "password");

  Serial.println("Connected to WiFi network!");
  Serial.println("Connected!");
}

void setup()
{
  Serial.begin(115200);
  setupWiFi();

  client.setServer("test.mosquitto.org", 1883);
  client.setCallback(callback);
  connectMQTT();

  EEPROM.begin(512);
  Collect_pos = EEPROM.readInt(COLLECT_POS_ADDR);
  Store_pos = EEPROM.readInt(STORE_POS_ADDR);
}

void savePositionsToEEPROM()
{
  EEPROM.writeInt(COLLECT_POS_ADDR, Collect_pos);
  EEPROM.writeInt(STORE_POS_ADDR, Store_pos);
  EEPROM.commit();
}

void handleMQTTMessage(String message)
{
  if (message.equals("collect"))
  {
    Collect_pos = BeaconRSSI;
    Serial.println("Set Collect");
  }
  else if (message.equals("store"))
  {
    Store_pos = BeaconRSSI;
    Serial.println("Set Store");
  }
  savePositionsToEEPROM();
}

void publishLocation()
{
  String location = (Collect_pos > Store_pos) ? "collect" : "store";
  if (current_location != location)
  {
    client.publish("uok/iot/wah20/location", location.c_str());
    current_location = location;
  }
}

void loop()
{
  if (!client.connected())
  {
    connectMQTT();
  }
  client.loop();

  BLEDevice::init("");
  BLEScan *pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); // start to scan
  BLEScanResults _ = pBLEScan->start(5); // wait for scanning

  pBLEScan->clearResults();

  publishLocation();
}
