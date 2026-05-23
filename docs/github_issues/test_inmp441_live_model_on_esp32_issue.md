# Test INMP441 Live Model on ESP32

## Summary

The final model path needs to be tested with the INMP441 microphone connected directly to the ESP32. The goal is to confirm that live audio can be captured, converted into features, passed into the TensorFlow Lite model, and used to produce a leak or no-leak result.

## What needs to be done

- Verify INMP441 wiring and I2S configuration
- Confirm the ESP32 is reading live audio data correctly
- Print live feature values to the Serial Monitor
- Run live inference with the exported TensorFlow Lite model
- Compare behavior between normal flow and leak conditions
- Document Serial Monitor output and LED behavior

## Goal

Validate the live microphone-to-model path on the ESP32.

## Acceptance Criteria

- The ESP32 reads live microphone data
- Feature values change between different pipe conditions
- The model runs without crashing
- Leak probability and final result are visible in Serial Monitor
- Live test notes are documented

## Notes

- The first successful test does not need perfect accuracy
- The main goal is proving that live mic input, feature extraction, and model inference all work together
