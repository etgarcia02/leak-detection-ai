#include <Arduino.h>

#define SOUND_PIN 34
#define LED_PIN 25

// Tune this after testing on your classroom pipe.
const int LIVE_THRESHOLD = 300;
const int SAMPLE_WINDOW_MS = 120;
const int REPORT_DELAY_MS = 800;

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.println("========================================");
  Serial.println("LIVE PIPE SENSING DEMO");
  Serial.println("Monitor values while changing pipe flow.");
  Serial.println("========================================");
}

void loop() {
  int signalMin = 4095;
  int signalMax = 0;
  long sampleSum = 0;
  int sampleCount = 0;

  unsigned long startMillis = millis();

  while (millis() - startMillis < SAMPLE_WINDOW_MS) {
    int sample = analogRead(SOUND_PIN);

    if (sample < signalMin) signalMin = sample;
    if (sample > signalMax) signalMax = sample;

    sampleSum += sample;
    sampleCount++;
  }

  float signalMean = (sampleCount > 0) ? (float)sampleSum / (float)sampleCount : 0.0f;
  int signalRange = signalMax - signalMin;

  const char* status = "NORMAL FLOW / NO LEAK";
  bool ledOn = false;

  if (signalRange >= LIVE_THRESHOLD) {
    status = "POSSIBLE LEAK / STRONG SOUND";
    ledOn = true;
  }

  digitalWrite(LED_PIN, ledOn ? HIGH : LOW);

  Serial.println("----------------------------------------");
  Serial.print("Signal Min   : ");
  Serial.println(signalMin);
  Serial.print("Signal Max   : ");
  Serial.println(signalMax);
  Serial.print("Signal Mean  : ");
  Serial.println(signalMean, 2);
  Serial.print("Signal Range : ");
  Serial.println(signalRange);
  Serial.print("Threshold    : ");
  Serial.println(LIVE_THRESHOLD);
  Serial.print("Status       : ");
  Serial.println(status);
  Serial.print("LED State    : ");
  Serial.println(ledOn ? "ON" : "OFF");

  delay(REPORT_DELAY_MS);
}
