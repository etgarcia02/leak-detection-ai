# 1/2-Inch Pipe Reference Table

Observed values from the Node 1 half-inch pipe test runs shared on May 7, 2026.

These are practical observed bands from the runs, not strict firmware thresholds.

## Main Table

| Condition | SoundLevel | SpectralCenter | ZeroCross | LeakProb | VibrationMean | VibrationSustain | Notes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| No leak baseline | `21k-29k` | `1300-2000` | `0.0157-0.0945` | usually `0.0000x-0.01` | `430-600` | `470-550` | Stable no-leak half-inch baseline. `Trigger:none` is correct here. |
| Small/medium leak cluster | `14k-26k` | `1700-2300` | `0.03-0.13` | often `0.001-0.03`, sometimes higher spikes | `700-950` | `720-840` | This was the earlier half-inch leak pattern. Best separator vs no leak was the higher vibration floor. |
| Large leak, low-sound pattern | `4k-12k` | `1600-2700` | `0.0157-0.20` | often `0.90-0.999` | `450-650` | `500-580` | This is the large-leak pattern that drove the `half_large_low_sound` logic. |
| Large leak clipped companion frames | `30k-32767` | `1400-1900` | `0.0079-0.0157` | near `0.0000x` | `450-650` | `500-560` | These appeared mixed into large-leak runs but looked like clipping/saturation, not the primary leak signature. |

## Quick Interpretation

| Condition | Best Visual Clue |
| --- | --- |
| No leak | High `SoundLevel`, low `LeakProb`, lower vibration band |
| Small/medium leak | Higher vibration than no-leak, even when sound overlaps |
| Large leak | Low `SoundLevel` plus very high `LeakProb` repeated across frames |

## Best Distinguishers

| Comparison | Most Useful Feature |
| --- | --- |
| No leak vs small/medium leak | `VibrationMean` and `VibrationSustain` |
| No leak vs large leak | repeated very high `LeakProb` with low `SoundLevel` |
| Large leak true signal vs clipped frames | low-sound/high-probability frames are the real signal; `32767` frames are likely clipping artifacts |
