# Raw Serial Logs

Put raw ESP32 serial log text files here.

Recommended file names:

- `session1_no_leak.txt`
- `session1_leak.txt`
- `session2_no_leak.txt`
- `session2_leak.txt`

Then run:

```powershell
python .\ml\scripts\parse_serial_logs.py
```

Outputs will be written to:

- `docs\parsed_serial_logs\`

The parser will:

- ignore boot and reset text
- extract numeric `SoundLevel` rows
- write raw row CSVs
- write cleaned row CSVs
- write one summary CSV across all logs
