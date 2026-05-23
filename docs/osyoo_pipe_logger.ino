#include <Arduino.h>

#define SOUND_PIN 34
#define LED_PIN 25

// Update these before each recording session.
const char* SESSION_ID = "session01";
const char* LABEL = "no_leak";   // use "no_leak" or "leak"

const int SAMPLE_WINDOW_MS = 100;
const int REPORT_DELAY_MS = 150;
const int LIVE_THRESHOLD = 300;

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.println("session_id,label,millis,signal_min,signal_max,signal_mean,signal_std,signal_range,led_state");
}

void loop() {
  int signalMin = 4095;
  int signalMax = 0;
  long sampleSum = 0;
  long sampleSquareSum = 0;
  int sampleCount = 0;

  unsigned long startMillis = millis();

  while (millis() - startMillis < SAMPLE_WINDOW_MS) {
    int sample = analogRead(SOUND_PIN);

    if (sample < signalMin) signalMin = sample;
    if (sample > signalMax) signalMax = sample;

    sampleSum += sample;
    sampleSquareSum += (long)sample * (long)sample;
    sampleCount++;
  }

  float signalMean = (sampleCount > 0) ? (float)sampleSum / (float)sampleCount : 0.0f;
  float meanSquare = (sampleCount > 0) ? (float)sampleSquareSum / (float)sampleCount : 0.0f;
  float variance = meanSquare - (signalMean * signalMean);
  if (variance < 0.0f) variance = 0.0f;
  float signalStd = sqrt(variance);

  int signalRange = signalMax - signalMin;
  bool ledOn = signalRange >= LIVE_THRESHOLD;
  digitalWrite(LED_PIN, ledOn ? HIGH : LOW);

  Serial.print(SESSION_ID);
  Serial.print(",");
  Serial.print(LABEL);
  Serial.print(",");
  Serial.print(millis());
  Serial.print(",");
  Serial.print(signalMin);
  Serial.print(",");
  Serial.print(signalMax);
  Serial.print(",");
  Serial.print(signalMean, 2);
  Serial.print(",");
  Serial.print(signalStd, 2);
  Serial.print(",");
  Serial.print(signalRange);
  Serial.print(",");
  Serial.println(ledOn ? "ON" : "OFF");

  delay(REPORT_DELAY_MS);
}
