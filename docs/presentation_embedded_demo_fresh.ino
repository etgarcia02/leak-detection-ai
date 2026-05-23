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

// Change this if your LED is inverted.
// Most boards use HIGH. If the LED behavior is backwards, change to LOW.
#define LED_ON_LEVEL HIGH

// Choose the example to demo:
// 0 = no_leak
// 1 = small_leak
// 2 = medium_leak
// 3 = large_leak
#define DEMO_CASE 0

static tflite::MicroErrorReporter micro_error_reporter;
static tflite::AllOpsResolver resolver;
static uint8_t tensor_arena[TENSOR_ARENA_SIZE];
static tflite::MicroInterpreter* interpreter = nullptr;

struct DemoExample {
  const char* file_name;
  const char* expected_label;
  float features[FEATURE_COUNT];
};

DemoExample DEMO_EXAMPLES[] = {
  {
    "synthetic_no_leak_004.wav",
    "NO LEAK",
    {
      19126.080078125f,
      0.0f,
      2033.6163153805533f,
      5202217795584.0f,
      127890841600.0f,
      97976221696.0f,
      0.0551181102362204f,
      0.0f,
      0.0f,
      0.0f,
      0.0f,
      0.0f,
      0.0f
    }
  },
  {
    "synthetic_small_leak_003.wav",
    "LEAK",
    {
      13537.3642578125f,
      0.0f,
      2221.683068516292f,
      522580393984.0f,
      1189477875712.0f,
      11636592640.0f,
      0.2283464566929134f,
      0.0f,
      0.0f,
      0.0f,
      0.0f,
      0.0f,
      0.0f
    }
  },
  {
    "synthetic_medium_leak_002.wav",
    "LEAK",
    {
      12868.4794921875f,
      2500.0f,
      2116.74872877399f,
      208717791232.0f,
      1222450610176.0f,
      5179531264.0f,
      0.2440944881889764f,
      0.0f,
      0.0f,
      0.0f,
      0.0f,
      0.0f,
      0.0f
    }
  },
  {
    "synthetic_large_leak_018.wav",
    "LEAK",
    {
      11995.294921875f,
      4750.0f,
      3055.933445862253f,
      105140051968.0f,
      547548692480.0f,
      547528015872.0f,
      0.4645669291338583f,
      0.0f,
      0.0f,
      0.0f,
      0.0f,
      0.0f,
      0.0f
    }
  }
};

void setResultLed(bool leak_detected) {
  digitalWrite(LED_PIN, leak_detected ? LED_ON_LEVEL : (LED_ON_LEVEL == HIGH ? LOW : HIGH));
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
    Serial.println("ERROR: AllocateTensors failed");
    while (true) {
      delay(1000);
    }
  }
}

void runDemoOnce() {
  DemoExample* example = &DEMO_EXAMPLES[DEMO_CASE];
  float normalized[FEATURE_COUNT];

  normalize_features(example->features, normalized);

  TfLiteTensor* input = interpreter->input(0);
  for (int i = 0; i < FEATURE_COUNT; i++) {
    input->data.f[i] = normalized[i];
  }

  if (interpreter->Invoke() != kTfLiteOk) {
    Serial.println("ERROR: Model invoke failed");
    setResultLed(false);
    return;
  }

  float probability = interpreter->output(0)->data.f[0];
  bool leak_detected = probability >= LEAK_THRESHOLD;

  setResultLed(leak_detected);

  Serial.println("========================================");
  Serial.println("EMBEDDED ML DEMO");
  Serial.print("Demo Case      : ");
  Serial.println(DEMO_CASE);
  Serial.print("Example File   : ");
  Serial.println(example->file_name);
  Serial.print("Expected Class : ");
  Serial.println(example->expected_label);
  Serial.print("Leak Prob.     : ");
  Serial.println(probability, 6);
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

  setupModel();
}

void loop() {
  runDemoOnce();
  delay(5000);
}
