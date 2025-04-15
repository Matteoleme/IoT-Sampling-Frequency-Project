#include "signal_processing.h"

// Arrays and FFT object
float vReal[samples];
float vImag[samples];
ArduinoFFT<float> FFT = ArduinoFFT<float>(vReal, vImag, samples, samplingFrequency);
uint16_t samplingFrequency = maxFrequencyAvailable;  // Initially max, will be optimized

void generateSyntheticSignal(float *buffer, float *img, uint16_t bufferSize) {
  float ratio = TWO_PI * signalFrequency / samplingFrequency;
  float ratio2 = TWO_PI * signalFrequency2 / samplingFrequency;
  float ratio3 = TWO_PI * signalFrequency3 / samplingFrequency;
  
  for (uint16_t i = 0; i < bufferSize; i++) {
    buffer[i] = float(amplitude * sin(i * ratio));
    buffer[i] += float(amplitude * sin(i * ratio2));
    buffer[i] += float(amplitude * sin(i * ratio3));
    img[i] = 0.0;
    
    if (i % 100 == 0) vTaskDelay(1);
  }
}

float findMaxFrequency(float *vData, float majorPeak) {
  float threshold = majorPeak * 0.5;
  float maxIndex = 0;
  
  for (int i = 1; i < samples / 2; i++) {
    if (vData[i] > threshold) maxIndex = i;
  }
  
  return (maxIndex * samplingFrequency) / samples;
}