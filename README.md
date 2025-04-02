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
