#include <Arduino.h>

#include <TensorFlowLite_ESP32.h>
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "leak_model_data.h"
#include "scaler_params.h"

#define LED_PIN 25
#define LEAK_THRESHOLD 0.5f
#define TENSOR_ARENA_SIZE (40 * 1024)

// Change this value before uploading:
// 0 = no_leak
// 1 = small_leak
// 2 = medium_leak
// 3 = large_leak
#define DEMO_CASE 3

static tflite::MicroErrorReporter micro_error_reporter;
static tflite::AllOpsResolver resolver;
static uint8_t tensor_arena[TENSOR_ARENA_SIZE];
static tflite::MicroInterpreter* interpreter = nullptr;

const char* FEATURE_NAMES[FEATURE_COUNT] = {
  "rms",
  "peak_frequency_hz",
  "spectral_centroid_hz",
  "low_band_energy",
  "mid_band_energy",
  "high_band_energy",
  "zero_crossing_rate",
  "temperature_c",
  "humidity_pct",
  "vibration_magnitude",
  "vibration_mean",
  "vibration_variance",
  "vibration_trend"
};

float noLeakExample[FEATURE_COUNT] = {
  19126.080078125f, 0.0f, 2033.6163153805533f, 5202217795584.0f, 127890841600.0f, 97976221696.0f,
  0.0551181102362204f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
};

float smallLeakExample[FEATURE_COUNT] = {
  13537.3642578125f, 0.0f, 2221.683068516292f, 522580393984.0f, 1189477875712.0f, 11636592640.0f,
  0.2283464566929134f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
};

float mediumLeakExample[FEATURE_COUNT] = {
  12868.4794921875f, 2500.0f, 2116.74872877399f, 208717791232.0f, 1222450610176.0f, 5179531264.0f,
  0.2440944881889764f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
};

float largeLeakExample[FEATURE_COUNT] = {
  11995.294921875f, 4750.0f, 3055.933445862253f, 105140051968.0f, 547548692480.0f, 547528015872.0f,
  0.4645669291338583f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
};

const char* selectedExampleName() {
  switch (DEMO_CASE) {
    case 0: return "synthetic_no_leak_004.wav";
    case 1: return "synthetic_small_leak_003.wav";
    case 2: return "synthetic_medium_leak_002.wav";
    case 3: return "synthetic_large_leak_018.wav";
    default: return "synthetic_large_leak_018.wav";
  }
}

float* selectedFeatures() {
  switch (DEMO_CASE) {
    case 0: return noLeakExample;
    case 1: return smallLeakExample;
    case 2: return mediumLeakExample;
    case 3: return largeLeakExample;
    default: return largeLeakExample;
  }
}

void setupModel() {
  const tflite::Model* model = tflite::GetModel(g_leak_model_data);

  static tflite::MicroInterpreter static_interpreter(
    model,
    resolver,
    tensor_arena,
    TENSOR_ARENA_SIZE,
    &micro_error_reporter
  );

  interpreter = &static_interpreter;

  if (interpreter->AllocateTensors() != kTfLiteOk) {
    Serial.println("AllocateTensors failed");
    while (true) {
      delay(1000);
    }
  }
}

void printFeatureSummary(const float* rawFeatures) {
  Serial.println("FEATURE SUMMARY");
  for (int i = 0; i < FEATURE_COUNT; i++) {
    Serial.print("  ");
    Serial.print(FEATURE_NAMES[i]);
    Serial.print(": ");
    Serial.println(rawFeatures[i], 6);
  }
}

void runEmbeddedDemo() {
  float* features = selectedFeatures();
  float normalized[FEATURE_COUNT];
  normalize_features(features, normalized);

  TfLiteTensor* input = interpreter->input(0);
  for (int i = 0; i < FEATURE_COUNT; i++) {
    input->data.f[i] = normalized[i];
  }

  if (interpreter->Invoke() != kTfLiteOk) {
    Serial.println("Invoke failed");
    digitalWrite(LED_PIN, LOW);
    return;
  }

  float probability = interpreter->output(0)->data.f[0];
  bool ledOn = probability >= LEAK_THRESHOLD;

  digitalWrite(LED_PIN, ledOn ? HIGH : LOW);

  Serial.println("========================================");
  Serial.println("EMBEDDED ML DEMO");
  Serial.print("Example File : ");
  Serial.println(selectedExampleName());
  printFeatureSummary(features);
  Serial.print("Leak Prob.   : ");
  Serial.println(probability, 6);
  Serial.print("Prediction   : ");
  Serial.println(ledOn ? "LEAK" : "NO LEAK");
  Serial.print("LED State    : ");
  Serial.println(ledOn ? "ON" : "OFF");
  Serial.println("========================================");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  setupModel();
}

void loop() {
  runEmbeddedDemo();
  delay(5000);
}
