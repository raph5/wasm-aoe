/* Copyright (c) 2025, Raphaël Guyader
 * All rights reserved.
 *
 * This source code is licensed under the GPL-3.0 license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef H_LIB_AUDIO
#define H_LIB_AUDIO

#include "src/base.h"
#include "src/lib/audio_wave.h"
#include "src/assets/midi.h"
#include "src/assets/sf2.h"

typedef struct {
  Wave wave;
  u32 index;
} AudioPlayer;

// returns a pointer to and array of `batch_len` samples generated from the
// audio source
// `ended` is set to true if all audio samples have been yielded, `ended` is set
// to false otherwise
f32 *audio_yield_batch(Arena *arena, AudioPlayer *audio, u32 batch_len, b32 *ended);

typedef struct {
  u32 sample_index;
  u32 time_register_remainder;
  u32 index;
  f32 margin[2 * WAVE_RESAMPLE_MARGIN];

  // TODO: add is_looping, pitch...
} SynthOscillator;

// returns a pointer to and array of `batch_len` samples generated from the
// audio source
// `ended` is set to true if all audio samples have been yielded, `ended` is set
// to false otherwise
void synth_oscillator_yeild_batch(Arena *arena, SynthOscillator *oscillator, u32 batch_len, b32 *ended);

#define SYNTH_VOICE_COUNT 128
typedef struct {
  Sf2Header *soundfont;
  MidiPlayer midi_player;

  u64 time;  // time in microseconds since the start of the midi track
  struct {
    u16 bank;
    u8 program;
    u8 volume;
    u8 pan;
  } channel[MIDI_CHANNEL_COUNT];

  usize voice_count;
  struct {
    SynthOscillator oscilator;
  } voice[SYNTH_VOICE_COUNT];
} Synth;

// returns a pointer to and array of `batch_len` samples generated from the
// audio source
// `ended` is set to true if all audio samples have been yielded, `ended` is set
// to false otherwise
f32 *synth_yield_batch(Arena *arena, Synth *synth, u32 batch_len, b32 *ended);

typedef struct {
  u32 track_count;
  Synth track[MIDI_TRACK_MAX];
} MusicPlayer;

void music_player_init(MusicPlayer *player, Sf2Header *soundfont, MidiHeader *midi);
// returns a pointer to and array of `batch_len` samples generated from the
// audio source
// `ended` is set to true if all audio samples have been yielded, `ended` is set
// to false otherwise
f32 *music_player_yield_batch(Arena *arena, MusicPlayer *player, u32 batch_len, b32 *ended);

#define AC_AUDIO_CAP 512
#define AC_MUSIC_CAP 4
typedef struct {
  u32 audio_count;
  f32 audio_gain[AC_AUDIO_CAP];
  AudioPlayer audio[AC_AUDIO_CAP];

  u32 music_count;
  f32 music_gain[AC_MUSIC_CAP];
  MusicPlayer music[AC_MUSIC_CAP];
} AudioContext;

void audio_context_play_audio(AudioContext *context, AudioPlayer audio, f32 gain);
void audio_context_push_samples(Arena *arena, AudioContext *context);  // calls al_push_samples

#endif
