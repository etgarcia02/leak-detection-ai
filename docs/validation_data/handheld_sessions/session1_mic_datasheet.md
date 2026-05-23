# Session 1 Microphone Datasheet

## Scope

- Sessions compared: `no_leak session 1` and `leak session 1`
- Sensor setup: `INMP441` microphone only
- Vibration sensor: not connected
- Environment note: a large motor was audible during testing
- Source format: serial logs pasted from the ESP32 live model sketch

## Important interpretation note

- `VibrationGate` should be ignored for these two sessions because the vibration sensor was not used.
- Any reset text, brownout text, or boot messages are not signal data and should be removed before analysis.

## Session summary

| Session ID | Label | Signal quality | Main acoustic pattern | Training use |
| --- | --- | --- | --- | --- |
| `NL_S1` | `no_leak` | Mixed quality | Mostly mid-range spectral centroid and zero-crossing values, with occasional false high-probability bursts | Keep after cleaning |
| `L_S1` | `leak` | Mixed quality | More frequent high spectral-centroid and high zero-crossing bursts, with more leak-like model spikes | Keep after cleaning |

## What was different

- `leak session 1` showed more repeated bursts with higher `SpectralCenter`, higher `ZeroCross`, and more windows where `ModelGate` became `1`.
- `no_leak session 1` still contained some high-probability spikes, so the classes are not perfectly separated.
- The leak session looked more turbulent overall, which is consistent with the expected leak sound signature.

## Data quality issues seen in the logs

- repeated rows with `SoundLevel: 0.00`
- repeated rows with `PeakFreq: 0.00` and `SpectralCenter: 0.00`
- saturated or startup-like frames near `SoundLevel: 32000+`
- ESP32 reset and boot text inside the leak session log
- brownout/reset messages inside the leak session log

## Cleaning rules to use

- drop any row where `SoundLevel = 0`
- drop any row where `PeakFreq = 0` and `SpectralCenter = 0`
- drop boot text, reset text, and brownout messages
- drop obviously saturated frames where `SoundLevel >= 32000`
- treat long low-signal sections with `LeakProb` pinned near `1.0` after resets as invalid unless they are independently verified

## Confidence and recommendation

- Separation result: `moderate`
- Conclusion: these two sessions do show a real leak vs no-leak difference, but the logs are noisy enough that they should be treated as cleaned training candidates, not final benchmark data.
- Best next step: keep both sessions in the dataset, mark them as `usable_with_cleaning`, and continue collecting shorter clean runs with the same mic position.

## Suggested dataset labels

| Session ID | Recommended label | Use flag | Reason |
| --- | --- | --- | --- |
| `NL_S1` | `no_leak` | `usable_with_cleaning` | Good negative example, but includes false spikes and invalid frames |
| `L_S1` | `leak` | `usable_with_cleaning` | Good positive example with clearer high-frequency burst behavior |

## Notebook-ready summary

`Session 1 microphone testing showed a meaningful difference between no-leak and leak conditions. The leak session produced more frequent high spectral-centroid and high zero-crossing bursts than the no-leak session, suggesting a more turbulent acoustic signature. Both logs contained invalid frames and reset-related artifacts, so the sessions should be cleaned before use in training or evaluation.`
