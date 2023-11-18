#include <PicoMQTT.h>
#include <PubSubClient.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>  // max7219 library
#include <WiFiManager.h>

#define MQTT_BROKER "test.mosquitto.org"
#define MQTT_PORT (1883)
#define MQTT_SetLocation_TOPIC "uok/iot/wah20/currentLocation"
#define MQTT_BinTime_TOPIC "uok/iot/wah20/binTime"


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

const uint8_t Confused[] = {
{
  0b00111000,
  0b01000100,
  0b00000100,
  0b00000100,
  0b00011000,
  0b00010000,
  0b00000000,
  0b00010000
}};

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
    if (location.equals("collect")) {
      drawImage(SMILEY_FACE, sizeof(Confused) / sizeof(SMILEY_FACE[0]));
      Serial.println("Show ? as it shouldn't be left in collect pos");
    }else{
      drawImage(SMILEY_FACE, sizeof(SMILEY_FACE) / sizeof(SMILEY_FACE[0]));
      Serial.println("Not Bin days - Smiley face");
    }
  }

  matrix.write();
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("Incoming message:");

  Serial.println(topic);
  String receivedMessage;
  receivedMessage.reserve(length + 1);  // +1 for the null terminator  
  for (int i = 0; i < length; i++) {
        receivedMessage += (char)payload[i];
  }
  Serial.println(receivedMessage);

  if (strcmp(topic, MQTT_BinTime_TOPIC) == 0) {
    if(receivedMessage.equals("true")){
      BinsOut = true;
    }else{
      BinsOut = false;
    }
  } else {
      Serial.print("Location saved as");
      Serial.println(receivedMessage);
      location = receivedMessage;
  }
  Serial.println("Finished message logic");

}

void connectToMQTT() {
  while (!client.connected()) {
    if (client.connect(("ESP32-" + String(random(0xffff), HEX)).c_str())) {
      //Serial.println("MQTT connected.");
      client.subscribe(MQTT_SetLocation_TOPIC);
      client.subscribe(MQTT_BinTime_TOPIC);
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
  //Set Location to store and wait till you get the other esp32 to tell you if it's moved
  location = "store";
}

void loop() {
  updateDisplay();
  Serial.println(">>>>>>>>>>>>>>>>");
  Serial.println("Vars:");
  Serial.println(BinsOut);
  Serial.println(location);
  Serial.println(">>>>>>>>>>>>>>>>");

  client.loop();
  delay(5000);
}
