#include <Arduino.h>
#include <math.h>
#include <driver/i2s.h>

#include <TensorFlowLite_ESP32.h>
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "leak_model_data.h"
#include "scaler_params.h"

// If you switch to your teammate's WS/LRCL pin on GPIO 25,
// move this LED pin to something else like GPIO 2 or GPIO 27.
#define LED_PIN 25
#define LEAK_THRESHOLD 0.5f
#define TENSOR_ARENA_SIZE (40 * 1024)

// INMP441 wiring defaults used in this sketch:
// SCK/BCLK -> GPIO 14
// WS/LRCL  -> GPIO 15
// SD/DOUT  -> GPIO 32
//
// Teammate's working wiring:
// SCK/BCLK -> GPIO 26
// WS/LRCL  -> GPIO 25
// SD/DOUT  -> GPIO 33
// If you use teammate wiring, do not keep LED_PIN on GPIO 25.
#define I2S_WS_PIN 15
#define I2S_SCK_PIN 14
#define I2S_SD_PIN 32

#define I2S_PORT I2S_NUM_0
#define SAMPLE_RATE 16000
#define WINDOW_SAMPLES 128

static tflite::MicroErrorReporter micro_error_reporter;
static tflite::AllOpsResolver resolver;
static uint8_t tensor_arena[TENSOR_ARENA_SIZE];
static tflite::MicroInterpreter* interpreter = nullptr;

static int32_t raw_samples[WINDOW_SAMPLES];
static int16_t audio_i16[WINDOW_SAMPLES];

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

void setupMic() {
  const i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 64,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };

  const i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK_PIN,
    .ws_io_num = I2S_WS_PIN,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD_PIN
  };

  i2s_driver_install(I2S_PORT, &i2s_config, 0, nullptr);
  i2s_set_pin(I2S_PORT, &pin_config);
  i2s_zero_dma_buffer(I2S_PORT);
}

bool readMicWindow() {
  size_t bytes_read = 0;
  esp_err_t result = i2s_read(
    I2S_PORT,
    raw_samples,
    sizeof(raw_samples),
    &bytes_read,
    portMAX_DELAY
  );

  if (result != ESP_OK || bytes_read != sizeof(raw_samples)) {
    return false;
  }

  long long sum = 0;
  for (int i = 0; i < WINDOW_SAMPLES; i++) {
    sum += raw_samples[i];
  }

  int32_t mean = (int32_t)(sum / WINDOW_SAMPLES);

  for (int i = 0; i < WINDOW_SAMPLES; i++) {
    int32_t centered = raw_samples[i] - mean;
    int32_t shifted = centered >> 14;

    if (shifted > 32767) shifted = 32767;
    if (shifted < -32768) shifted = -32768;

    audio_i16[i] = (int16_t)shifted;
  }

  return true;
}

float computeRms(const int16_t* audio) {
  double sum_sq = 0.0;
  for (int i = 0; i < WINDOW_SAMPLES; i++) {
    double value = (double)audio[i];
    sum_sq += value * value;
  }
  return (float)sqrt(sum_sq / WINDOW_SAMPLES + 1e-12);
}

float computeZeroCrossingRate(const int16_t* audio) {
  int crossings = 0;
  for (int i = 1; i < WINDOW_SAMPLES; i++) {
    bool prev = audio[i - 1] >= 0;
    bool curr = audio[i] >= 0;
    if (prev != curr) {
      crossings++;
    }
  }
  return (float)crossings / (float)(WINDOW_SAMPLES - 1);
}

void computeSpectralFeatures(
  const int16_t* audio,
  float& peak_frequency_hz,
  float& spectral_centroid_hz,
  float& low_band_energy,
  float& mid_band_energy,
  float& high_band_energy
) {
  const int bins = WINDOW_SAMPLES / 2 + 1;
  float magnitudes[bins];
  float freqs[bins];

  float max_magnitude = -1.0f;
  int peak_index = 0;
  float magnitude_sum = 0.0f;
  float weighted_sum = 0.0f;

  low_band_energy = 0.0f;
  mid_band_energy = 0.0f;
  high_band_energy = 0.0f;

  for (int k = 0; k < bins; k++) {
    double real_part = 0.0;
    double imag_part = 0.0;

    for (int n = 0; n < WINDOW_SAMPLES; n++) {
      double angle = -2.0 * PI * (double)k * (double)n / (double)WINDOW_SAMPLES;
      real_part += (double)audio[n] * cos(angle);
      imag_part += (double)audio[n] * sin(angle);
    }

    float magnitude = (float)sqrt(real_part * real_part + imag_part * imag_part);
    float freq = ((float)k * SAMPLE_RATE) / WINDOW_SAMPLES;
    float power = magnitude * magnitude;

    magnitudes[k] = magnitude;
    freqs[k] = freq;

    if (magnitude > max_magnitude) {
      max_magnitude = magnitude;
      peak_index = k;
    }

    magnitude_sum += magnitude;
    weighted_sum += freq * magnitude;

    if (freq < 1000.0f) {
      low_band_energy += power;
    } else if (freq < 4000.0f) {
      mid_band_energy += power;
    } else if (freq <= 8000.0f) {
      high_band_energy += power;
    }
  }

  peak_frequency_hz = freqs[peak_index];
  spectral_centroid_hz = (magnitude_sum > 0.0f) ? (weighted_sum / magnitude_sum) : 0.0f;
}

void runLiveModel() {
  if (!readMicWindow()) {
    Serial.println("Mic read failed");
    digitalWrite(LED_PIN, LOW);
    delay(500);
    return;
  }

  float features[FEATURE_COUNT] = {0.0f};
  float normalized[FEATURE_COUNT] = {0.0f};

  features[0] = computeRms(audio_i16);
  computeSpectralFeatures(
    audio_i16,
    features[1],
    features[2],
    features[3],
    features[4],
    features[5]
  );
  features[6] = computeZeroCrossingRate(audio_i16);

  normalize_features(features, normalized);

  TfLiteTensor* input = interpreter->input(0);
  for (int i = 0; i < FEATURE_COUNT; i++) {
    input->data.f[i] = normalized[i];
  }

  if (interpreter->Invoke() != kTfLiteOk) {
    Serial.println("Model invoke failed");
    digitalWrite(LED_PIN, LOW);
    delay(500);
    return;
  }

  float probability = interpreter->output(0)->data.f[0];
  bool leak_detected = probability >= LEAK_THRESHOLD;

  digitalWrite(LED_PIN, leak_detected ? HIGH : LOW);

  Serial.print("Sound Level      : ");
  Serial.println(features[0], 2);
  Serial.print("Peak Frequency   : ");
  Serial.print(features[1], 2);
  Serial.println(" Hz");
  Serial.print("Spectral Center  : ");
  Serial.print(features[2], 2);
  Serial.println(" Hz");
  Serial.print("Zero Cross Rate  : ");
  Serial.println(features[6], 4);
  Serial.print("Leak Probability : ");
  Serial.println(probability, 6);
  Serial.print("Result           : ");
  Serial.println(leak_detected ? "LEAK" : "NO LEAK");
  Serial.print("LED              : ");
  Serial.println(leak_detected ? "ON" : "OFF");
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  setupMic();
  setupModel();

  Serial.println("INMP441 live model test ready");
  Serial.println("Reading live mic audio and running the ESP32 model");
  Serial.println();
}

void loop() {
  runLiveModel();
  delay(800);
}
