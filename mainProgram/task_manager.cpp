#include "task_manager.h"

// Handler task
TaskHandle_t readSignalHandle = NULL;
TaskHandle_t fftProcessHandle = NULL;
TaskHandle_t continuousSamplingHandle = NULL;
TaskHandle_t windowBufferHandle = NULL;
TaskHandle_t windowProcessHandle = NULL;

// Semaphore
SemaphoreHandle_t dataReadySemaphore = NULL;
SemaphoreHandle_t processingDoneSemaphore = NULL;
SemaphoreHandle_t windowReadySemaphore = NULL;
QueueHandle_t dataQueue = NULL;

// Control flags
RTC_DATA_ATTR volatile bool dataReady = false;
RTC_DATA_ATTR volatile bool processingDone = false;
RTC_DATA_ATTR volatile bool runFFT = true;
RTC_DATA_ATTR volatile bool optimizedFrequencyFound = false;

// Circular buffer
float *windowBuffer;
float samplesWindowSize;
volatile uint16_t buffIndex = 0;

// temp var to store found frequency
RTC_DATA_ATTR uint16_t registerSamplingFreq;

void readSignalTask(void *pvParameters) {
  Serial.println("Started signal generation task for FFT");

  // Generate synthetic signal for FFT
  generateSyntheticSignal(vReal, vImag, samples);

  Serial.println("Signal generation for FFT complete");
  dataReady = true;
  xSemaphoreGive(dataReadySemaphore);
  vTaskDelete(NULL);
}

void fftProcessTask(void *pvParameters) {
  Serial.println("Started FFT processing task");

  if (xSemaphoreTake(dataReadySemaphore, portMAX_DELAY) == pdTRUE && runFFT) {
    Serial.println("Processing signal with FFT...");

    // Perform FFT
    FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward);
    FFT.compute(FFTDirection::Forward);
    FFT.complexToMagnitude();
    float majorPeak = FFT.majorPeak();
    // Find max frequency and optimize sampling rate
    float maxFreq = findMaxFrequency(vReal, majorPeak);
    float newSamplingFreq = maxFreq * 2 * 1.1;  // 10% margin

    Serial.print("Found max frequency: ");
    Serial.print(maxFreq);
    Serial.print(" Hz, New sampling frequency: ");
    Serial.println(newSamplingFreq);

    samplingFrequency = newSamplingFreq;
    registerSamplingFreq = newSamplingFreq;

    // Signal that FFT processing is done
    processingDone = true;
    xSemaphoreGive(processingDoneSemaphore);

    // set the flags
    optimizedFrequencyFound = true;
    runFFT = false;

    // once I found the optimized frequency, I can calculate how many samples for 5s
    samplesWindowSize = windowSize * samplingFrequency;
    Serial.print("# samples to compute: ");
    Serial.println(samplesWindowSize);

    windowBuffer = (float *)malloc(samplesWindowSize * (sizeof(float)));
    if (windowBuffer == NULL) {
      Serial.println("ERROR: Failed to allocate window buffer!");
      while (1);
    }
    // Start continuous sampling tasks
    xTaskCreatePinnedToCore(
      continuousSamplingTask, "ContinuousSampling",
      4096, NULL, 2, &continuousSamplingHandle, 0);

    xTaskCreatePinnedToCore(
      windowBufferTask, "WindowBuffer",
      4096, NULL, 1, &windowBufferHandle, 1);

    xTaskCreatePinnedToCore(
      windowProcessTask, "WindowProcess",
      4096, NULL, 1, &windowProcessHandle, 1);
  }

  vTaskDelete(NULL);
}

void continuousSamplingTask(void *pvParameters) {
  Serial.println("Started continuous sampling task");
  // Time interval between 2 samples
  float samplingIntervalMs = 1000 / samplingFrequency;
  TickType_t ticks = pdMS_TO_TICKS(samplingIntervalMs);
  if (ticks < 1) ticks = 1;

  Serial.printf("Sampling interval: %d ticks\n", ticks);

  uint32_t sampleCount = 0;

  while (1) {
    // Synthetic signal
    float time = sampleCount * (1.0 / samplingFrequency);
    float value = amplitude * sin(TWO_PI * signalFrequency * time);
    value += amplitude * sin(TWO_PI * signalFrequency2 * time);
    value += amplitude * sin(TWO_PI * signalFrequency3 * time);

    // Send to the queue
    if (xQueueSend(dataQueue, &value, 0) != pdPASS) {
      if (sampleCount % 100 == 0) {
        Serial.println("WARNING: Queue full, sample dropped");
      }
    }
    sampleCount++;
    vTaskDelay(ticks);
  }
}

void windowBufferTask(void *pvParameters) {
  Serial.println("Started window buffer task");

  float value;
  while (1) {
    // Get data from queue
    if (xQueueReceive(dataQueue, &value, portMAX_DELAY) == pdPASS) {
      // Add to window buffer
      windowBuffer[buffIndex] = value;
      buffIndex++;
      // If window is full, signal processing task
      if (buffIndex >= samplesWindowSize) {
        buffIndex = 0;
        xSemaphoreGive(windowReadySemaphore);
      }
    }
  }
}

void windowProcessTask(void *pvParameters) {
  Serial.println("Started window processing task");

  uint32_t windowCount = 0;

  while (1) {
    // Wait for a full window buffer
    if (xSemaphoreTake(windowReadySemaphore, portMAX_DELAY) == pdTRUE) {
      // Process window data
      float max = windowBuffer[0];

      for (int i = 1; i < samplesWindowSize; i++) {
        if (windowBuffer[i] > max) max = windowBuffer[i];
      }

      // Print window statistics
      Serial.print("Window #");
      Serial.print(windowCount++);
      Serial.print(" - max: ");
      Serial.println(max);

      // Publish to MQTT if connected
      if (WiFi.status() == WL_CONNECTED && mqttClient.connected()) {
        publishMQTTMessage(mqttTopic, max);
      }
      vTaskDelay(1);
    }

    // deep sleep each 5 mqtt message
    if(windowCount%10==5){
      esp_sleep_enable_timer_wakeup(15000000);
      esp_deep_sleep_start();
    }
  }
}

void initializeTasks() {
  // Create semaphores
  dataReadySemaphore = xSemaphoreCreateBinary();
  processingDoneSemaphore = xSemaphoreCreateBinary();
  windowReadySemaphore = xSemaphoreCreateBinary();

  // Create data queue
  dataQueue = xQueueCreate(512, sizeof(float));

  if (!dataReadySemaphore || !processingDoneSemaphore || !windowReadySemaphore || !dataQueue) {
    Serial.println("ERROR: Could not create semaphores or queue!");
    while (1);
  }

  // Start FFT process if needed
  if (runFFT) {
    xTaskCreatePinnedToCore(readSignalTask, "ReadSignal", 4096, NULL, 1, &readSignalHandle, 0);

    xTaskCreatePinnedToCore(fftProcessTask, "FFTProcess", 8192, NULL, 1, &fftProcessHandle, 1);
  } else {
    // Skip FFT and use default sampling frequency
    optimizedFrequencyFound = true;
    samplingFrequency = registerSamplingFreq;     // Take previous computed frequency
    
    // Compute how many samples
    samplesWindowSize = windowSize * samplingFrequency;
    Serial.print("# samples to compute: ");
    Serial.println(samplesWindowSize);
    windowBuffer = (float *)malloc(samplesWindowSize * (sizeof(float)));
    if (windowBuffer == NULL) {
      Serial.println("ERROR: Failed to allocate window buffer!");
      while (1);
    }
    
    // Start continuous sampling directly
    xTaskCreatePinnedToCore(continuousSamplingTask, "ContinuousSampling", 4096, NULL, 2, &continuousSamplingHandle, 0);

    xTaskCreatePinnedToCore(windowBufferTask, "WindowBuffer", 4096, NULL, 1, &windowBufferHandle, 1);

    xTaskCreatePinnedToCore(windowProcessTask, "WindowProcess", 4096, NULL, 1, &windowProcessHandle, 1);
  }
}