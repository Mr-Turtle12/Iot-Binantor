#include <WiFiManager.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <PubSubClient.h>

const char *ssid = "BLEsensorSetUp";
const char *password = "password";
const char *mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;
const char *mqtt_publish_topic = "uok/iot/wah20/location";
const char *mqtt_subscribe_topic = "uok/iot/wah20/setLocation";

int Collect_pos = 0;
int Store_pos = 0;
int BeaconRSSI;
char current_location = ""
WiFiClient espClient;
PubSubClient client(espClient);

void callback(char *topic, byte *payload, unsigned int length)
{
  payload[length] = '\0'; // Null-terminate the payload
  String message = String((char *)payload);

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
}

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
      client.subscribe(mqtt_subscribe_topic);
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

void setup_wifi()
{
  WiFiManager wm;
  wm.resetSettings();
  bool res;
  res = wm.autoConnect(ssid, password);

  Serial.println("Connected to WiFi network!");
  Serial.println("Connected!");

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void setup()
{
  Serial.begin(115200);
  setup_wifi();
  connectMQTT()
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
  pBLEScan->setActiveScan(true);                    // start to scan
  BLEScanResults _ = pBLEScan->start(5); // wait for scanning

  pBLEScan->clearResults();
  String location = (Collect_pos > Store_pos) ? "collect" : "store";
  if(current_location != location){
      client.publish(mqtt_publish_topic, location.c_str());
      current_location = location;
  }
  delay(10); // Scan every 5 seconds
}
