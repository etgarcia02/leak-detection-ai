# Collect Real Pipe Audio Dataset

## Summary

The next step for the model is collecting real pipe audio from the classroom rig. The current pipeline works, but the training data is still mostly synthetic. We need a small but organized real dataset for `no_leak` and `leak` so the model can be retrained and tested on real microphone input.

## What needs to be done

- Set up the INMP441 microphone with the ESP32
- Record real audio from the pipe rig
- Capture at least two conditions:
  - `no_leak`
  - `leak`
- Save recordings in a consistent folder structure
- Label each recording clearly
- Keep notes on recording conditions such as flow state and leak state

## Goal

Build the first real microphone dataset that can be used for feature extraction, retraining, and live testing.

## Acceptance Criteria

- Real pipe recordings exist for both `no_leak` and `leak`
- Files are organized and labeled clearly
- Recording notes are saved
- Data is ready to be added to the ML pipeline

## Notes

- Start with binary classes first
- Do not worry about small, medium, and large leak classes until the binary pipeline is stable
- Consistent mic placement matters more than collecting a huge amount of data on the first pass
