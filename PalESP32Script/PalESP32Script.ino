#include <PicoMQTT.h>
#include <PubSubClient.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>  // max7219 library
#include <WiFiManager.h>

#define MQTT_BROKER "test.mosquitto.org"
#define MQTT_PORT (1883)
#define MQTT_BinDay_TOPIC "uok/iot/wah20/bin_day"
#define MQTT_SetLocation_TOPIC "uok/iot/wah20/bin_day"

String location;
WiFiClient wifiClient;
PubSubClient client(wifiClient);
bool BinsOut = false;
Max72xxPanel matrix = Max72xxPanel(5, 1, 1);

const uint8_t SMILEY_FACE[] = {
  0b00111100,
  0b01000010,
  0b10100101,
  0b10000001,
  0b10100101,
  0b10011001,
  0b01000010,
  0b00111100
};

const uint8_t SAD_FACE[] = {
  0b00111100,
  0b01000010,
  0b10100101,
  0b10000001,
  0b10011001,
  0b10100101,
  0b01000010,
  0b00111100
};

void drawImage(const uint8_t* image, int size) {
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < 8; j++) {
      if ((image[i] >> j) & 1) {
        matrix.drawPixel(j, i, 1);
      }
    }
  }
}

void setUpMatrix() {
  matrix.setIntensity(15);
  matrix.setRotation(0);
  matrix.fillScreen(0);
  matrix.write();
}

void updateDisplay() {
  matrix.fillScreen(0);

  if (BinsOut) {
    if (location.equals("collect")) {
      Serial.println("Smiley face - in the correct pos");
      drawImage(SMILEY_FACE, sizeof(SMILEY_FACE) / sizeof(SMILEY_FACE[0]));
    } else {
      drawImage(SAD_FACE, sizeof(SAD_FACE) / sizeof(SAD_FACE[0]));
      Serial.println("Sad face - not in correct pos");
    }
  } else {
    drawImage(SMILEY_FACE, sizeof(SMILEY_FACE) / sizeof(SMILEY_FACE[0]));
    Serial.println("Not Bin days - Smiley face");
  }

  matrix.write();
}

void callback(char* topic, byte* payload, unsigned int length) {
  if (strcmp(topic, MQTT_BinDay_TOPIC) == 0) {
    BinsOut = !BinsOut;
  } else {
    location = String((char*)payload);
  }
}

void connectToMQTT() {
  while (!client.connected()) {
    if (client.connect(("ESP32-" + String(random(0xffff), HEX)).c_str())) {
      Serial.println("MQTT connected.");
      client.subscribe(MQTT_BinDay_TOPIC);
      client.subscribe(MQTT_SetLocation_TOPIC);
    } else {
      Serial.printf("Failed, rc=%d. Retrying in 5 seconds.", client.state());
      delay(5000);
    }
  }
}

void setupWiFi() {
  WiFiManager wm;
  bool res;
  res = wm.autoConnect("BinPalSetUp", "password");
}

void setup() {
  // Setup serial
  Serial.begin(115200);

  setUpMatrix();

  // Connect to WiFi
  setupWiFi();

  // Connect to MQTT broker
  client.setServer(MQTT_BROKER, MQTT_PORT);
  client.setCallback(callback);

  connectToMQTT();
}

void loop() {
  updateDisplay();
  Serial.println(BinsOut);
  Serial.println(location);

  client.loop();
  delay(5000);
}
