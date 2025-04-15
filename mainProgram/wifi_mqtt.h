#ifndef WIFI_MQTT_H
#define WIFI_MQTT_H

#include "config.h"

extern WiFiClient espClient;
extern PubSubClient mqttClient;

void setupWiFi();
void setupMQTT();
void mqttReconnect();
bool publishMQTTMessage(const char* topic, float value);

#endif // WIFI_MQTT_H