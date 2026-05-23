# Appendix C. Feature Extraction Equations

This appendix summarizes the main signal-processing equations used to compute the acoustic and vibration features in the embedded leak detection system. These features were extracted from live sensor data on the ESP32 and then used for threshold logic and embedded model inference.

## C.1 Audio Window Definition

Let the sampled audio window be

\[
x[n], \quad n = 0,1,2,\dots,N-1
\]

where:

- \(N = 128\) samples per analysis window
- \(f_s = 16000 \, \text{Hz}\) sample rate

## C.2 Sound Level

The sound level was computed as the root-mean-square (RMS) value of the audio window:

\[
\text{SoundLevel} = \sqrt{\frac{1}{N}\sum_{n=0}^{N-1} x[n]^2 + 10^{-12}}
\]

This value provides a compact measurement of the overall acoustic energy in each sampled window.

## C.3 Zero-Crossing Rate

The zero-crossing rate measures how often the waveform changes sign:

\[
\text{ZeroCross} = \frac{1}{N-1}\sum_{n=1}^{N-1} I\big(\text{sign}(x[n]) \neq \text{sign}(x[n-1])\big)
\]

where \(I(\cdot)\) is an indicator function equal to 1 when the condition is true and 0 otherwise.

This feature helps describe the noisiness or texture of the signal.

## C.4 Spectral Features

The discrete Fourier transform magnitude of each frequency bin was computed as:

\[
X[k] = \sum_{n=0}^{N-1} x[n] e^{-j2\pi kn/N}
\]

\[
|X[k]| = \sqrt{\Re(X[k])^2 + \Im(X[k])^2}
\]

The frequency corresponding to bin \(k\) is:

\[
f_k = \frac{k f_s}{N}
\]

### Peak Frequency

The peak frequency is the frequency bin with maximum magnitude:

\[
k_{\max} = \arg\max_k |X[k]|
\]

\[
\text{PeakFrequency} = \frac{k_{\max} f_s}{N}
\]

### Spectral Center (Spectral Centroid)

The spectral center was computed as the magnitude-weighted mean frequency:

\[
\text{SpectralCenter} =
\frac{\sum_k f_k |X[k]|}{\sum_k |X[k]|}
\]

This feature helps describe where the energy of the signal is concentrated in frequency.

## C.5 Band Energy Features

Band energies were computed using squared spectral magnitude:

\[
P[k] = |X[k]|^2
\]

The frequency bands used were:

- low band: \(f_k < 1000 \, \text{Hz}\)
- mid band: \(1000 \leq f_k < 4000 \, \text{Hz}\)
- high band: \(4000 \leq f_k \leq 8000 \, \text{Hz}\)

Thus,

\[
\text{LowBandEnergy} = \sum_{f_k < 1000} P[k]
\]

\[
\text{MidBandEnergy} = \sum_{1000 \leq f_k < 4000} P[k]
\]

\[
\text{HighBandEnergy} = \sum_{4000 \leq f_k \leq 8000} P[k]
\]

These features were included to separate low-frequency flow behavior from higher-frequency leak behavior.

## C.6 Vibration Feature Extraction

For the three-axis vibration sensor, baseline-corrected motion was calculated from the difference between each live reading and its baseline value.

Let the raw sensor value on axis \(a\) at burst sample \(i\) be \(r_{a,i}\), and let the baseline value be \(b_a\).

The axis difference is:

\[
\Delta_{a,i} = |r_{a,i} - b_a|
\]

The combined raw vibration magnitude is:

\[
M_i = \sqrt{\Delta_{x,i}^2 + \Delta_{y,i}^2 + \Delta_{z,i}^2}
\]

The firmware then scales this value:

\[
\delta_i = \text{round}(s \cdot M_i)
\]

where \(s\) is the vibration scaling constant used in the firmware.

### Vibration Delta

The vibration delta feature is the peak burst value:

\[
\text{VibrationDelta} = \max_i \delta_i
\]

### Vibration Burst Mean

If \(B\) samples are collected in one burst, then:

\[
\text{BurstMean} = \frac{1}{B}\sum_{i=1}^{B} \delta_i
\]

### Active Vibration Mean

Only burst samples above the activity threshold are treated as active vibration samples. If the active set is

\[
A = \{ i \mid \delta_i \geq T \}
\]

then the active vibration mean is:

\[
\text{VibrationMean} =
\frac{1}{|A|}\sum_{i \in A} \delta_i
\]

### Vibration Variance

The active vibration variance is:

\[
\text{VibrationVar} =
\frac{1}{|A|}\sum_{i \in A} \delta_i^2
- \left(\text{VibrationMean}\right)^2
\]

If this value becomes negative due to numerical rounding, it is clamped to zero.

### Vibration Trend

The vibration trend is the positive rise in active mean between the current and previous window:

\[
\text{VibrationTrend} =
\max\left(\text{VibrationMean}_t - \text{VibrationMean}_{t-1}, \, 0\right)
\]

### Vibration Sustain

The vibration sustain feature is an exponential moving average of burst mean:

\[
\text{VibrationSustain}_t =
\alpha \cdot \text{BurstMean}_t
 + (1-\alpha)\cdot \text{VibrationSustain}_{t-1}
\]

where \(\alpha\) is the sustain smoothing constant used in the firmware.

This feature captures whether vibration remains elevated over time rather than appearing only as a short transient.

## C.7 Leak Probability

After feature extraction, the feature vector was normalized and passed into the embedded TensorFlow Lite model. The resulting model output was reported as:

\[
\text{LeakProb} \in [0,1]
\]

This value is not a directly measured sensor feature. Instead, it is the model's estimated probability that the current window corresponds to a leak-related condition.

## C.8 Summary

The final feature set combined:

- time-domain audio energy
- frequency-domain spectral distribution
- zero-crossing behavior
- engineered vibration features
- model-based leak probability

Together, these features were used to distinguish no-leak, small leak, medium leak, and large leak conditions in the embedded system.
