# Organized Microphone Sessions

## What this is

This is the cleaned-up session organizer for the microphone-only tests you already ran.

It does **not** pretend every raw serial row has been perfectly extracted from chat text.
Instead, it organizes the sessions you collected into a practical datasheet you can use right now for notes, training decisions, and future data collection.

## Sessions

| Session ID | Session | Label | Quality | What it means | Recommended use |
| --- | --- | --- | --- | --- | --- |
| `NL_S1` | Session 1 | `no_leak` | `usable_with_cleaning` | Good negative example, but still has some false spikes | Keep after cleaning |
| `L_S1` | Session 1 | `leak` | `usable_with_cleaning` | Stronger leak-like acoustic bursts than `NL_S1` | Keep after cleaning |
| `NL_S2` | Session 2 | `no_leak` | `usable_with_cleaning` | Noisier negative example with invalid frames mixed in | Keep with caution after cleaning |
| `L_S2` | Session 2 | `leak` | `usable_with_cleaning` | Leak-like behavior is present, but not as clean as Session 1 | Keep with caution after cleaning |

## Main takeaways

- `Session 1` is your better comparison pair.
- `L_S1` looked more leak-like than `NL_S1`.
- `Session 2` still showed a leak vs no-leak difference, but it was much messier.
- The large motor likely affected both sessions by adding environmental noise to the microphone.

## Cleaning rules for all four sessions

- remove rows where `SoundLevel = 0`
- remove rows where `PeakFreq = 0` and `SpectralCenter = 0`
- remove rows with startup text, reset text, or brownout text
- remove obviously saturated rows where `SoundLevel >= 32000`
- ignore `VibrationGate` because these were microphone-only tests

## Best current training set judgment

### Stronger pair

- `NL_S1`
- `L_S1`

These are the better pair to use first when building your real mic dataset.

### Weaker but still useful pair

- `NL_S2`
- `L_S2`

These are still useful, but they should be marked as noisier examples.

## Notebook-ready summary

`The microphone-only testing sessions were organized into four labeled runs: NL_S1, L_S1, NL_S2, and L_S2. Session 1 showed the clearest leak-vs-no-leak difference, with L_S1 producing more high spectral-centroid and high zero-crossing bursts than NL_S1. Session 2 also showed some separation, but both runs were more contaminated by invalid frames and environmental noise. All four sessions should be cleaned before use in training or evaluation.`
