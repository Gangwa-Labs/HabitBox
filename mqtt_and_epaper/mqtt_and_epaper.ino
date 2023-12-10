#include <WiFi.h>
#include <PubSubClient.h>
#include <GxEPD2_BW.h>  // including both doesn't use more code or ram
#include "image.h"
// WiFi credentials
const char* ssid = "enable";
const char* password = "parthenon";

// MQTT Broker settings
const char* mqtt_broker = "192.168.0.104";
const int mqtt_port = 1883;
int dayMark = 0;
int week[7];

WiFiClient espClient;
PubSubClient client(espClient);
GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(GxEPD2_154_D67(/*CS=5*/ 5, /*DC=*/0, /*RST=*/2, /*BUSY=*/15));  // GDEH0154D67 200x200, SSD1681

void setupWifi() {
  delay(10);
  Serial.println("Connecting to WiFi...");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    if (client.connect("HabitBox")) {
      Serial.println("connected");
      client.subscribe("update");
      client.subscribe("currentDate");
      client.subscribe("reset");
      client.subscribe("sunday");
      client.subscribe("monday");
      client.subscribe("tuesday");
      client.subscribe("wednesday");
      client.subscribe("thursday");
      client.subscribe("friday");
      client.subscribe("saturday");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  display.init(115200, true, 2, false);  // USE THIS for Waveshare boards with "clever" reset circuit, 2ms reset pulse
  display.setTextColor(GxEPD_BLACK);
  display.firstPage();
  setupWifi();
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    messageTemp += (char)message[i];
  }
  if (strcmp(topic, "currentDate") == 0) {
    dayMark = messageTemp.toInt();
    updateDisplay();
  }
  if (strcmp(topic, "update") == 0) {
    updateDisplay();
  }
  if (strcmp(topic, "sunday") == 0) {
    week[0] = messageTemp.toInt();
  }
  if (strcmp(topic, "monday") == 0) {
    week[1] = messageTemp.toInt();
  }
  if (strcmp(topic, "tuesday") == 0) {
    week[2] = messageTemp.toInt();
  }
  if (strcmp(topic, "wednesday") == 0) {
    week[3] = messageTemp.toInt();
  }
  if (strcmp(topic, "thursday") == 0) {
    week[4] = messageTemp.toInt();
  }
  if (strcmp(topic, "friday") == 0) {
    week[5] = messageTemp.toInt();
  }
  if (strcmp(topic, "saturday") == 0) {
    week[6] = messageTemp.toInt();
  }
  Serial.println(messageTemp);
}

void updateDisplay() {
  display.fillScreen(GxEPD_WHITE);
  display.drawBitmap(0, 0, epd_bitmap_weekBitmaps, 136, 200, GxEPD_BLACK);
  updateDay(dayMark);
  updateCircle();
  display.display();
  if (week[dayMark] == 1) {
    Serial.println("open the box");
  } else {
    Serial.println("close the box");
  }
}
void updateCircle() {
  for (int i = 0; i < 7; i++) {
    if (week[i] == 0) {
      display.drawBitmap(140, (i * 29), epd_bitmap_closedCircle, 26, 26, GxEPD_BLACK);
    } else {
      display.drawBitmap(140, (i * 29), epd_bitmap_openCircle, 26, 26, GxEPD_BLACK);
    }
  }
}

void updateDay(int currentDay) {
  display.drawBitmap(172, 11 + (29 * currentDay), epd_bitmap_Line, 22, 5, GxEPD_BLACK);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
