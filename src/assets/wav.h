/* Copyright (c) 2025, Raphaël Guyader
 * All rights reserved.
 *
 * This source code is licensed under the GPL-3.0 license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef H_ASSET_WAV
#define H_ASSET_WAV

#include "src/base.h"
#include "src/lib/audio.h"

typedef struct {
  u32 sample_rate;
  u32 sample_count;
  u8 *data;
  u8 channel_count;
  u8 bits_per_sample;
} WavHeader;

WavHeader *wav_parse_header(Arena *arena, u8 *bin, usize bin_len);
Audio wav_decode(Arena *arena, WavHeader *header);

#endif
