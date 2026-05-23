#include <Arduino.h>

#define LED_PIN 25
#define LEAK_THRESHOLD 0.5f

// If the LED behavior is backwards, change HIGH to LOW.
#define LED_ON_LEVEL HIGH

// 0 = no leak
// 1 = small leak
// 2 = medium leak
// 3 = large leak
#define DEMO_CASE 0

struct DemoResult {
  const char* file_name;
  const char* expected_label;
  float leak_probability;
};

static DemoResult DEMO_RESULTS[] = {
  {"synthetic_no_leak_004.wav", "NO LEAK", 0.00060515566f},
  {"synthetic_small_leak_003.wav", "LEAK", 0.98732800f},
  {"synthetic_medium_leak_002.wav", "LEAK", 0.99783677f},
  {"synthetic_large_leak_018.wav", "LEAK", 0.99955770f}
};

void setResultLed(bool leak_detected) {
  digitalWrite(LED_PIN, leak_detected ? LED_ON_LEVEL : (LED_ON_LEVEL == HIGH ? LOW : HIGH));
}

void runDemoOnce() {
  DemoResult* example = &DEMO_RESULTS[DEMO_CASE];
  bool leak_detected = example->leak_probability >= LEAK_THRESHOLD;

  setResultLed(leak_detected);

  Serial.println("========================================");
  Serial.println("EMBEDDED ML RESULTS DEMO");
  Serial.println("Saved outputs from trained model");
  Serial.println("========================================");
  Serial.print("Demo Case      : ");
  Serial.println(DEMO_CASE);
  Serial.print("Example File   : ");
  Serial.println(example->file_name);
  Serial.print("Expected Class : ");
  Serial.println(example->expected_label);
  Serial.print("Leak Prob.     : ");
  Serial.println(example->leak_probability, 6);
  Serial.print("Prediction     : ");
  Serial.println(leak_detected ? "LEAK" : "NO LEAK");
  Serial.print("LED State      : ");
  Serial.println(leak_detected ? "ON" : "OFF");
  Serial.println("========================================");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LED_PIN, OUTPUT);
  setResultLed(false);
}

void loop() {
  runDemoOnce();
  delay(5000);
}
