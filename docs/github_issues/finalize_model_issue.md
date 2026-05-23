# Finalize Leak Detection Model

## Summary

The current leak detection pipeline is working, but the model still needs to be finalized using real microphone data and a cleaner live test flow. Right now, the embedded model has been trained mainly on synthetic audio and tested on the ESP32, but the next step is to validate and improve it with real pipe recordings.

## Current Status

- Synthetic audio pipeline is working
- Feature extraction is working
- Baseline binary model is trained
- TensorFlow Lite export is working
- ESP32 model deployment is working
- Live sensing demos have been tested separately

## What needs to be done

- Collect real microphone recordings from the pipe rig
- Build a labeled real dataset for `no_leak` and `leak`
- Extract features from the real recordings
- Retrain the binary model on the updated dataset
- Evaluate model performance on real data
- Re-export the model to TensorFlow Lite
- Regenerate:
  - `leak_model_data.h`
  - `leak_model_data.cpp`
  - `scaler_params.h`
- Test the updated model on the ESP32 with live input

## Goal

Finalize a binary `leak` / `no_leak` model that works more reliably on real pipe audio and is ready for the final integrated ESP32 demo.

## Acceptance Criteria

- Real pipe audio has been added to the dataset
- Model has been retrained on real recordings
- Updated model artifacts have been exported
- ESP32 can run the updated model
- Live test results are documented

## Notes

- The current model is binary, even though earlier synthetic data used `no_leak`, `small_leak`, `medium_leak`, and `large_leak`
- The Osoyoo sound sensor was useful for live sensing demos, but it is not the best long-term input for the current WAV-based feature pipeline
- The INMP441 microphone path should be the main focus for final model testing
