#include <arduinoFFT.h>
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
const uint16_t samples = 1024;
const float signalFrequency = 10;
const float signalFrequency2 = 300;
const float signalFrequency3 = 1200;
const uint16_t samplingFrequency = 2500;
const uint16_t amplitude = 50;
float vReal[samples];
float vImag[samples];

ArduinoFFT<float> FFT = ArduinoFFT<float>(vReal, vImag, samples, samplingFrequency);

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
  // FFT and returns avg value
  float avg = performFFT();
  
  // Send the computed value to LoRa
  lpp.reset();
  lpp.addAnalogInput(1, avg);
  
  if (ttn.sendBytes(lpp.getBuffer(), lpp.getSize())) {
    Serial.printf("FFT Average: %f\n", avg);
  }
  
  delay(10000);
}


float performFFT() {
  float ratio = TWO_PI * signalFrequency / samplingFrequency;
  float ratio2 = TWO_PI * signalFrequency2 / samplingFrequency;
  float ratio3 = TWO_PI * signalFrequency3 / samplingFrequency;
  
  for (uint16_t i = 0; i < samples; i++) {
    vReal[i] = float(amplitude * sin(i * ratio));
    vReal[i] += float(amplitude * sin(i * ratio2));
    vReal[i] += float(amplitude * sin(i * ratio3));
    vImag[i] = 0.0;
  }
  
  FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward);
  FFT.compute(FFTDirection::Forward);
  FFT.complexToMagnitude();
  
  float average = 0;
  for (int i = 0; i < samples / 2; i++) {
    average += vReal[i];
  }
  average /= (samples / 2);
  
  Serial.print("FFT average value: ");
  Serial.println(average, 6);
  
  return average;
}
