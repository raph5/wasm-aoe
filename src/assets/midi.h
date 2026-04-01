/* Copyright (c) 2025, Raphaël Guyader
 * All rights reserved.
 *
 * This source code is licensed under the GPL-3.0 license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef H_ASSETS_MIDI
#define H_ASSETS_MIDI

#include "src/base.h"

// the max number of tracks that a midi file can containt
#define MIDI_TRACK_MAX 16
#define MIDI_CHANNEL_COUNT 16

typedef struct {
  u16 tick_per_quater_note;
  u16 track_count;
  struct {
    u8 *buf;
    usize buf_len;
  } track[MIDI_TRACK_MAX];
} MidiHeader;

MidiHeader *midi_parse_header(Arena *arena, u8 *bin, usize bin_len);

typedef struct {
  MidiHeader *header;
  u8 track_index;

  u32 buf_index;
  u32 time;
  u32 microsec_per_quater_note;
  u8 running_status;
} MidiPlayer;

typedef enum {
  ME_NOTE_ON,
  ME_PITCH_CHANGE,
  ME_CONTROL_CHANGE,
  ME_PROGRAM_CHANGE,
} MidiEventType;

typedef struct {
  u64 time;
  u8 channel;
  MidiEventType type;
  union {
    struct {
      u8 key;
      u8 velocity;
    } note_on;
    struct {
      u16 pitch;
    } pitch_change;
    struct {
      u8 controller;
      u8 value;
    } control_change;
    struct {
      u8 program;
    } program_change;
  } data;
} MidiEvent;

// Yields the first even that happens before the time indicated my `before`. 
// `before` is expressed in microseconds since the begining of the song. If
// no event can be yield, midi_player_yield_event returns false.
// `ended` is set to true if all events of the MidiPlayer have been yeilded,
// `ended` is set to false otherwise.
b32 midi_player_yield_event(MidiPlayer *player, MidiEvent *event, u64 before, b32 *ended);

#endif
