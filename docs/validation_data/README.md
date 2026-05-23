# Validation Data

This folder contains the real collected validation material included in the public ENKI repository snapshot.

## What Is Included

### 1-inch Pipe Clamp Data

The `raw_serial_logs/` folder includes real ESP32 serial captures from the `1-inch` pipe clamp testing runs, including:

- run 1 captures
- run 2 `60-second` captures
- run 3 `r6` captures
- capture notes describing run order and test consistency guidance

These files are the closest raw record of the clamp runs collected on the physical pipe rig.

### Handheld / Microphone Session Data

The `handheld_sessions/` folder includes the cleaned documentation for the microphone-focused handheld sessions:

- organized session labels
- session quality notes
- session 1 datasheet
- collection log CSV

The `raw_serial_logs/` folder also includes:

- `session1_no_leak.txt`
- `session1_leak.txt`
- `session2_no_leak.txt`
- `session2_leak.txt`

These represent the microphone-only leak and no-leak session captures discussed during model retraining and validation review.

### Parsed / Cleaned Session Data

The `parsed_serial_logs/` folder contains cleaned and organized CSV outputs derived from the raw serial logs, including:

- per-session cleaned rows
- per-session organized rows
- combined clean CSVs
- summary CSVs
- report-ready markdown tables

These files are useful when reviewing signal quality, invalid rows, and basic leak-vs-no-leak separation without re-parsing the raw serial text.

### 1/2-inch Pipe Clamp Data

The `half_inch_pipe/` folder contains the summarized `1/2-inch` clamp reference table used during the later validation and threshold-tuning work.

This public repo snapshot includes the `1/2-inch` data primarily as a cleaned reference summary rather than as one uniform raw log bundle. Those runs were collected iteratively during tuning, so the most reliable public artifact is the observed-band summary in:

- `half_inch_pipe_reference_table.md`

## Suggested Reading Order

1. `raw_serial_logs/11in_capture_notes.txt`
2. `raw_serial_logs/11in_run2_60s/11in_run2_60s_notes.txt`
3. `raw_serial_logs/11in_run3_r6/11in_run3_r6_notes.txt`
4. `parsed_serial_logs/session_report_table.md`
5. `handheld_sessions/organized_mic_sessions.md`
6. `half_inch_pipe/half_inch_pipe_reference_table.md`

## Why This Data Is Included

This validation material helps show that the project was not only a synthetic ML pipeline. It also included:

- repeated clamp testing on a physical `1-inch` pipe
- follow-up tuning on a `1/2-inch` pipe
- handheld microphone session comparisons
- cleaned summaries and notes used to interpret noisy real-world embedded data
