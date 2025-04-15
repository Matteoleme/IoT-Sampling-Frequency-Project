#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include "config.h"
#include "signal_processing.h"
#include "wifi_mqtt.h"
#include "esp_sleep.h"

// Tasks handle
extern TaskHandle_t readSignalHandle;
extern TaskHandle_t fftProcessHandle;
extern TaskHandle_t continuousSamplingHandle;
extern TaskHandle_t windowBufferHandle;
extern TaskHandle_t windowProcessHandle;
extern TaskHandle_t monitorHandle;

// Semaphore
extern SemaphoreHandle_t dataReadySemaphore;
extern SemaphoreHandle_t processingDoneSemaphore;
extern SemaphoreHandle_t windowReadySemaphore;
extern QueueHandle_t dataQueue;

// Control flags
extern volatile bool firstRun;
extern volatile bool dataReady;
extern volatile bool processingDone;
extern volatile bool runFFT;
extern volatile bool optimizedFrequencyFound;

// Tasks
void readSignalTask(void *pvParameters);
void fftProcessTask(void *pvParameters);
void continuousSamplingTask(void *pvParameters);
void windowBufferTask(void *pvParameters);
void windowProcessTask(void *pvParameters);

void initializeTasks();

#endif // TASK_MANAGER_H