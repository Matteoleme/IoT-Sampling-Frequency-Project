#include "wifi_mqtt.h"

// WiFi and MQTT objects
WiFiClient espClient;
PubSubClient mqttClient(espClient);

void setupWiFi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setupMQTT() {
  mqttClient.setServer(mqttServer, port);
}

void mqttReconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    
    // Attempt to connect
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
  }
}

bool publishMQTTMessage(const char* topic, float value) {
  char msg[MSG_BUFFER_SIZE];
  snprintf(msg, MSG_BUFFER_SIZE, "%.4f", value);
  
  Serial.print("Publishing to ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(msg);
  
  return mqttClient.publish(topic, msg);
}