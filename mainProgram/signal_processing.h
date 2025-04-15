#ifndef SIGNAL_PROCESSING_H
#define SIGNAL_PROCESSING_H

#include "config.h"

extern float vReal[];
extern float vImag[];
extern ArduinoFFT<float> FFT;

void generateSyntheticSignal(float *buffer, float *img, uint16_t bufferSize);
float findMaxFrequency(float *vData, float majorPeak);

#endif // SIGNAL_PROCESSING_H