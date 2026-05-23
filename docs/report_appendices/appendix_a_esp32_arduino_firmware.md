# APPENDIX A: ESP32 / ARDUINO FIRMWARE

Appendix A contains the ESP32 firmware used for the ENKI prototype. This firmware handles sensor input, feature extraction, TensorFlow Lite Micro inference, and output control for leak classification.

Due to length, the full ESP32 firmware is provided in the project source files listed below. Key code sections are summarized in this appendix.

Main ESP32 firmware files used in the project:

- Handheld firmware: [final_model_demo_handheld.ino](../../firmware/handheld_trident/final_model_demo_handheld.ino)
- Clamp Node 1 firmware: [Clamp01.ino](../../firmware/clam_node1/Clamp01.ino)
- Clamp Node 2 firmware: [Clamp02.ino](../../firmware/clam_node2/Clamp02.ino)
- Half-inch validation firmware: [Clamp01_node1_clean_test.ino](../../firmware/clamp_validation/Clamp01_node1_clean_test.ino)

The snippets below are representative examples taken from the ESP32 firmware that runs on the device.

## A.1 Main Firmware Overview

The ESP32 firmware controls sensor data collection, feature extraction, embedded model inference, and output feedback. The program starts the serial interface, initializes the microphone, vibration sensor, `I2C` devices, and TensorFlow Lite Micro model, then continuously reads live data and updates the leak state.

```cpp
void setup() {
  Serial.begin(115200);
  setupI2CBuses();
  setupDisplay();
  setupEnvironmentSensor();
  setupMic();
  setupModel();

  calibrateVibrationBaseline();
  updateEnvironmentState();
  stabilizeUntilMs = millis() + STARTUP_STABILIZE_MS;
}

void loop() {
  if (handleSerialCommands()) {
    delay(5);
    return;
  }

  runHandheldModel();
  delay(5);
}
```

This part of the code is the main control structure. `setup()` prepares the hardware and the embedded model. `loop()` repeatedly runs the live leak detection cycle.

## A.2 Sensor Reading Code

This section contains the code that reads from the sensors attached to the `ESP32`, including the `INMP441` microphone, the vibration sensor, and the `BME680` temperature and humidity sensor.

### Microphone / INMP441

```cpp
void setupMic() {
  const i2s_config_t i2sConfig = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S
  };

  const i2s_pin_config_t pinConfig = {
    .bck_io_num = I2S_SCK_PIN,
    .ws_io_num = I2S_WS_PIN,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD_PIN
  };

  i2s_driver_install(I2S_PORT, &i2sConfig, 0, nullptr);
  i2s_set_pin(I2S_PORT, &pinConfig);
  i2s_zero_dma_buffer(I2S_PORT);
}

bool readMicWindow() {
  size_t bytesRead = 0;
  esp_err_t result = i2s_read(
    I2S_PORT,
    rawSamples,
    sizeof(rawSamples),
    &bytesRead,
    pdMS_TO_TICKS(I2S_READ_TIMEOUT_MS)
  );
  return result == ESP_OK && bytesRead == sizeof(rawSamples);
}
```

This code configures the microphone through the `I2S` peripheral and reads one audio window at a time into the sample buffer.

### Vibration Sensor

```cpp
void updateVibrationState() {
  int localPeakDelta = 0;

  for (int sample = 0; sample < GY61_SAMPLE_BURST; sample++) {
    int raw[3] = {0, 0, 0};
    for (int axis = 0; axis < 3; axis++) {
      raw[axis] = analogRead(gy61Pins[axis]);
    }

    float deltaMagnitudeSquared = 0.0f;
    for (int axis = 0; axis < 3; axis++) {
      int axisDelta = abs(raw[axis] - gy61Baseline[axis]);
      deltaMagnitudeSquared += (float)axisDelta * (float)axisDelta;
    }

    float rawDeltaMagnitude = sqrtf(deltaMagnitudeSquared);
    int scaledDelta = (int)lroundf(rawDeltaMagnitude * GY61_DELTA_SCALE);
    if (scaledDelta > localPeakDelta) {
      localPeakDelta = scaledDelta;
    }
  }
}
```

This part of the firmware reads the three vibration axes, compares them to baseline values, and converts the motion into a single vibration magnitude.

### Temperature / Humidity Sensor

```cpp
void updateEnvironmentState() {
  if (!bmeReady) {
    return;
  }

  if (!bme680.performReading()) {
    return;
  }

  envTemperatureC = bme680.temperature;
  envHumidityPct = bme680.humidity;
}
```

This code updates the environmental sensor values used for display and logging.

## A.3 Feature Extraction Code

This section contains the embedded feature calculations used to convert raw sensor signals into model-ready inputs. These features were important in the project because they describe the sound and vibration behavior of the pipe during leak testing.

### RMS

```cpp
float computeRms(const int16_t* audio) {
  double sumSquares = 0.0;
  for (int i = 0; i < WINDOW_SAMPLES; i++) {
    double sample = (double)audio[i];
    sumSquares += sample * sample;
  }
  return sqrt(sumSquares / WINDOW_SAMPLES);
}
```

### Zero Crossing Rate

```cpp
float computeZeroCrossingRate(const int16_t* audio) {
  int crossings = 0;
  for (int i = 1; i < WINDOW_SAMPLES; i++) {
    bool currentPositive = audio[i] >= 0;
    bool previousPositive = audio[i - 1] >= 0;
    if (currentPositive != previousPositive) {
      crossings++;
    }
  }
  return (float)crossings / (float)(WINDOW_SAMPLES - 1);
}
```

### Peak Frequency and Spectral Centroid

```cpp
void computeSpectralFeatures(
  const int16_t* audio,
  float& peakFrequency,
  float& spectralCentroid,
  float& lowBandEnergy,
  float& midBandEnergy,
  float& highBandEnergy
) {
  const int bins = WINDOW_SAMPLES / 2 + 1;
  float maxMagnitude = -1.0f;
  int peakIndex = 0;
  float magnitudeSum = 0.0f;
  float weightedSum = 0.0f;

  for (int k = 0; k < bins; k++) {
    double realPart = 0.0;
    double imagPart = 0.0;

    for (int n = 0; n < WINDOW_SAMPLES; n++) {
      double angle = -2.0 * PI * (double)k * (double)n / (double)WINDOW_SAMPLES;
      realPart += (double)audio[n] * cos(angle);
      imagPart += (double)audio[n] * sin(angle);
    }

    float magnitude = (float)sqrt(realPart * realPart + imagPart * imagPart);
    float frequency = ((float)k * SAMPLE_RATE) / WINDOW_SAMPLES;

    if (magnitude > maxMagnitude) {
      maxMagnitude = magnitude;
      peakIndex = k;
    }

    magnitudeSum += magnitude;
    weightedSum += frequency * magnitude;
  }

  peakFrequency = ((float)peakIndex * SAMPLE_RATE) / WINDOW_SAMPLES;
  spectralCentroid = magnitudeSum > 0.0f ? weightedSum / magnitudeSum : 0.0f;
}
```

### Vibration Calculations

```cpp
void updateEngineeredVibrationFeatures(
  float peakDelta,
  float activeMean,
  float variance,
  float burstMean
) {
  vibrationLastDelta = (int)peakDelta;
  vibrationMeanFeature = activeMean;
  vibrationVarianceFeature = variance;

  float trend = vibrationMeanFeature - previousVibrationMeanFeature;
  vibrationTrendFeature = trend > 0.0f ? trend : 0.0f;
  previousVibrationMeanFeature = vibrationMeanFeature;

  vibrationSustainFeature =
    (VIBRATION_SUSTAIN_ALPHA * burstMean) +
    ((1.0f - VIBRATION_SUSTAIN_ALPHA) * vibrationSustainFeature);
}
```

These feature functions generate the main values used later in leak classification, including `SoundLevel`, `PeakFrequency`, `SpectralCenter`, `ZeroCross`, and the vibration features.

## A.4 TensorFlow Lite Micro Inference Code

This section shows the code that loads the embedded model, normalizes the features, sends them into the model, and returns the leak probability.

```cpp
void setupModel() {
  const tflite::Model* model = tflite::GetModel(g_leak_model_data);

  static tflite::MicroInterpreter staticInterpreter(
    model,
    resolver,
    tensorArena,
    TENSOR_ARENA_SIZE,
    &microErrorReporter
  );

  interpreter = &staticInterpreter;
  interpreter->AllocateTensors();
}

bool runTensorFlowModel(float features[FEATURE_COUNT], float& probability) {
  float normalized[FEATURE_COUNT] = {0.0f};
  normalize_features(features, normalized);

  TfLiteTensor* input = interpreter->input(0);
  for (int i = 0; i < FEATURE_COUNT; i++) {
    input->data.f[i] = normalized[i];
  }

  if (interpreter->Invoke() != kTfLiteOk) {
    probability = 0.0f;
    return false;
  }

  probability = interpreter->output(0)->data.f[0];
  return true;
}
```

This is the embedded machine-learning part of the ESP32 firmware. The model is loaded from `leak_model_data.h`, the features are normalized, the interpreter is invoked, and the output is read as the leak confidence value.

## A.5 Output Control Code

This section includes the code responsible for output feedback, such as the `OLED` display and the clamp `RGB LED`.

### Display Output

```cpp
void updateDisplay() {
  if (!oledReady) {
    return;
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Leak Detector");

  display.setTextSize(2);
  display.setCursor(0, 12);
  display.println(leakLevelLabel(leakLevel));

  display.setTextSize(1);
  display.setCursor(0, 34);
  display.print("A:");
  display.println((int)soundLevel);

  display.display();
}
```

### Leak Detection Logic and Output State

```cpp
void runHandheldModel() {
  float features[FEATURE_COUNT] = {0.0f};

  features[0] = computeRms(audioI16);
  computeSpectralFeatures(audioI16, features[1], features[2], features[3], features[4], features[5]);
  features[6] = computeZeroCrossingRate(audioI16);

  updateVibrationState();

  features[9] = vibrationLastDelta;
  features[10] = vibrationMeanFeature;
  features[11] = vibrationVarianceFeature;
  features[12] = vibrationTrendFeature;

  if (!runTensorFlowModel(features, modelScore)) {
    rawLeakLevel = LEAK_LEVEL_NONE;
  } else {
    rawLeakLevel = classifyLeak(
      modelScore,
      soundLevel,
      spectralCenterHz,
      zeroCrossRate,
      vibrationLastDelta,
      vibrationMeanFeature,
      vibrationSustainFeature
    );
  }

  leakLevel = applyLeakConfirmation(rawLeakLevel);
  updateDisplay();
  printSerial();
}
```

This output section ties the whole firmware together. After the ESP32 reads sensors, extracts features, and runs the model, it converts that result into a user-visible output state.

### RGB LED Output

```cpp
void setLeakOutputs(LeakLevel leakLevel, bool signalFault = false) {
  bool redOn = false;
  bool greenOn = false;
  bool blueOn = false;

  if (signalFault) {
    redOn = true;
    blueOn = true;
  } else {
    switch (leakLevel) {
      case LEAK_LEVEL_NONE:
        greenOn = true;
        break;
      case LEAK_LEVEL_SMALL:
      case LEAK_LEVEL_MEDIUM:
        redOn = true;
        greenOn = true;
        break;
      case LEAK_LEVEL_LARGE:
        blueOn = true;
        break;
    }
  }

  writeRgbPin(RGB_RED_PIN, redOn);
  writeRgbPin(RGB_GREEN_PIN, greenOn);
  writeRgbPin(RGB_BLUE_PIN, blueOn);
}
```

In the clamp firmware, the `RGB LED` gives a direct visual leak state. `Green` means no leak, `red/yellow` indicates a small or medium leak, `blue` indicates a large leak, and a fault condition can be shown with a different color combination.

