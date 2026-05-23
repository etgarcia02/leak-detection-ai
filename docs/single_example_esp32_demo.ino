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

// Change this value to choose which example file to run:
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

// Feature vector from synthetic_no_leak_000.wav
float noLeakExample[FEATURE_COUNT] = {
  15126.54296875f,    // rms
  0.0f,               // peak_frequency_hz
  2207.33736861f,     // spectral_centroid_hz
  3122855673856.0f,   // low_band_energy
  117771091968.0f,    // mid_band_energy
  86585171968.0f,     // high_band_energy
  0.0393700787f,      // zero_crossing_rate
  0.0f,               // temperature_c
  0.0f,               // humidity_pct
  0.0f,               // vibration_magnitude
  0.0f,               // vibration_mean
  0.0f,               // vibration_variance
  0.0f                // vibration_trend
};

// Feature vector from synthetic_small_leak_000.wav
float smallLeakExample[FEATURE_COUNT] = {
  15464.97753906f,    // rms
  2125.0f,            // peak_frequency_hz
  2180.53993549f,     // spectral_centroid_hz
  375651663872.0f,    // low_band_energy
  1747504726016.0f,   // mid_band_energy
  10563721216.0f,     // high_band_energy
  0.2519685039f,      // zero_crossing_rate
  0.0f,               // temperature_c
  0.0f,               // humidity_pct
  0.0f,               // vibration_magnitude
  0.0f,               // vibration_mean
  0.0f,               // vibration_variance
  0.0f                // vibration_trend
};

// Feature vector from synthetic_medium_leak_000.wav
float mediumLeakExample[FEATURE_COUNT] = {
  10930.78027344f,    // rms
  0.0f,               // peak_frequency_hz
  2483.88687692f,     // spectral_centroid_hz
  169050767360.0f,    // low_band_energy
  783399976960.0f,    // mid_band_energy
  86033932288.0f,     // high_band_energy
  0.3149606299f,      // zero_crossing_rate
  0.0f,               // temperature_c
  0.0f,               // humidity_pct
  0.0f,               // vibration_magnitude
  0.0f,               // vibration_mean
  0.0f,               // vibration_variance
  0.0f                // vibration_trend
};

// Feature vector from synthetic_large_leak_000.wav
float largeLeakExample[FEATURE_COUNT] = {
  11061.78222656f,    // rms
  2500.0f,            // peak_frequency_hz
  2581.02103346f,     // spectral_centroid_hz
  218121404416.0f,    // low_band_energy
  655263334400.0f,    // mid_band_energy
  150613803008.0f,    // high_band_energy
  0.3307086614f,      // zero_crossing_rate
  0.0f,               // temperature_c
  0.0f,               // humidity_pct
  0.0f,               // vibration_magnitude
  0.0f,               // vibration_mean
  0.0f,               // vibration_variance
  0.0f                // vibration_trend
};

const char* selectedExampleName() {
  switch (DEMO_CASE) {
    case 0:
      return "synthetic_no_leak_000.wav";
    case 1:
      return "synthetic_small_leak_000.wav";
    case 2:
      return "synthetic_medium_leak_000.wav";
    case 3:
      return "synthetic_large_leak_000.wav";
    default:
      return "synthetic_large_leak_000.wav";
  }
}

float* selectedFeatures() {
  switch (DEMO_CASE) {
    case 0:
      return noLeakExample;
    case 1:
      return smallLeakExample;
    case 2:
      return mediumLeakExample;
    case 3:
      return largeLeakExample;
    default:
      return largeLeakExample;
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

void printFeatureData(const float* rawFeatures, const float* normalizedFeatures) {
  Serial.println("Extracted feature data from selected sound file:");
  for (int i = 0; i < FEATURE_COUNT; i++) {
    Serial.print("  ");
    Serial.print(FEATURE_NAMES[i]);
    Serial.print(" = ");
    Serial.print(rawFeatures[i], 6);
    Serial.print(" | normalized = ");
    Serial.println(normalizedFeatures[i], 6);
  }
}

void runOneExample() {
  float normalized[FEATURE_COUNT];
  float* features = selectedFeatures();

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

  Serial.println("=================================");
  Serial.print("Example file: ");
  Serial.println(selectedExampleName());
  printFeatureData(features, normalized);
  Serial.print("Leak probability: ");
  Serial.println(probability, 6);

  if (probability >= LEAK_THRESHOLD) {
    Serial.println("MODEL OUTPUT: LEAK");
    digitalWrite(LED_PIN, HIGH);
  } else {
    Serial.println("MODEL OUTPUT: NO LEAK");
    digitalWrite(LED_PIN, LOW);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  setupModel();
  runOneExample();
}

void loop() {
  // One-shot demo: keep the LED in the result state.
  delay(1000);
}
