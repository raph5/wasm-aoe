/* Copyright (c) 2025, Raphaël Guyader
 * All rights reserved.
 *
 * This source code is licensed under the GPL-3.0 license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef H_LIB_AUDIO
#define H_LIB_AUDIO

#include "src/base.h"

// an f32 PCM stereo signal sampled at AL_RATE Hz
// each sample must stay between -1 and 1
typedef struct {
  u32 sample_rate;
  u32 sample_count;
  u32 channel_count;

  // if Sample is mono buf simply stores the samples of the mono channel
  // if Sample is stereo buf stores the samples of the two channels following
  // this layout: AAAAAAAABBBBBBBB where A represent the samples of first
  // channel and B the samples of the second channel.
  f32 *buf;
} Sample;

Sample sample_alloc(Arena *arena, u32 sample_rate, u32 sample_count, u32 channel_count);
Sample sample_resample_fast(Arena *arena, Sample input, u32 output_rate);
// sample_resmaple uses the bandlimited intrepolation algorithm described in
// docs/resample.pdf
// This algorithme trims the first and last 13 samples from the original sample
// out of the resampled sample. To avoid loosing these 13 samples you can set
// `trim` to false. This will add 13 zeros and the start and the end of the
// input sample. That way sample_resample will trim the 13 zeros instead of
// trimming samples from the original sample.
Sample sample_resample(Arena *arena, Sample input, u32 output_rate, b32 trim);

typedef struct {
  Sample sample;
  u32 index;
} Audio;

typedef struct {
} Music;

#define AC_AUDIO_CAP 512
#define AC_MUSIC_CAP 8
typedef struct {
  u32 audio_count;
  f32 audio_gain[AC_AUDIO_CAP];
  Audio audio[AC_AUDIO_CAP];
  u32 music_count;
  f32 music_gain[AC_MUSIC_CAP];
  Music music[AC_MUSIC_CAP];
} AudioContext;

void audio_context_play_audio(AudioContext *context, Audio audio, f32 gain);
void audio_context_push_samples(Arena *arena, AudioContext *context);  // calls al_push_samples

#endif
