# Report-Ready Session Table

## Real Pipe Microphone Sessions

| Session | Label | Parsed Rows | Valid Rows | Invalid Rows | Valid % | Mean Sound Level | Mean Spectral Center (Hz) | Mean Zero-Cross Rate | Mean Leak Prob | High-Prob Rows (`>= 0.90`) |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Session 1 | No Leak | 361 | 358 | 3 | 99.2% | 15243.20 | 2298.55 | 0.1647 | 0.2721 | 26 |
| Session 1 | Leak | 694 | 657 | 37 | 94.7% | 16628.54 | 2349.64 | 0.1542 | 0.2689 | 111 |
| Session 2 | No Leak | 279 | 254 | 25 | 91.0% | 16638.13 | 1972.29 | 0.1138 | 0.2364 | 32 |
| Session 2 | Leak | 301 | 218 | 83 | 72.4% | 17759.97 | 1927.65 | 0.0801 | 0.2298 | 37 |

## Session-Level Interpretation

| Session | Comparison | Interpretation |
| --- | --- | --- |
| Session 1 | Leak vs No Leak | Session 1 showed the clearer leak-vs-no-leak separation. The leak run had many more high-probability leak rows than the no-leak run and is the stronger positive example pair overall. |
| Session 2 | Leak vs No Leak | Session 2 was noisier and had substantially more invalid rows, especially in the leak run. It is still usable, but it should be treated as a weaker comparison pair than Session 1. |

## Suggested Report Note

`Session 1 provided the clearest microphone-only separation between leak and no-leak conditions, while Session 2 was more contaminated by invalid frames and environmental noise. All sessions remain usable after cleaning, but Session 1 is the stronger pair for initial reporting and model retraining.`
