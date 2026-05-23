# APPENDIX B: MACHINE LEARNING / TENSORFLOW MODEL

Appendix B documents the machine-learning pipeline used to create the TensorFlow model deployed on the `ESP32`. This appendix covers the repository files used for model development, dataset generation, feature inputs, training workflow, normalization files, TensorFlow Lite conversion, and model output classes.

Project repository snapshot: [repo root](../../README.md)

## B.1 Machine Learning Repository Files

The main machine-learning files used in the project are listed below.

Core scripts:

- Training script: [train_model.py](../../ml/scripts/train_model.py)
- Feature extraction script: [extract_features.py](../../ml/scripts/extract_features.py)
- TensorFlow Lite export script: [export_tflite.py](../../ml/scripts/export_tflite.py)
- TFLite-to-firmware conversion script: [tflite_to_c_array.py](../../ml/scripts/tflite_to_c_array.py)
- Evaluation script: [evaluate_model.py](../../ml/scripts/evaluate_model.py)
- Manifest builder: [build_manifest.py](../../ml/scripts/build_manifest.py)
- Synthetic dataset generator: [generate_synthetic_audio.py](../../ml/scripts/generate_synthetic_audio.py)

Core support modules:

- Feature calculations: [features.py](../../ml/src/features.py)
- Model architecture: [modeling.py](../../ml/src/modeling.py)
- Scaling utilities: [scaler_utils.py](../../ml/src/scaler_utils.py)
- Data loading and WAV utilities: [data_utils.py](../../ml/src/data_utils.py)
- Synthetic audio routines: [synthetic_audio.py](../../ml/src/synthetic_audio.py)

Configuration and artifacts:

- Experiment configuration: [experiment.json](../../ml/configs/experiment.json)
- Dataset manifest: [manifest.csv](../../ml/data/manifest.csv)
- Trained Keras model: [leak_binary_classifier.keras](../../ml/models/leak_binary_classifier.keras)
- Feature column order: [feature_columns.json](../../ml/models/feature_columns.json)
- Label file: [label_map.json](../../ml/models/label_map.json)
- Scaler parameters: [scaler_params.json](../../ml/models/scaler_params.json)
- Float32 TensorFlow Lite model: [leak_model_float.tflite](../../ml/exports/leak_model_float.tflite)
- Int8 TensorFlow Lite model: [leak_model_int8.tflite](../../ml/exports/leak_model_int8.tflite)
- Firmware-exported model header: [leak_model_data.h](../../ml/exports/firmware_model/leak_model_data.h)
- Firmware-exported model source: [leak_model_data.cpp](../../ml/exports/firmware_model/leak_model_data.cpp)
- Firmware scaler header: [scaler_params.h](../../ml/exports/firmware_model/scaler_params.h)

These files make up the full machine-learning workflow from training data to embedded deployment.

## B.2 Dataset Generation

The machine-learning dataset in this repository was generated as a structured synthetic audio dataset for pipeline development and smoke testing. The synthetic audio generator creates separate waveform clips for:

- `no_leak`
- `small_leak`
- `medium_leak`
- `large_leak`

The dataset generation flow is:

1. [generate_synthetic_audio.py](../../ml/scripts/generate_synthetic_audio.py) creates `.wav` files under `ml/data/raw/`.
2. [build_manifest.py](../../ml/scripts/build_manifest.py) scans those folders and builds [manifest.csv](../../ml/data/manifest.csv).
3. [extract_features.py](../../ml/scripts/extract_features.py) converts each audio clip into a row of extracted features for training.

The current manifest shows `40` synthetic clips per class for the four main raw labels, for a total of `160` audio clips. The labels are stored with both:

- `raw_label`, which preserves the original class such as `small_leak`
- `label`, which maps the clip into the binary training target
- `leak_size_label`, which keeps the leak size category for reference

The manifest also contains optional metadata fields such as:

- temperature
- humidity
- pressure
- gas resistance
- distance
- vibration measurements
- notes

In the current synthetic manifest, most of these metadata fields are blank placeholders. They are part of the schema so the same pipeline can later be used with richer real-world datasets.

## B.3 Feature Inputs Used by the Model

The feature inputs used by the model are defined in [experiment.json](../../ml/configs/experiment.json) and extracted by [features.py](../../ml/src/features.py).

The model uses `13` ordered input features:

1. `rms`
2. `peak_frequency_hz`
3. `spectral_centroid_hz`
4. `low_band_energy`
5. `mid_band_energy`
6. `high_band_energy`
7. `zero_crossing_rate`
8. `temperature_c`
9. `humidity_pct`
10. `vibration_magnitude`
11. `vibration_mean`
12. `vibration_variance`
13. `vibration_trend`

These are stored in [feature_columns.json](../../ml/models/feature_columns.json) and used as the exact input order for training and deployment.

The feature categories can be grouped as:

- acoustic energy feature: `rms`
- frequency-domain features: `peak_frequency_hz`, `spectral_centroid_hz`, band energies
- time-domain audio feature: `zero_crossing_rate`
- environmental features: `temperature_c`, `humidity_pct`
- vibration features: `vibration_magnitude`, `vibration_mean`, `vibration_variance`, `vibration_trend`

This feature design matches the embedded project goal of combining microphone and vibration sensing instead of relying on audio alone.

## B.4 Model Training Script

The main training file is [train_model.py](../../ml/scripts/train_model.py). This script loads the extracted feature dataset, splits the data into train, validation, and test sets, normalizes the features, builds the neural network, trains the model, and saves the results.

The training configuration in [experiment.json](../../ml/configs/experiment.json) uses:

- validation split: `0.15`
- test split: `0.15`
- batch size: `16`
- epochs: `40`
- learning rate: `0.001`
- random seed: `42`

The model architecture is defined in [modeling.py](../../ml/src/modeling.py). It is a small feedforward binary classifier with:

- input layer for normalized feature vectors
- hidden layer with `32` units and `ReLU`
- hidden layer with `16` units and `ReLU`
- dropout rate of `0.15`
- output layer with `1` sigmoid unit named `leak_probability`

The model is compiled with:

- `Adam` optimizer
- `binary_crossentropy` loss
- metrics: accuracy, precision, recall, and AUC

The training script also uses early stopping so that the model restores the best validation weights instead of always taking the final training epoch.

The saved training artifacts include:

- [history.csv](../../ml/models/history.csv)
- [metrics.json](../../ml/models/metrics.json)
- [train_set.csv](../../ml/models/train_set.csv)
- [validation_set.csv](../../ml/models/validation_set.csv)
- [test_set.csv](../../ml/models/test_set.csv)

The current saved metrics report:

- training set size: `112`
- validation set size: `24`
- test set size: `24`
- decision threshold: `0.5`

The saved validation and test metrics are all `1.0` on the current synthetic dataset, which indicates the synthetic classes are very separable in this repository pipeline.

## B.5 Normalization Files

Feature normalization is handled by [scaler_utils.py](../../ml/src/scaler_utils.py). The training script computes the feature means and standard deviations from the training set, then applies standard scaling:

- subtract feature mean
- divide by feature standard deviation

The normalization outputs are saved in two forms:

- JSON file for Python-side reuse: [scaler_params.json](../../ml/models/scaler_params.json)
- C header for firmware deployment: [scaler_params.h](../../ml/exports/firmware_model/scaler_params.h)

The firmware header contains:

- `FEATURE_COUNT`
- the ordered feature list in comments
- `FEATURE_MEANS`
- `FEATURE_STDS`
- an inline `normalize_features(...)` function

This is important because the `ESP32` must use the same scaling values that were used during model training. If the firmware-side normalization order or constants change, the on-device inference output becomes unreliable.

## B.6 TensorFlow Lite Conversion

After training, the Keras model is exported to TensorFlow Lite using [export_tflite.py](../../ml/scripts/export_tflite.py). This script:

- loads the saved `.keras` model
- loads the ordered feature columns
- loads the saved scaler values
- scales representative feature rows
- converts the model to `.tflite`
- writes metadata about the exported model

The repository currently includes both exports:

- float model: [leak_model_float.tflite](../../ml/exports/leak_model_float.tflite)
- int8 model: [leak_model_int8.tflite](../../ml/exports/leak_model_int8.tflite)

The current float model metadata file [leak_model_float_metadata.json](../../ml/exports/leak_model_float_metadata.json) shows:

- input shape: `1 x 13`
- output shape: `1 x 1`
- input dtype: `float32`
- output dtype: `float32`
- model size: `6264` bytes
- external scaling required: `true`

After the `.tflite` file is created, [tflite_to_c_array.py](../../ml/scripts/tflite_to_c_array.py) converts the TensorFlow Lite model into firmware-ready C files:

- [leak_model_data.h](../../ml/exports/firmware_model/leak_model_data.h)
- [leak_model_data.cpp](../../ml/exports/firmware_model/leak_model_data.cpp)

These generated files are the ones copied into the Arduino sketches so the `ESP32` can run the model locally.

## B.7 Model Output Classes

The current TensorFlow model is a binary classifier. The output classes are stored in [label_map.json](../../ml/models/label_map.json) and configured in [experiment.json](../../ml/configs/experiment.json).

The two deployment classes are:

- `no_leak`
- `leak`

Internally, the raw dataset labels are mapped as follows:

- `no_leak -> no_leak`
- `small_leak -> leak`
- `medium_leak -> leak`
- `large_leak -> leak`
- `leak -> leak`

This means the machine-learning model itself does not directly output `small`, `medium`, or `large` as separate classes. Instead, it outputs a single leak probability through the sigmoid output node. The firmware then combines that model score with threshold logic and additional rules to distinguish different leak severities during live operation.

In the embedded firmware, this probability is reported as:

- `LeakProb`

This value is then used together with sound, spectral, and vibration features to determine the final displayed leak level.

## B.8 Appendix B Summary

Appendix B shows that the project’s machine-learning pipeline follows a complete path from structured dataset generation to embedded deployment. The repository contains:

- scripts for generating and organizing training data
- feature extraction code aligned with the embedded feature set
- a compact TensorFlow binary classifier
- standard-scaling files for consistent preprocessing
- TensorFlow Lite export utilities
- firmware conversion tools that package the model into Arduino-ready files

The most important takeaway is that the model deployed on the `ESP32` is not a separate or abstract research artifact. It is directly connected to the project repository, feature pipeline, and firmware deployment files used by the ENKI prototype.

