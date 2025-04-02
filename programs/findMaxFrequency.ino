uint32_t campioni = 0;
unsigned long ultimoMillis = 0;

void setup() {
  Serial.begin(9600);
}

void loop() {
  int val = analogRead(7);
  campioni++;

  // Stampo ogni secondo
  if (millis() - ultimoMillis >= 1000) {
    Serial.print("Campionamento ADC: ");
    Serial.print(campioni);
    Serial.println(" campioni/s");
    campioni = 0;
    ultimoMillis = millis();
  }
}
