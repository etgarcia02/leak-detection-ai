# Firmware Overview

This folder contains the main ESP32 firmware used across the ENKI leak-detection project.

## Folders

- `handheld_trident/`
  - portable handheld inspection device
  - reads microphone, vibration, and environment sensors
  - runs TensorFlow Lite Micro locally and updates the OLED display

- `clam_node1/`
  - clamp-mounted monitoring node for one pipe position
  - includes local inference, RGB LED output, and optional cloud hooks

- `clam_node2/`
  - second clamp-mounted monitoring node for comparing readings at a different pipe location

- `clamp_validation/`
  - dedicated tuning and validation sketch used while testing `1/2-inch` pipe behavior

## Notes

- `leak_model_data.*` and `scaler_params.h` are the firmware-side model artifacts copied from the ML export pipeline.
- `Clamp01.ino` and `Clamp02.ino` are checked in with `ENABLE_AWS_IOT` disabled by default for public sharing.
- If cloud publishing is re-enabled, create a private `aws_iot_config.h` based on the example header in each clamp folder and keep it out of version control.
