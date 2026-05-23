#pragma once

#define FEATURE_COUNT 13

// Feature order used by the ESP32 model.
// 0: rms
// 1: peak_frequency_hz
// 2: spectral_centroid_hz
// 3: low_band_energy
// 4: mid_band_energy
// 5: high_band_energy
// 6: zero_crossing_rate
// 7: temperature_c
// 8: humidity_pct
// 9: vibration_magnitude
// 10: vibration_mean
// 11: vibration_variance
// 12: vibration_trend

static const float FEATURE_MEANS[FEATURE_COUNT] = {12939.06933594f, 1160.71423340f, 2379.68090820f, 1055878610944.00000000f, 720262987776.00000000f, 104713863168.00000000f, 0.24592240f, 0.00000000f, 0.00000000f, 0.00000000f, 0.00000000f, 0.00000000f, 0.00000000f};
static const float FEATURE_STDS[FEATURE_COUNT] = {2287.07250977f, 1543.40368652f, 220.48286438f, 1494494609408.00000000f, 422587695104.00000000f, 115956244480.00000000f, 0.11471006f, 1.00000000f, 1.00000000f, 1.00000000f, 1.00000000f, 1.00000000f, 1.00000000f};

inline void normalize_features(const float* input, float* output) {
  for (int i = 0; i < FEATURE_COUNT; ++i) {
    output[i] = (input[i] - FEATURE_MEANS[i]) / FEATURE_STDS[i];
  }
}
