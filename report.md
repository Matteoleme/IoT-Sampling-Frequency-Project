# Development Timeline

## 1. Initial Testing on Wokwi

At the beginning, I tested various example codes provided by the professor and the official Arduino FFT library on Wokwi. This helped me understand the functionality of the FFT algorithm on simple signals. Once I had a grasp of how it worked with basic functions, I moved on to more complex signals, where I noticed that using majorPeak did not always yield useful results.

## 2. Finding the Relevant Frequency

Since the goal was to determine the highest relevant frequency rather than the peak amplitude, I modified the approach. I wrote a function that processes the vReal vector after performing the Fourier Transform and identifies the highest frequency that surpasses a threshold. Initially, I used a fixed threshold, but I soon realized that it was not generalizable across different signals. To improve accuracy, I implemented a dynamic threshold based on the average of all values in vReal.

## 3. Implementing Adaptive Sampling

With the ability to determine the dominant frequency, I proceeded to implement an adaptive sampling mechanism:

First, I wrote a program that samples the signal at a high frequency to accurately detect the dominant frequency.

Using the detected frequency, I developed another program that samples at twice this value, applying the Nyquist theorem to ensure proper signal reconstruction.

## 4. Sending Aggregated Data via MQTT

Once I had the sampled data, I computed the mean of the vReal vector for aggregation. To test the communication, I set up an MQTT broker on my PC using WSL. Since the ESP32 needed to send data externally, I configured Ngrok to expose the MQTT service. I then successfully published the processed value via MQTT and verified the data reception using an MQTT client on WSL.
