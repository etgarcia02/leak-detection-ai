# ENKI — On-Device Water Leak Detection

**Ethan Garcia** · Electrical Engineering, University of the Incarnate Word  
Accepted for presentation at UIW Excellence Summit 2026, IEEE Sensors 2027, and ASEE 2027

ENKI is an embedded leak-detection system that combines ESP32 firmware, signal 
processing, and TensorFlow Lite Micro for real-time on-device leak classification. 
No cloud required — inference runs entirely on the microcontroller.

This repository is organized for technical review and includes:

- Production-oriented handheld and clamp firmware
- End-to-end ML training and export pipeline
- Deployable TensorFlow Lite artifacts
- Feature engineering and validation materials
- Presentation and report assets documenting the system design

## What This Project Does

The system monitors water pipe conditions using an INMP441 digital microphone, 
ADXL335 analog 3-axis vibration sensor, BME680 environmental sensor, and an ESP32 
running local signal processing and model inference.

The firmware extracts acoustic and vibration features, normalizes them using the 
same scaler used during training, runs a TensorFlow Lite Micro model on the ESP32, 
and updates local outputs including an OLED display and RGB LED state indicators.

## System Architecture

![Overall system diagram](assets/diagrams/pipeline_diagram.svg)

Two device form factors were developed:

- **TRIDENT handheld** for close-range inspection and leak localization
- **CLAM clamp nodes** for mounted monitoring across different pipe sections

The deployed model is a binary classifier outputting `no_leak` or `leak`. 
Final severity shown on-device combines the model score with additional firmware 
rules and sensor thresholds.

## Hardware Validation

The system was validated on a Hampden H-6952 Fluid Circuit Demonstrator using 
both handheld and clamp-mounted prototypes simultaneously. Testing covered:

- 1-inch pipe sections for standard detection range
- 1/2-inch pipe sections to evaluate acoustic and vibration differences by diameter
- No-leak, small-leak, medium-leak, and large-leak conditions simulated by 
  adjusting a valve connected to a small bleed tube
- AWS IoT used for cloud-based sensor logging and remote monitoring during 
  fixed-node testing

Node 1 showed a clear model confidence increase during leak events on the 1-inch 
pipe. Node 2 was retrained after initial calibration issues and correctly aligned 
confidence with actual leak periods. The handheld detected all three conditions 
in real time and displayed results on the OLED screen.

## Demo Videos

Live demos and validation clips are available in the 
[ENKI demo video folder](https://drive.google.com/drive/folders/1o9d9tqtpuJP-XiGKITMrDfAXcVD0qzO7).

Recommended clips:

- Handheld leak-detection demo
- Clamp testing on the 1-inch pipe
- Clamp testing on the 1/2-inch pipe
- Large-leak validation runs
- Side-by-side node comparison clips

## Machine Learning Pipeline

![ML pipeline diagram](assets/diagrams/ml_demo_diagram.svg)

The full pipeline is included in this repository:

1. Generate or collect audio data
2. Build a manifest
3. Extract engineered features
4. Train a compact binary classifier
5. Evaluate model performance
6. Export to TensorFlow Lite
7. Convert TFLite model to Arduino-ready C arrays
8. Deploy model and scaler parameters to ESP32 firmware

The model uses 13 ordered features: `rms`, `peak_frequency_hz`, 
`spectral_centroid_hz`, `low_band_energy`, `mid_band_energy`, `high_band_energy`, 
`zero_crossing_rate`, `temperature_c`, `humidity_pct`, `vibration_magnitude`, 
`vibration_mean`, `vibration_variance`, `vibration_trend`

## Validation Snapshot

- Training classes: `no_leak`, `small_leak`, `medium_leak`, `large_leak`
- Dataset: 160 clips — 112 train / 24 validation / 24 test
- Synthetic split metrics: accuracy, precision, recall, F1, and ROC AUC all 1.0
- Real-pipe validation results are documented in `docs/report_appendices/`

## Included Model Artifacts

- Trained Keras model: `ml/models/leak_binary_classifier.keras`
- Scaler parameters: `ml/models/scaler_params.json`
- Metrics summary: `ml/models/metrics.json`
- Float TFLite model: `ml/exports/leak_model_float.tflite`
- Int8 TFLite model: `ml/exports/leak_model_int8.tflite`
- Firmware-ready model arrays: `ml/exports/firmware_model/`

## Repository Structure

```text
.
|-- assets/
|   |-- diagrams/
|   `-- images/
|-- firmware/
|   |-- handheld_trident/
|   |-- clam_node1/
|   |-- clam_node2/
|   `-- clamp_validation/
|-- ml/
|   |-- configs/
|   |-- data/
|   |-- exports/
|   |-- models/
|   |-- scripts/
|   `-- src/
|-- docs/
|   |-- report_appendices/
`-- run_*.ps1
```

## Key Firmware Entry Points

- Handheld sketch: `firmware/handheld_trident/final_model_demo_handheld.ino`
- Clamp node 1: `firmware/clam_node1/Clamp01.ino`
- Clamp node 2: `firmware/clam_node2/Clamp02.ino`
- Half-inch validation: `firmware/clamp_validation/Clamp01_node1_clean_test.ino`

See `firmware/README.md` for more context on each folder.

## Quick Start

Requires Python 3.11 on Windows.

```powershell
python -m venv .venv
.venv\Scripts\Activate.ps1
pip install -r ml\requirements.txt
python ml\scripts\generate_synthetic_audio.py
python ml\scripts\build_manifest.py
python ml\scripts\extract_features.py
python ml\scripts\train_model.py
python ml\scripts\evaluate_model.py
python ml\scripts\export_tflite.py --quantize none
python ml\scripts\export_tflite.py --quantize int8
python ml\scripts\tflite_to_c_array.py
```

```powershell
.\run_audio_baseline.ps1
.\run_audio_visualizations.ps1
.\run_demo_audio.ps1
```

## Selected Documentation

- Validation data guide: `docs/validation_data/README.md`
- Firmware appendix: `docs/report_appendices/appendix_a_esp32_arduino_firmware.md`
- ML appendix: `docs/report_appendices/appendix_b_machine_learning_tensorflow_model.md`
- Feature equations: `docs/report_appendices/appendix_c_feature_extraction_equations.md`
- Final report: `docs/report_appendices/final_report_for_upload.md`

## Notes

Private Wi-Fi and AWS IoT credentials were removed from this portfolio version. 
Cloud publishing is disabled by default in the checked-in clamp sketches. This 
repository excludes local virtual environments and intermediate files that do not 
contribute to technical review.
