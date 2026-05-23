# Session Datasheet

These values were parsed from your raw serial logs and organized into Session 1 and Session 2.

## Session 1

| Label | Parsed rows | Valid rows | Invalid rows | Mean sound level | Mean spectral center | Mean zero cross | Mean leak prob | High-prob rows (`>= 0.90`) |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `leak` | 694 | 657 | 37 | 16628.537245 | 2349.640472 | 0.154175 | 0.268907 | 111 |
| `no_leak` | 361 | 358 | 3 | 15243.196788 | 2298.546788 | 0.164701 | 0.272082 | 26 |

## Session 2

| Label | Parsed rows | Valid rows | Invalid rows | Mean sound level | Mean spectral center | Mean zero cross | Mean leak prob | High-prob rows (`>= 0.90`) |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `leak` | 301 | 218 | 83 | 17759.965459 | 1927.649725 | 0.080050 | 0.229757 | 37 |
| `no_leak` | 279 | 254 | 25 | 16638.128898 | 1972.291772 | 0.113805 | 0.236375 | 32 |

## Files you can open

- `session_1_organized.csv`: all parsed rows from Session 1
- `session_1_clean.csv`: cleaned rows from Session 1
- `session_2_organized.csv`: all parsed rows from Session 2
- `session_2_clean.csv`: cleaned rows from Session 2
- `serial_log_summary.csv`: per-file summary across all four logs

## Notes

- `valid_rows` use the cleaning rules we discussed:
  - remove rows where `SoundLevel = 0`
  - remove rows where `PeakFreq = 0` and `SpectralCenter = 0`
  - remove obviously saturated rows where `SoundLevel >= 32000`
  - remove low-signal rows with pinned high probability
- `VibrationGate` is still present in the logs, but these were microphone-only sessions.
