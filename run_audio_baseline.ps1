$ErrorActionPreference = "Stop"

Write-Host "Running ESP32 leak-model pipeline..."

python ml\scripts\generate_synthetic_audio.py
python ml\scripts\build_manifest.py
python ml\scripts\extract_features.py
python ml\scripts\train_model.py
python ml\scripts\evaluate_model.py
python ml\scripts\export_tflite.py --quantize none
python ml\scripts\export_tflite.py --quantize int8
python ml\scripts\tflite_to_c_array.py

Write-Host "Pipeline complete."
Write-Host "Model artifacts: ml\models"
Write-Host "Firmware artifacts: ml\exports\firmware_model"
