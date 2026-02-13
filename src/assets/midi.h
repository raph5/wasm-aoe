/* Copyright (c) 2025, Raphaël Guyader
 * All rights reserved.
 *
 * This source code is licensed under the GPL-3.0 license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef H_ASSETS_MIDI
#define H_ASSETS_MIDI

#include "src/base.h"

typedef struct {
  u8 *buf;
  usize buf_len;
} MidiTrackHeader;

typedef struct {
  u16 division;
  u16 track_count;
  MidiTrackHeader track[];
} MidiHeader;

typedef struct {
} MidiPlayer;

MidiHeader *midi_parse_header(Arena *arena, u8 *bin, usize bin_len);

#endif
