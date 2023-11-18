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
String current_location = "";
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


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("Incoming message:");
  Serial.println(topic);
  String receivedMessage;
  receivedMessage.reserve(length + 1);  // +1 for the null terminator  
  for (int i = 0; i < length; i++) {
        receivedMessage += (char)payload[i];
  }
  if(strcmp(topic, "uok/iot/wah20/setLocation") == 0){ 
    if (receivedMessage.equals("setcollect"))
    {
      Collect_pos = BeaconRSSI;
      Serial.println("Set Collect");
    }
    else if (receivedMessage.equals("setstore"))
    {
      Store_pos = BeaconRSSI;
      Serial.println("Set Store");
    }
    savePositionsToEEPROM();
  }else if (strcmp(topic, "uok/iot/wah20/getLocation") == 0){
    publishLocation();
  }
  Serial.println("Finished logic for message");
}


void connectMQTT()
{
  while (!client.connected())
  {
    Serial.println("Attempting MQTT connection...");
    if (client.connect("BLE_Client"))
    {
      Serial.println("Connected to MQTT broker");
      client.subscribe("uok/iot/wah20/setLocation");
      client.subscribe("uok/iot/wah20/getLocation");
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


void publishLocation()
{
  client.publish("uok/iot/wah20/currentLocation", current_location.c_str());
  Serial.print("Publish location:");
  Serial.println(current_location);
}

void loop()
{
  Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
  Serial.println("Start loop");
  Serial.print("Store:");
  Serial.println(Store_pos);
  Serial.print("Collect");
  Serial.println(Collect_pos);
  Serial.println(current_location);

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
  String location = (abs(Collect_pos - BeaconRSSI) < abs(Store_pos - BeaconRSSI)) ? "collect" : "store";
  if (current_location != location)
  {
    current_location = location;
    publishLocation();
  }
}
