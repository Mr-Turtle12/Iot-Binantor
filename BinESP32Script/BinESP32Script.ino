#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <WebServer.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <ESPmDNS.h>


int Collect_pos = 0;
int Store_pos = 0;
int BeaconRSSI;
WebServer server(80);

void handleRoot() {
  if(abs(BeaconRSSI - Collect_pos) < abs(BeaconRSSI - Store_pos)){
        server.send(200, "text/plain", "Collect");
      }else{
        server.send(200, "text/plain", "Store");
      }
}

void SetCollect() {
  Collect_pos = BeaconRSSI;
  SetsButton();
}
void SetStore() {
  Store_pos = BeaconRSSI;
  SetsButton();
}
void SetsButton(){
  server.send(200, "text/html", "<html><body><h1>Setup for bin sensor</h1><p>Bin collect current saved postion: " + String(Collect_pos) + "<form action='/collect' method='get'><button name='button1'>Set Collect position</button></form><p>Bin store current saved postion: " + String(Store_pos) + "</p><form action='/store' method='get'><button name='button2'>Set Store position</button></form></body></html>");
}
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.getAddress() == BLEAddress("fd:d3:6f:39:97:b8")) {
      BeaconRSSI = advertisedDevice.getRSSI();
      Serial.println(BeaconRSSI);
    }
  }
};

void setup() {
  Serial.begin(115200);
  setup_wifi();
}

void setup_wifi() {
   WiFiManager wm;
    wm.resetSettings();
    bool res;
    res = wm.autoConnect("BLEsensorSetUp","password"); 

  if (!MDNS.begin("esp32config")) {
    Serial.println("Error setting up MDNS responder!");
  }

  Serial.println("Connected to WiFi network!");
  Serial.println("Connected!");
  server.on("/", handleRoot);
  server.on("/Setup", SetsButton);
  server.on("/collect", SetCollect);
  server.on("/store", SetStore);

  server.begin();
  Serial.println("HTTP server started");
}


void loop() {
  server.handleClient();
  BLEDevice::init("");
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);                   // start to scan
  BLEScanResults _ = pBLEScan->start(5);  // wait for scanning

  pBLEScan->clearResults();
  delay(10);  // Scan every 5 seconds
}
