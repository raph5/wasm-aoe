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

  // if Audio is mono buf simply stores the samples of the mono channel
  // if Audio is stereo buf stores the samples of the two channels following
  // this layout: AAAAAAAABBBBBBBB where A represent the samples of first
  // channel and B the samples of the second channel.
  f32 *buf;
} Audio;

Audio audio_alloc(Arena *arena, u32 sample_rate, u32 sample_count, u32 channel_count);
Audio audio_resample_fast(Arena *arena, Audio input, u32 output_rate);
// audio_resmaple uses the bandlimited intrepolation algorithm described in
// docs/resample.pdf
// This algorithme trims the first and last 13 samples from the original audio
// out of the resampled audio. To avoid loosing these 13 samples you can set
// `trim` to false. This will add 13 zeros and the start and the end of the
// input audio. That way audio_resample will trim the 13 zeros instead of
// trimming samples from the original audio.
Audio audio_resample(Arena *arena, Audio input, u32 output_rate, b32 trim);

#define AUDIO_CONTEXT_CAP 512
typedef struct {
  usize source_count;
  struct {
    Audio audio;
    f32 gain;
    u32 index;  // the index of the next sample to be played
  } source[AUDIO_CONTEXT_CAP];
} AudioContext;

void audio_context_play(AudioContext *context, Audio audio, f32 gain);
void audio_context_push_samples(Arena *arena, AudioContext *context);  // calls al_push_samples

#endif
