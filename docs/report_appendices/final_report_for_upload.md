# Leak Detection System Final Report

## Table of Contents

1. Introduction  
2. Project Objectives  
3. System Design  
4. Machine Learning Pipeline  
5. Experimental Setup  
6. Results and Discussion  
7. Conclusion and Future Work  
8. Figures to Insert  
Appendix A. ESP32 / Arduino Firmware  
Appendix B. Machine Learning / TensorFlow Model  
Appendix C. Feature Extraction Equations  

## 1. Introduction

This project developed an embedded leak detection system for water pipe testing using `ESP32` hardware, acoustic sensing, vibration sensing, and lightweight machine learning. The system was built around two device formats: a handheld inspection unit and a clamp-based monitoring unit. The overall goal was to determine whether leak conditions could be identified in real pipe configurations and whether different device placements would support different leak detection roles.

## 2. Project Objectives

The main objectives of the project were:

- design an `ESP32`-based leak detection prototype
- collect and analyze pipe data under no-leak and leak conditions
- extract meaningful sound and vibration features from live sensor data
- train a TensorFlow model and deploy it to the `ESP32`
- compare handheld and clamp-based sensing behavior

## 3. System Design

The system used a microphone, vibration sensor, and environmental sensor connected to the `ESP32`. The microphone captured pipe sound, the vibration sensor captured local mechanical activity, and the environmental sensor provided temperature and humidity reference values. These signals were processed on-device to compute features and generate a leak decision.

The handheld device, referred to as `TRIDENT`, was designed for close-range inspection. It included an `OLED` display for local feedback and was intended to be moved near a suspected leak location.

The clamp device, referred to as `CLAM`, was designed for mounted monitoring on the pipe. Two clamp nodes were used during testing so that readings could be compared across positions and pipe sizes. The clamp firmware also used `RGB LED` feedback to show leak state locally.

The project repository is available at [repository root](../../README.md).

## 4. Machine Learning Pipeline

The machine-learning workflow began with structured audio data and associated metadata. The repository pipeline generated a manifest, extracted features, trained a binary classifier, and exported the trained model to TensorFlow Lite for embedded deployment.

The model inputs included:

- RMS
- peak frequency
- spectral centroid
- low, mid, and high band energy
- zero crossing rate
- vibration magnitude
- vibration mean
- vibration variance
- vibration trend
- temperature
- humidity

The trained model was exported as a `.tflite` file and then converted into Arduino-compatible source files so the `ESP32` could run the inference locally.

## 5. Experimental Setup

Testing was performed on the `Hampden fluid circuit demonstrator` using both `1-inch` and `1/2-inch` pipe sections. The tests focused on four conditions:

- no leak
- small leak
- medium leak
- large leak

For clamp testing, both clamps were first placed on the same `1-inch` pipe at different locations to compare readings between nodes. After that, the second clamp was moved to the `1/2-inch` pipe so that a second dataset could be collected under a different pipe configuration.

For handheld testing, the `TRIDENT` device was placed close to the pipe so that it could be used as a portable inspection tool under no-leak, small-leak, medium-leak, and large-leak conditions.

## 6. Results and Discussion

The handheld device was most useful for close-range leak inspection. Because it could be positioned near the suspected leak location, it was better suited for detecting smaller changes in leak behavior and for supporting leak localization.

The clamp device was more useful for mounted monitoring and comparison across pipe locations. It was especially useful for observing larger leak behavior and for validating how readings changed across different pipe sections.

During `1/2-inch` pipe testing, the data showed that no-leak behavior, small and medium leak behavior, and large leak behavior could produce different sound and vibration patterns. In particular, vibration-related features were often more useful than sound level alone when separating normal flow from leak conditions.

One important limitation is that the current repository model is a binary `no_leak` versus `leak` classifier. Final leak severity interpretation is completed in the firmware using thresholds and additional logic rather than by direct multi-class model output.

## 7. Conclusion and Future Work

This project demonstrated that an `ESP32`-based embedded leak detector can combine sound and vibration sensing with TensorFlow Lite Micro inference for pipe monitoring. The system supports both a handheld inspection workflow and a clamp-based monitoring workflow.

Future work should focus on:

- expanding the real-world pipe dataset
- retraining the model with more real audio and vibration recordings
- improving consistency across pipe sizes and clamp locations
- refining the relationship between binary model output and final leak severity labels

## 8. Figures to Insert

Insert the following figures in the report:

1. Overall system block diagram  
   Source: [pipeline_diagram.svg](../../docs/presentation_assets/pipeline_diagram.svg)

2. Machine learning pipeline diagram  
   Source: [ml_demo_diagram.svg](../../docs/presentation_assets/ml_demo_diagram.svg)

3. Clamp testing setup on the pipe rig  
   Insert a photo showing both clamp nodes mounted during testing.

4. Handheld `TRIDENT` device during testing  
   Insert one no-leak photo and one leak-state photo.

5. `1-inch` and `1/2-inch` pipe comparison setup  
   Insert a photo showing the different pipe locations used for testing.

6. Serial monitor or live feature output screenshot  
   Insert a screenshot showing example feature values such as `SoundLevel`, `SpectralCenter`, `ZeroCross`, `LeakProb`, and vibration values.

7. `1/2-inch` reference table  
   Insert a table or screenshot summarizing no-leak, small/medium leak, and large leak patterns.

## Appendix A. ESP32 / Arduino Firmware

Appendix A contains the `ESP32` firmware used for the ENKI prototype. This firmware handles sensor input, feature extraction, TensorFlow Lite Micro inference, and output control for leak classification.

- Main firmware overview
- Sensor reading code
- Feature extraction code
- TensorFlow Lite Micro inference code
- Output control code

Main repository reference: [ESP32 firmware files](../../docs)

## Appendix B. Machine Learning / TensorFlow Model

Appendix B summarizes the machine-learning repository files, dataset generation flow, feature inputs, training script, normalization files, TensorFlow Lite conversion path, and final output classes.

- repository files
- dataset generation
- feature inputs
- training script
- normalization files
- TensorFlow Lite conversion
- model output classes

Main repository reference: [ML pipeline](../../ml)

## Appendix C. Feature Extraction Equations

Appendix C contains the main equations used to compute the sensor features passed into the embedded model.

- RMS
- peak frequency
- spectral centroid
- zero crossing rate
- vibration calculations

Short reference: [Feature extraction sketch](../../docs/feature_extraction_equations_sketch.ino)

