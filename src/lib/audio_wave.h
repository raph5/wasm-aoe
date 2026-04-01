/* Copyright (c) 2025, Raphaël Guyader
 * All rights reserved.
 *
 * This source code is licensed under the GPL-3.0 license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef H_LIB_AUDIO_WAVE
#define H_LIB_AUDIO_WAVE

#include "src/base.h"

// an f32 PCM mono signal sampled at AL_RATE Hz
// each value of the sample must stay between -1 and 1
typedef struct {
  u32 sample_rate;
  u32 sample_count;
  f32 *buf;
} Wave;

Wave wave_alloc(Arena *arena, u32 sample_rate, u32 sample_count);
Wave wave_resample_fast(Arena *arena, Wave input, u32 output_rate);

#define WAVE_RESAMPLE_MARGIN 13
// wave_resample uses the bandlimited intrepolation algorithm described in
// docs/resample.pdf
Wave wave_resample(Arena *arena, Wave input, u32 output_rate);
// whereas `wave_resample` is a general purpose resampling function,
// `wave_resample_running` is only meant to be used by the SynthOscillator
Wave wave_resample_running(Arena *arena, Wave input, u32 output_rate, u32 *time_register_remainder);

#endif
