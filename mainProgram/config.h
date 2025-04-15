#ifndef CONFIG_H
#define CONFIG_H

// Libraries
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include "arduinoFFT.h"

// WiFi and MQTT conf
static const char* ssid = "";
static const char* password = "";
static const char* mqttServer = "";
static const int port = 1883;
static const char* mqttTopic = "matteo/max";
#define MSG_BUFFER_SIZE (50)

// Synthetic signal
static const float signalFrequency = 1;
static const float signalFrequency2 = 7;
static const float signalFrequency3 = 20;
static const uint16_t amplitude = 10;

// FFT conf
static const uint16_t samples = 512;
static const uint16_t maxFrequencyAvailable = 32500;  // Max frequency
extern uint16_t samplingFrequency;  // Initially max, will be optimized


static const float windowSize = 5;  // Size of the window for aggregation
// if I want to know how many samples, I have to multiply time and sampling frequency

#endif // CONFIG_H