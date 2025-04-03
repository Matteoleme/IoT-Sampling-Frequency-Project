# IoT-Individual Assignment

## Project Overview

This project is an individual assignment for the "IoT Algorithms and Services" course. The goal is to develop an IoT system that collects data from a sensor, processes the data locally, and communicates an aggregated value to a nearby server. The system optimizes its sampling frequency to save energy and reduce communication overhead. The IoT device is based on an ESP32 prototype board, and the firmware is developed using FreeRTOS.

## Input Signal

The input signal used in this project is synthetically generated. It consists of the sum of three sine functions:

-   First sine wave: frequency = 10 Hz
    
-   Second sine wave: frequency = 300 Hz
    
-   Third sine wave: frequency = 1200 Hz
    

All sine waves have an amplitude of 50.

## Determining the Maximum Sampling Frequency

To determine the maximum sampling frequency of the ESP32, a simple program was executed. This program continuously reads an analog value in a loop and stores it in a variable. Since no sensor was connected to the analog pin, the reading was essentially a dummy value.

Each time a reading occurred, a counter was incremented. Using the `millis()` function, the counter value was printed every second, representing the number of analog samples that could be read per second. The maximum sampling frequency found was **32,500 Hz**.

## Identifying the Optimal Sampling Frequency

Once the maximum sampling frequency was determined, it was used to sample the input signal. The Arduino FFT library was employed to analyze the sampled data and compute the Fast Fourier Transform (FFT). The program follows these steps in the loop:

1.  Construct the raw signal and store it in the `vReal` vector.
    
2.  Execute the necessary library functions to compute the FFT and obtain the frequency magnitudes.
    
3.  Print the `vReal` vector containing the frequency components.
    
4.  Identify and print the most significant frequency (the highest peak above a certain threshold, excluding the zero frequency component).
    

### Frequency Identification Function

The function for identifying the dominant frequency works as follows:

-   Computes the average of the `vReal` vector, which now represents the magnitude of each frequency.
    
-   Identifies the most distant frequency from zero that exceeds this average threshold.
    

This frequency is then used to re-run the program with the correct sampling rate for efficient data acquisition.

## Computing Aggregate Function and Communicating to the Server

After executing this program, the identified frequency was approximately **2475 Hz**. According to the sampling theorem, a signal can be correctly reconstructed if sampled at twice its highest frequency component. Since the highest frequency in the generated signal was **1200 Hz**, the calculated sampling frequency aligns correctly with the theory.

Now, using the identified frequency (2500 Hz), the signal is sampled at a lower frequency to reduce energy consumption. After computing the Fourier Transform of the signal again, the average value of the `vReal` vector is calculated over a time window equal to `samples/samplingFrequency`. In this case, the window is approximately **0.4 seconds**. We could increase the time window by using more samples.

This aggregated value is then transmitted via **MQTT** to a server. The ESP32 is connected to Wi-Fi and uses the [`PubSubClient`](https://docs.arduino.cc/libraries/pubsubclient/) library to connect to the MQTT server. The computed value is published to the topic **"matteo/FFT/avg"**.

### Infrastructure Setup

The following infrastructure is used for communication:

1.  **Local MQTT Server**: Raspberry Pi running Mosquitto
    
2.  **Client Publisher**: ESP32
    
3.  **Client Subscriber**: Raspberry Pi or PC with Mosquitto client
    

### Subscribing to the Topic

To subscribe to the topic, the following command can be used:

    mosquitto_sub -h <server> -p <port> -t "matteo/FFT/avg"*

## [Communicating the Aggregate Value to the Cloud](https://github.com/Matteoleme/IoT-Sampling-Frequency-Project/blob/main/programs/AvgToLoRa.ino)

The same logic used for computing the FFT and calculating the average was applied in this step. However, instead of sending the data via Wi-Fi, the ESP32 transmits the aggregated value using **LoRa** to **The Things Network (TTN)**.

### Infrastructure Setup

1.  **ESP32 LoRa Connection**:
    
    -   The ESP32 was connected to **TTN**.
        
    -   After establishing the connection, the necessary credentials were obtained from TTN and integrated into the program.
        
    -   The [`EzLoRaWAN.h`](https://github.com/rgot-org/EzLoRaWan) library was used to send messages to the LoRaWAN infrastructure.
        
    -   The ESP32 was tested at **Sapienza University, San Pietro in Vincoli**, where it connected to a nearby LoRa gateway.
        
2.  **PC MQTT Client**:
    
    -   Using **TTN**, the MQTT server details were retrieved.
        
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
