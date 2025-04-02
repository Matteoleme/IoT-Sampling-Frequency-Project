uint32_t samples = 0;
unsigned long lastMillis = 0;

void setup() {
  Serial.begin(9600);
}

void loop() {
  int val = analogRead(7);
  samples++;

  // Print each second
  if (millis() - lastMillis >= 1000) {
    Serial.print("ADC: ");
    Serial.print(samples);
    Serial.println(" samples/s");
    millis = 0;
    lastMillis = millis();
  }
}
