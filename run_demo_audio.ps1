$ErrorActionPreference = "Stop"

Write-Host "Generating audible synthetic demo WAV files..."
python ml\scripts\generate_demo_audio.py
Write-Host "Done."
Write-Host "Output folder: ml\data\demo_audio"
