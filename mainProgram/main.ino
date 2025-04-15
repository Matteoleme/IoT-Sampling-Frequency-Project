#include "config.h"
#include "signal_processing.h"
#include "wifi_mqtt.h"
#include "task_manager.h"
#include <WiFi.h>
#include "PubSubClient.h"

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\nStart!\n\n");

  // Setup WiFi
  setupWiFi();
  
  // Setup MQTT
  setupMQTT();
  while(WiFi.status() == WL_CONNECTED && !mqttClient.connected()) mqttReconnect();
  Serial.println("Connected to mqtt server");
  // Initialize all tasks
  initializeTasks();
  
  Serial.println("System initialized and running.");
}

void loop() {
  // Handle MQTT connection if needed
  if (WiFi.status() == WL_CONNECTED && !mqttClient.connected()) {
    mqttReconnect();
  }
  
  // Keep MQTT client alive
  mqttClient.loop();
  
  // Nothing else needed in the main loop as tasks handle everything
  vTaskDelay(100);
}
