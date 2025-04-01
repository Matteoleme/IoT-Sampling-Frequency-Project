#include <arduinoFFT.h>

#include "arduinoFFT.h"

const uint16_t samples = 1024;
const float signalFrequency = 10;
const float signalFrequency2 = 300;
const float signalFrequency3 = 1200;
const uint16_t samplingFrequency = 5000;    // sostituisci con frequenza massima possibile
const uint16_t amplitude = 50;

float vReal[samples];
float vImag[samples];

/* Create FFT object */
ArduinoFFT<float> FFT = ArduinoFFT<float>(vReal, vImag, samples, samplingFrequency);

/* To use print vector */
#define SCL_INDEX 0x00
#define SCL_TIME 0x01
#define SCL_FREQUENCY 0x02
#define SCL_PLOT 0x03

void setup() {
  Serial.begin(9600);
  while (!Serial)
    ;
  Serial.println("Ready");
}

void loop() {
  /* Build raw data */
  float ratio = twoPi * signalFrequency / samplingFrequency;
  float ratio2 = twoPi * signalFrequency2 / samplingFrequency;
  float ratio3 = twoPi * signalFrequency3 / samplingFrequency;

  for (uint16_t i = 0; i < samples; i++) {
    vReal[i] = float(amplitude * sin(i * ratio));
    vReal[i] += float(amplitude * sin(i * ratio2));
    vReal[i] += float(amplitude * sin(i * ratio3));

    vImag[i] = 0.0;
  }
  FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward);
  FFT.compute(FFTDirection::Forward); /* Compute FFT */

  FFT.complexToMagnitude(); /* Compute magnitudes */
  PrintVector(vReal, (samples >> 1), SCL_FREQUENCY);

  float maxFreq = maxFrequenza(vReal);
  Serial.print("Dovresti campionare a :");
  Serial.print(maxFreq * 2, 6);
  Serial.println("Hz");
  while (1)
    ; /* Run Once */
}

void PrintVector(float *vData, uint16_t bufferSize, uint8_t scaleType) {
  for (uint16_t i = 0; i < bufferSize; i++) {
    float abscissa;
    /* Print abscissa value */
    switch (scaleType) {
      case SCL_INDEX:
        abscissa = (i * 1.0);
        break;
      case SCL_TIME:
        abscissa = ((i * 1.0) / samplingFrequency);
        break;
      case SCL_FREQUENCY:
        abscissa = ((i * 1.0 * samplingFrequency) / samples);
        break;
    }
    Serial.print(abscissa, 6);
    if (scaleType == SCL_FREQUENCY)
      Serial.print("Hz");
    Serial.print(" ");
    Serial.println(vData[i], 4);
  }
  Serial.println();
}

float maxFrequenza(float *vReal) {
  float soglia = average(vReal);
  float maxIndex = 0;
  for (int i = 0; i < samples / 2; i++) {
    if (vReal[i] > soglia) maxIndex = i;
  }
  return (maxIndex * samplingFrequency) / samples;
}

float average(float *vReal) {
  float average = 0;
  for (int i = 0; i < samples / 2; i++) {
    average += vReal[i];
  }
  return average / samples;
}
