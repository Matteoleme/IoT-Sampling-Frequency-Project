#include <EzLoRaWAN_CayenneLPP.h>
#include <EzLoRaWAN.h>

// OTAA Conf
const char* appEui = ""; // TTN Application EUI
const char* devEui = ""; // TTN Device EUI
const char* appKey = ""; // TTN Application Key

EzLoRaWAN ttn;
EzLoRaWAN_CayenneLPP lpp;

#ifndef AUTO_PIN_MAP
#include "board_config.h"
#endif

// Configurazione FFT
const float samplingFrequency = 50;
float signalFrequency = 1;
float signalFrequency2 = 7;
float signalFrequency3 = 20;
float amplitude = 10;

const int sec = 10;     // window time size
const int samplesWindowSize = samplingFrequency * sec;
float windowBuffer[samplesWindowSize];
int buffIndex = 0;
int windowCount = 0;

// Timers
unsigned long lastSampleTime = 0;
unsigned long samplingIntervalMs = 1000.0 / samplingFrequency;

float lastSampleValue = 0;
bool windowReady = false;


void setup() {
  Serial.begin(9600);
  Serial.println("Starting");
  
  #ifndef AUTO_PIN_MAP
  SPI.begin(RADIO_SCLK_PIN, RADIO_MISO_PIN, RADIO_MOSI_PIN);
  #endif
  
  ttn.begin();
  ttn.join(devEui, appEui, appKey);
  
  Serial.print("Joining TTN ");
  while (!ttn.isJoined()) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\njoined!");
}


void loop() {
    continuousSamplingFunction(); // campiona e aggiorna il buffer

  if (windowReady) {
    float maxVal = processWindowAndGetMax();
    windowCount++;
    windowReady = false;

    // invia su TTN
    lpp.reset();
    lpp.addAnalogInput(1, maxVal);
    if (ttn.sendBytes(lpp.getBuffer(), lpp.getSize())) {
      Serial.printf("Window #%d - Max: %f\n", windowCount, maxVal);
    }

    // dopo 5 finestre vai in deep sleep
    if (windowCount % 10 == 5) {
      Serial.println("Going to deep sleep for 15 seconds...");
      esp_sleep_enable_timer_wakeup(15000000); // 15 secondi
      esp_deep_sleep_start();
    }
  }
}


void continuousSamplingFunction() {
  unsigned long now = millis();
  if (now - lastSampleTime >= samplingIntervalMs) {
    float time = windowCount * samplesWindowSize / samplingFrequency + buffIndex / samplingFrequency;
    float value = amplitude * sin(TWO_PI * signalFrequency * time);
    value += amplitude * sin(TWO_PI * signalFrequency2 * time);
    value += amplitude * sin(TWO_PI * signalFrequency3 * time);

    lastSampleValue = value;
    lastSampleTime = now;
    
    // Passa alla funzione del buffer
    bufferSample(value);
  }
}

void bufferSample(float value) {
  windowBuffer[buffIndex++] = value;
  if (buffIndex >= samplesWindowSize) {
    buffIndex = 0;
    windowReady = true;
  }
}

float processWindowAndGetMax() {
  float maxVal = windowBuffer[0];
  for (int i = 1; i < samplesWindowSize; i++) {
    if (windowBuffer[i] > maxVal) maxVal = windowBuffer[i];
  }
  return maxVal;
}
