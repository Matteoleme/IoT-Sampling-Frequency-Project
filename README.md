# IoT-Individual Assignment

## Project Overview

This project is an individual assignment for the "IoT Algorithms and Services" course. The goal is to develop an IoT system that collects data from a sensor, processes the data locally, and communicates an aggregated value to a nearby server. The system optimizes its sampling frequency to save energy and reduce communication overhead. The IoT device is based on an ESP32 prototype board, and the firmware is developed using FreeRTOS.

## Input Signal

The input signal used in this project is synthetically generated. It consists of the sum of three sine functions:

-   First sine wave: frequency = 1 Hz
    
-   Second sine wave: frequency = 7 Hz
    
-   Third sine wave: frequency = 20 Hz
    

All sine waves have an amplitude of 10.

## Determining the Maximum Sampling Frequency

To determine the maximum sampling frequency of the ESP32, a simple program was executed. This program continuously reads an analog value in a loop and stores it in a variable. Since no sensor was connected to the analog pin, the reading was essentially a dummy value.

Each time a reading occurred, a counter was incremented. Using the `millis()` function, the counter value was printed every second, representing the number of analog samples that could be read per second. The maximum sampling frequency found was **32,500 Hz**.

[Code](https://github.com/Matteoleme/IoT-Sampling-Frequency-Project/blob/main/programs/findMaxFrequency.ino)

## Identifying the Optimal Sampling Frequency

Once the maximum sampling frequency was determined, I was sure that I could analyze signal with about 15000 Hz maximum frequency. The Arduino FFT library was employed to analyze the sampled data and compute the Fast Fourier Transform (FFT). The program follows these steps:

1. Instantiate the Arduino FFT object with the found frequency.
 
2. Construct the raw signal without any delay, to take samples at the maximum frequency available, and store it in the `vReal` vector.
    
3.  Execute the necessary library functions to compute the FFT and obtain the frequency magnitudes.

4.  Identify and print the most significant frequency (the highest peak above a certain threshold, excluding the zero frequency component).

### Frequency Identification Function

The function for identifying the dominant frequency works as follows:

-   Computes the *FFT.majorPeak()* of the `vReal` vector, which now represents the magnitude of each frequency.
    
-   Identifies the most distant frequency from zero that is below the peak of 50%.
    

This frequency is then used to sample the input signal with the correct sampling rate for efficient data acquisition.

## Computing Aggregate Function and Communicating to the Server

After executing the FFT, the identified frequency was approximately **139Hz**. According to the sampling theorem, a signal can be correctly reconstructed if sampled at twice (I added a little margin) its highest frequency component. Since the highest frequency in the generated signal was **63Hz**, the calculated sampling frequency aligns correctly with the theory.

Now, using the identified frequency (139Hz), the signal is sampled at a lower frequency to reduce energy consumption.

![FoundFrequency](https://raw.githubusercontent.com/Matteoleme/IoT-Sampling-Frequency-Project/refs/heads/main/media/FoundFreq.png)

### Aggregate value
In this project, the aggregated value is the **maximum** of the collected samples over a fixed time window.

The window duration is defined by the variable `windowSize`, which represents the time (in seconds) over which data is collected.  
The number of samples in each window is calculated as:

    samplesWindowSize = windowSize * samplingFrequency;


After collecting `samplesWindowSize` values, the aggregated value is then transmitted via **MQTT** to a server. The ESP32 is connected to Wi-Fi and uses the [`PubSubClient`](https://docs.arduino.cc/libraries/pubsubclient/) library to connect to the MQTT server. The computed value is published to the topic **"matteo/max"**.

### Infrastructure Setup

The following infrastructure is used for communication:

1.  **Local MQTT Server**: Raspberry Pi running Mosquitto
    
2.  **Client Publisher**: ESP32
    
3.  **Client Subscriber**: Raspberry Pi or PC with Mosquitto client

![Mqtt infrastructure](https://raw.githubusercontent.com/Matteoleme/IoT-Sampling-Frequency-Project/refs/heads/main/media/mqtt_infrastructure.png)    

### Subscribing to the Topic

To subscribe to the topic, the following command can be used:

    mosquitto_sub -h <server> -p <port> -t "matteo/max"

## System Architecture & Task Overview

This project is structured using FreeRTOS tasks, semaphores, and a queue to manage concurrent operations on the ESP32. Here's a breakdown of the key components and their responsibilities:

### 1. `readSignalTask`

-   Generates a synthetic signal used for frequency analysis via FFT.
    
-   Triggers the next task once the signal is ready.
    

### 2. `fftProcessTask`

-   Performs a Fast Fourier Transform (FFT) to detect the highest frequency component of the signal.
    
-   Based on the FFT result, it calculates the minimum required sampling frequency (Nyquist + 10% margin).
    
-   Allocates memory for a window buffer used to store signal values over a fixed time interval.
    

### 3. `continuousSamplingTask`

-   Samples the signal at the computed frequency.
	- I used the `vTaskDelay()` function to pause the task between samples, simulating the time interval that would naturally occur when sampling a real signal at a fixed frequency. This avoids busy-waiting and allows other tasks to run in the meantime.
    
-   Each value is pushed into a FreeRTOS queue shared with the window buffer task.
    

### 4. `windowBufferTask`

-   Reads values from the queue and stores them in a circular buffer (`windowBuffer`).
    
-   Once the buffer is full (based on `samplesWindowSize`), it signals the next task.
    

### 5. `windowProcessTask`

-   Waits for a full buffer, then computes the maximum value in the time window.
    
-   Publishes the maximum via MQTT to the topic `"matteo/max"`.
    

### Synchronization Tools

-   Semaphores ensure correct execution order between signal generation, FFT, and window processing.
    
-   Queues are used to pass sampled values between tasks safely.

## [Communicating the Aggregate Value to the Cloud](https://github.com/Matteoleme/IoT-Sampling-Frequency-Project/blob/main/programs/AvgToLoRa.ino)

The same logic used before was applied in this step. However, instead of sending the data via Wi-Fi, the ESP32 transmits the aggregated value using **LoRa** to The Things Network (TTN).

### Infrastructure Setup

1.  **ESP32 LoRa Connection**:
    
    -   The ESP32 was connected to TTN.
        
    -   After establishing the connection, the necessary credentials were obtained from TTN and integrated into the program.
        
    -   The [`EzLoRaWAN.h`](https://github.com/rgot-org/EzLoRaWan) library was used to send messages to the LoRaWAN infrastructure.
        
    -   The ESP32 was tested at Sapienza University, San Pietro in Vincoli, where it connected to a nearby LoRa gateway.
        
2.  **PC MQTT Client**:
    
    -   Using TTN, the MQTT server details were retrieved.
        
    -   The following command was used to subscribe to the topic and receive the transmitted data:
        
    
    ```
    mosquitto_sub -h eu1.cloud.thethings.network -p 1883 -t "#" -u "appName@ttn" -P "API_KEY" -d
    ```

### Respose example
Part of the message I received 
```
"received_at": "2025-04-03T12:36:34.269817643Z",
"uplink_message": {
"session_key_id": "AZX7poOIR70Fhnx4VhmGhA==",
"f_port": 1,
"f_cnt": 4,
"frm_payload": "AQI/Ag==",
"decoded_payload": {
"analog_in_1": 161.3
},
"rx_metadata": [
{
"gateway_ids": {
"gateway_id": "spv-rooftop-panorama",
"eui": "AC1F09FFFE057698"
},
"timestamp": 938868292,
"rssi": -107,
"channel_rssi": -107,
"snr": 4.8,
"location": {
"latitude": 41.8937,
"longitude": 12.49399,
"altitude": 55,
"source": "SOURCE_REGISTRY"
},
"uplink_token": "CiIKIAoUc3B2LXJvb2Z0b3AtcGFub3JhbWESCKwfCf/+BXaYEMT8178DGgsI0oG6vwYQpICOHiCg86THqb6bAQ==",
"channel_index": 6,
"received_at": "2025-04-03T12:36:34.040594204Z"
}],
```


## Measure Energy Consumption 

To measure energy consumption, I used the following setup:

-   **Monitor**: An ESP-WROOM-32 connected via **SDA** and **SCL** to an **INA219** sensor to monitor power usage.
	- [Firmware](https://learn.adafruit.com/adafruit-ina219-current-sensor-breakout/wiring)
    
-   **Load**: The main ESP32 LoRa V3 device running the firmware described in this project.

![Power measurement infrastructure](https://raw.githubusercontent.com/Matteoleme/IoT-Sampling-Frequency-Project/refs/heads/main/media/PowerMesurement.jpg)
### Initial Attempts

At the beginning, I used `vTaskDelay()` to reduce CPU usage between samples. While this reduced power consumption slightly, it was not as effective as expected compared to using light or deep sleep modes.

I then tested a different approach: entering light sleep between each sampling cycle, based on the computed sampling frequency. However, this led to two major issues:

1.  **Unsuitable for the frequency range**: At those frequencies, the time and energy required to repeatedly enter and exit sleep mode made the approach inefficient.
    
2.  **Wi-Fi instability**: Entering light sleep caused the ESP32 to disconnect from Wi-Fi, leading to inconsistent MQTT publishing. Sometimes the device was unable to reconnect, making communication unreliable and inefficient (due to frequent re-authentication).

```
float samplingIntervalSec = 1.0 / samplingFrequency;
uint64_t samplingIntervalMicros = samplingIntervalSec * 1e6;
esp_sleep_enable_timer_wakeup(samplingIntervalMicros);
esp_light_sleep_start();
```
[
![Light Sleep behaviour]([https://i.sstatic.net/Vp2cE.png](https://img.freepik.com/free-vector/modern-flat-style-clean-white-video-player-template_1017-25482.jpg))](https://drive.google.com/file/d/16RiuPJs4-Rk6Lq4jKIH2b_k_tzCFZ9ze/view?usp=sharing)
### Final Approach


-   Use `vTaskDelay()` during continuous sampling.
    
-   After publishing 5 aggregated values via MQTT, the device enters **deep sleep** for 15 seconds to save energy.
    
-   Before entering deep sleep, the optimized *sampling frequency and a flag* to skip FFT on boot are *saved in RTC memory*.
    

This way, when the ESP32 wakes up:

-   It does not recompute the FFT.
    
-   It *immediately starts sampling at the correct frequency*, reducing startup time and energy usage.
    

This approach provided a good trade-off between performance, reliability, and energy efficiency.

### Power values

 - Sampling: 130 mW
 - Send value to MQTT server: 350 mW
 - Deep sleep: 4mW
 
![PowerConsumptionGraph](https://raw.githubusercontent.com/Matteoleme/IoT-Sampling-Frequency-Project/refs/heads/main/media/GraphEnergyConsumption.jpg)
[
![Deep Sleep energy consumption]([https://i.sstatic.net/Vp2cE.png](https://img.freepik.com/free-vector/modern-flat-style-clean-white-video-player-template_1017-25482.jpg))](https://drive.google.com/file/d/16OsdTBBzRtsf416MMIjxJvz8iMgTNrcx/view?usp=sharing)

### **Evaluating Data Transmission Volume**


In this project, the number of samples collected depends on the sampling frequency, since the goal is to capture a fixed time window (e.g., 5 seconds). As a result, increasing the sampling frequency leads to collecting more samples within the same time period.

While an oversampled system captures data at a higher rate, it does not increase the amount of data transmitted over the network. The adaptive sampling strategy adjusts the interval between samples and the number of samples accordingly, but in the end, only one aggregated value (the maximum in this case) is sent over Wi-Fi, not the entire `vReal` vector.

### **Measuring End-to-End Latency**

To evaluate the end-to-end latency of the system, I measured the time from when the ESP32 sends the aggregated value until it receives an acknowledgment (ACK) from the MQTT subscriber. The approach is based on calculating the Round Trip Time (RTT) and dividing it by 2 to estimate the one-way latency.

#### **Measurement Approach:**

1.  **ESP32 (Publisher):**
    
    -   After computing the FFT and extracting the average value, the ESP32 publishes the result to the MQTT broker on the topic `"matteo/FFT/avg"`.
        
    -   Immediately after publishing, it starts a timer.
        
    -   It listens for an ACK message on the `"ack_channel"` topic.
        
    -   Upon receiving the ACK, the ESP32 stops the timer and calculates RTT.
        
2.  **MQTT Broker (Raspberry Pi):**
    
    -   Acts as both the MQTT server and the subscriber to `"matteo/FFT/avg"`.
        
    -   Upon receiving a message on `"matteo/FFT/avg"`, it immediately responds by publishing an acknowledgment on `"ack_channel"`.
                

To configure the Raspberry Pi to automatically send an ACK upon receiving a message, I used the following command:

    mosquitto_sub -t "matteo/FVT/avg" | xargs -I{} mosquitto_pub -t "ack_channel" -m {}

[![MQTT Latency video]([https://i.sstatic.net/Vp2cE.png](https://img.freepik.com/free-vector/modern-flat-style-clean-white-video-player-template_1017-25482.jpg))](https://drive.google.com/file/d/1Ixhvbo92gGXvTQKjJ2w-WtfCB5ZSCrPN/view?usp=sharing)

### **Performance Analysis with Different Input Signals**
To obtain a clean measurement of the impact of sampling frequency on energy consumption, I wrote a simplified version of the program. In this version, the ESP32 only samples the signal and enters light sleep between samples. No FFT or aggregated value is computed—just pure periodic sampling.

I ran two tests using the same input signal:

-   A high sampling frequency of **15,000 Hz**
    
-   A low sampling frequency of **12 Hz**
    

Using the same measurement setup as before (INA219 connected to a second ESP32 acting as a monitor), I observed a clear difference in power consumption:

-   At **15,000 Hz**, the system consumed approximately **240 mW**
![HighFreqResult](https://raw.githubusercontent.com/Matteoleme/IoT-Sampling-Frequency-Project/refs/heads/main/media/HighFreqSamplingResult.png)
    
-   At **12 Hz**, the power consumption dropped to around **16 mW**
![LowFreqResult](https://raw.githubusercontent.com/Matteoleme/IoT-Sampling-Frequency-Project/refs/heads/main/media/LowFreqSamplingResult.png)    

These results confirm that lowering the sampling frequency leads to significantly lower energy consumption, especially when combined with power-saving techniques like light sleep between samples.

This energy saving becomes even more significant when the input signal changes slowly over time, allowing the microcontroller to remain in sleep mode for longer periods without missing important variations.
