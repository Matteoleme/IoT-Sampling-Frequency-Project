#include <arduinoFFT.h>

#include "arduinoFFT.h"

/* Signal constants */
const uint16_t samples = 1024;
const float signalFrequency = 10;
const float signalFrequency2 = 300;
const float signalFrequency3 = 1200;
const uint16_t samplingFrequency = 2500;
const uint16_t amplitude = 50;

float vReal[samples];
float vImag[samples];

/* Create FFT object */
ArduinoFFT<float> FFT = ArduinoFFT<float>(vReal, vImag, samples, samplingFrequency);

/* Connection variables */
#include <WiFi.h>
#include "PubSubClient.h"


// WiFi Setup
const char* ssid = "";
const char* psw = "";

//MQTT Setup
const char* mqttServer = "";
int port = 1883;
long lastMsg = 0;
String stMac;
char mac[50];
char clientId[50];

// Topics
const char* pubTopic = "matteo/FFT/avg";
const char* ackTopic = "ack_channel";

#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long t1 = 0;

void setup()
{
  Serial.begin(9600);
  while(!Serial);

  // Wifi connection
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, psw);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  client.setServer(mqttServer, port);
  client.setCallback(callback);
  
  Serial.println(" Connected!");
  delay(2500);

  Serial.println("Ready");
}


void mqttReconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    long r = random(1000);
    sprintf(clientId, "clientId-%ld", r);
    if (client.connect(clientId)) {
      Serial.print(clientId);
      Serial.println(" connected");
      client.subscribe(ackTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Compute latency when the ack is received
  if (String(topic) == ackTopic) {
    unsigned long t3 = millis();
    unsigned long rtt = t3 - t1;
    float latency = rtt / 2.0; // Latenza one-way
    Serial.print("Latenza: ");
    Serial.print(latency);
    Serial.println(" ms");
  }
}


void loop()
{
  // connection
  delay(10);
  int val = 0;
  if (!client.connected()) {
    mqttReconnect();
  }
  client.loop();

  /* Build raw data */
  float ratio = twoPi * signalFrequency / samplingFrequency;
  float ratio2 = twoPi * signalFrequency2 / samplingFrequency;
  float ratio3 = twoPi * signalFrequency3 / samplingFrequency;

  for (uint16_t i = 0; i < samples; i++)
  {
    vReal[i] = float(amplitude * sin(i * ratio));
    vReal[i] += float(amplitude * sin(i * ratio2));
    vReal[i] += float(amplitude * sin(i * ratio3));

    vImag[i] = 0.0;

  }
  FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward);
  FFT.compute(FFTDirection::Forward); /* Compute FFT */

  FFT.complexToMagnitude(); /* Compute magnitudes */

  // compute average value
  float average = avg(vReal);

  // send the value to the mqtt server
  t1 = millis(); // save timestamp before send the message
  snprintf (msg, MSG_BUFFER_SIZE, "%f time: %lu", average, t1);
  int result = client.publish(pubTopic, msg);
  Serial.print("Publish message: ");
  Serial.println(msg);
  Serial.println(result);
  delay(5000);
}


float avg(float *vReal){
  float average = 0;
  for(int i=0; i<samples/2; i++){
    average += vReal[i];
  }
  return average / samples;
}
