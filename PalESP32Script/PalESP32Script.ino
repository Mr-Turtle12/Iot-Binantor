#include <PicoMQTT.h>
#include <HTTPClient.h>
#include <PubSubClient.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>  //  max7219 library
#include <WiFiManager.h>


#define MQTT_BROKER "test.mosquitto.org"
#define MQTT_PORT (1883)
#define MQTT_SUBSCRIBE_TOPIC "uok/iot/wah20/subscribe"


String location;
WiFiClient wifiClient;
PubSubClient client(wifiClient);
bool BinsOut = false;
HTTPClient http;
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

void setUpmatrix(){
  matrix.setIntensity(15);
  matrix.setRotation(0);
  matrix.fillScreen(0);
  matrix.write();
}
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print(topic);
  BinsOut = !BinsOut;
}

void setup() {
  // Setup serial
  Serial.begin(115200);

  setUpmatrix();

  // Connect to WiFi
  WiFiManager wm;
  bool res;
  res = wm.autoConnect("BinPalSetUp","password"); 

  //Connect to both API
  client.setServer(MQTT_BROKER, MQTT_PORT);  // set broker settings
  client.setCallback(callback);           
  while (!client.connected()) {              // check connected status
    if (client.connect(("ESP32-" + String(random(0xffff), HEX)).c_str())) {  // connect with random id
      Serial.println("MQTT connected.");                                   // report success
      client.subscribe(MQTT_SUBSCRIBE_TOPIC);                              // subscribe to the chosen topic
    } else {
      Serial.printf(" failed , rc=%d try again in 5 seconds", client.state());  // report error
      delay(5000);                                                              // wait 5 seconds
    }
  }
  http.begin("http://esp32config/");

}

void loop() {

  matrix.fillScreen(0);
  matrix.write();
  Serial.println(BinsOut);
  Serial.println(location);
  if (BinsOut) {
    if (location == ("Collect")) {
      Serial.println("simily face - in the correct pos");
      drawImage(SMILEY_FACE, sizeof(SMILEY_FACE) / sizeof(SMILEY_FACE[0]));

    } else {
      drawImage(SAD_FACE, sizeof(SAD_FACE) / sizeof(SAD_FACE[0]));
      Serial.println("sad face - not in correct pos");

    }
  } else {
    drawImage(SMILEY_FACE, sizeof(SMILEY_FACE) / sizeof(SMILEY_FACE[0]));
    Serial.println("Not Bin days - Simily face");
  }
  matrix.write();
  client.loop();

  int httpCode = http.GET();
  if (httpCode > 0) {
    String payload = http.getString();
    location = payload;
  } else {
    Serial.println("Error on HTTP request");
  }
  http.end();  // Close connection

  delay(5000);
}
