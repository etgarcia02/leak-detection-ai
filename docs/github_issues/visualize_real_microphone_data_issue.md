# Visualize Real Microphone Data

## Summary

Once real pipe recordings are collected, the next step is to visualize them. This will help confirm that the microphone is capturing useful differences between normal flow and leak conditions before retraining the model.

## What needs to be done

- Generate waveforms for real pipe recordings
- Generate spectrograms for real pipe recordings
- Compare at least one `no_leak` file against one `leak` file
- Save the plots for documentation and report use

## Goal

Confirm that the real microphone data contains visible differences that support the leak detection pipeline.

## Acceptance Criteria

- At least one waveform is generated for `no_leak`
- At least one waveform is generated for `leak`
- At least one spectrogram is generated for each class
- Visuals are saved in the project outputs folder

## Notes

- This issue is mainly for verification and documentation
- If the plots look too similar, that is useful feedback before spending more time on retraining
