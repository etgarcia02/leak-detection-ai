# Retrain Model With Real Audio

## Summary

After collecting real pipe recordings, the model needs to be retrained with that data. The current model was built mainly from synthetic audio, so this step is needed to move toward a more realistic final model.

## What needs to be done

- Add real recordings to the dataset structure
- Rebuild the manifest
- Extract features from the updated dataset
- Retrain the binary `no_leak` / `leak` model
- Evaluate the updated model
- Save updated metrics and model artifacts

## Goal

Train a binary leak detection model that uses real microphone data instead of depending mostly on synthetic examples.

## Acceptance Criteria

- Real recordings are included in the training pipeline
- A new trained model is saved
- Updated evaluation results are saved
- The new model is ready for export

## Notes

- Keep the model binary for now
- The main focus is getting stable real-data performance before adding more classes
