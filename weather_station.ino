#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <NTPClient.h>
#include <DHT.h>
#include "config.h"


// Pins
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   64
#define OLED_MOSI       D7
#define OLED_CLK        D5
#define OLED_DC         D2
#define OLED_CS         D8
#define OLED_RESET      D3
#define DHTPIN          D1
#define DHTTYPE         DHT22


// initialization
WiFiUDP ntpUDP;
WiFiClient wifiClient;
PubSubClient client(wifiClient);
NTPClient timeClient(ntpUDP);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
                         OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
DHT dht(DHTPIN, DHTTYPE);


void setup() {
  Serial.begin(115200);
  connectWiFi();
  dht.begin();
  timeClient.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
}

void loop() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  printWeatherToDisplay(temperature, humidity);
  publishWeatherToMqtt(temperature, humidity);
  delay(5000);
}


// helper functions
void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("connecting to wifi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println(".");
    delay(3000);
  }
  Serial.println(WiFi.localIP());
  connectToMQTTServer();
}

void connectToMQTTServer() {
  client.setServer(MQTT_BROKER, MQTT_PORT);
  int maxRetries = 3;
  int count = 0;
  while (!client.connected()) {
    Serial.println("connecting to mqtt server");
    if (client.connect("nodemcu-jibin")) {
      Serial.println("connection to mqtt server is successful");
    } else {
      Serial.print("connection failed due to");
      Serial.println(client.state());
      delay(2000);
    }
    count++;
    if (count >= maxRetries) break;
  }
}

void publishWeatherToMqtt(float temperature, float humidity) {
  StaticJsonDocument<200> doc;
  char json_string[256];
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  serializeJson(doc, json_string);
  client.publish("weather", json_string);
}

int getEpochTime() {
  timeClient.update();
  return timeClient.getEpochTime();
}

void printWeatherToDisplay(float temperature, float humidity) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print(F("T: "));
  display.print(temperature);
  display.print("C");
  display.println();
  display.println();
  display.print(F("H: "));
  display.print(humidity);
  display.print("%");
  display.println();
  display.display();
}
