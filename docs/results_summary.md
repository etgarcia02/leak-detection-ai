# Results Summary

This file gives a short technical summary of the current ENKI project snapshot.

## Model Snapshot

- classifier type: binary `no_leak` vs `leak`
- feature count: `13`
- float TFLite size: `6264` bytes
- int8 TFLite size: `4864` bytes
- saved split sizes: `112` train / `24` validation / `24` test
- current saved synthetic metrics: accuracy, precision, recall, F1, and ROC AUC all `1.0`

## Embedded Deployment

- TensorFlow Lite Micro is executed directly on the `ESP32`
- firmware-side normalization uses the same `FEATURE_MEANS` and `FEATURE_STDS` generated during training
- leak probability is combined with threshold logic to produce user-facing leak levels

## Validation Notes

- `1-inch` pipe tests were used to compare node consistency across two clamp positions
- `1/2-inch` pipe tests helped tune threshold behavior for smaller pipe geometry
- handheld operation was more useful for close inspection and smaller leak localization
- clamp monitoring was more useful for mounted monitoring and larger leak patterns

## Where To Look

- live firmware: [`../firmware/`](../firmware)
- ML training and export pipeline: [`../ml/`](../ml)
- selected report materials: [`report_appendices/`](report_appendices)
