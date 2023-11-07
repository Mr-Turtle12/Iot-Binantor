#include <PicoMQTT.h>
#include <HTTPClient.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>  //  max7219 library


String location;
String BinsOut;
HTTPClient http1;
HTTPClient http2;

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

void setup() {
  // Setup serial
  Serial.begin(115200);

  matrix.setIntensity(15);
  matrix.setRotation(0);
  matrix.fillScreen(0);
  matrix.write();

  // Connect to WiFi
   WiFiManager wm;
    wm.resetSettings();
    bool res;
    res = wm.autoConnect("BinPalSetUp","password"); 

  if (!MDNS.begin("esp32config")) {
    Serial.println("Error setting up MDNS responder!");
  }

  http1.begin("http://192.168.118.195:5000/GeneralCheckValue/Canterbury ct27lh 64/");
  http2.begin("http://esp32config/");

}

void loop() {

  matrix.fillScreen(0);
  matrix.write();
  Serial.println(BinsOut);
  Serial.println(location);
  if (BinsOut == ("true\n")) {
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

  int httpCode = http2.GET();
  if (httpCode > 0) {
    String payload = http2.getString();
    location = payload;
  } else {
    Serial.println("Error on HTTP request");
  }
  http2.end();  // Close connection

  httpCode = http1.GET();
  if (httpCode > 0) {
    String payload = http1.getString();
    BinsOut = payload;
  } else {
    Serial.println("Error on HTTP request");
  }
  http1.end();  // Close connection

  delay(5000);
}
