/* Copyright (c) 2025, Raphaël Guyader
 * All rights reserved.
 *
 * This source code is licensed under the GPL-3.0 license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "src/base.h"
#include "src/base.c"

#include "src/lib/audio_wave.h"
#include "src/lib/audio_wave.c"
#include "src/lib/audio.h"
#include "src/lib/audio.c"

#include "src/assets/pack.h"
#include "src/assets/pack.c"
#include "src/assets/drs.h"
#include "src/assets/drs.c"
#include "src/assets/pal.h"
#include "src/assets/pal.c"
#include "src/assets/slp.h"
#include "src/assets/slp.c"
#include "src/assets/wav.h"
#include "src/assets/wav.c"
#include "src/assets/sf2.h"
#include "src/assets/sf2.c"
#include "src/assets/midi.h"
#include "src/assets/midi.c"

#include "src/ui.h"
#include "src/ui.c"

#define UI_EVENT_QUEUE_CAP 1024
typedef struct {
  u64 time;
  // usize ui_event_queue_len;
  // UiEvent ui_event_queue[UI_EVENT_QUEUE_CAP];
} GameStateTickInput;

GameStateTickInput game_state_tick_input;
GameStateTickInput *game_state_tick_input_ptr = &game_state_tick_input;

extern const unsigned char *binary_assets_start;
extern const unsigned char *binary_assets_end;

Gpa gpa;
Arena static_arena = { .allocator = &gpa, .region_size = MiB(1) };  // static lifetime arena
Arena frame_arena = { .allocator = &gpa, .region_size = MiB(1) };
Arena audio_arena = { .allocator = &gpa, .region_size = MiB(1) };
Arena state_arena = { .allocator = &gpa, .region_size = MiB(1) };

SlpHeader *slp;
PalPalette *palette;
AudioPlayer arrow_audio, taunt_audio, key_audio;
AudioContext audio_context = {0};

void game_init(void) {
  gpa_init(&gpa);

  PackHeader *pack_header = pack_header_parse(&static_arena, (u8 *) binary_assets_start, binary_assets_end - binary_assets_start);

  usize interface_drs_len;
  u8 *interface_drs = pack_file_get(pack_header, string8_static("game/data/interfac.drs"), &interface_drs_len);
  DrsHeader *interface_drs_header = drs_header_parse(&static_arena, interface_drs, interface_drs_len);

  usize graphics_drs_len;
  u8 *graphics_drs = pack_file_get(pack_header, string8_static("game/data/graphics.drs"), &graphics_drs_len);
  DrsHeader *graphics_drs_header = drs_header_parse(&static_arena, graphics_drs, graphics_drs_len);

  usize sounds_drs_len;
  u8 *sounds_drs = pack_file_get(pack_header, string8_static("game/data/sounds.drs"), &sounds_drs_len);
  DrsHeader *sounds_drs_header = drs_header_parse(&static_arena, sounds_drs, sounds_drs_len);

  usize wonder_slp_file_len;
  u8 *wonder_slp_file = drs_file_get(graphics_drs_header, string8_static("210.slp"), &wonder_slp_file_len);
  usize palette_file_len;
  u8 *palette_file = drs_file_get(interface_drs_header, string8_static("50500.bina"), &palette_file_len);
  palette = pal_parse(&static_arena, palette_file, palette_file_len);
  slp = slp_parse_header(&static_arena, wonder_slp_file, wonder_slp_file_len);

  usize arrow_wav_file_len;
  u8 *arrow_wav_file = drs_file_get(sounds_drs_header, string8_static("5201.wav"), &arrow_wav_file_len);
  WavHeader *arrow_wav = wav_parse_header(&static_arena, arrow_wav_file, arrow_wav_file_len);
  Wave arrow_decoded = wav_decode(&static_arena, arrow_wav);
  arrow_audio = (AudioPlayer) {
    .wave = wave_resample(&static_arena, arrow_decoded, AL_RATE),
  };

  usize taunt_wav_file_len;
  u8 *taunt_wav_file = pack_file_get(pack_header, string8_static("game/sound/taunt006.wav"), &taunt_wav_file_len);
  WavHeader *taunt_wav = wav_parse_header(&static_arena, taunt_wav_file, taunt_wav_file_len);
  Wave taunt_decoded = wav_decode(&static_arena, taunt_wav);
  taunt_audio = (AudioPlayer) {
    .wave = wave_resample(&static_arena, taunt_decoded, AL_RATE),
  };

  usize soundfont_file_len;
  u8 *soundfont_file = pack_file_get(pack_header, string8_static("gm.sf2"), &soundfont_file_len);
  Sf2Header *soundfont = sf2_parse_header(&static_arena, soundfont_file, soundfont_file_len);

  usize music_file_len;
  u8 *music_file = pack_file_get(pack_header, string8_static("game/sound/music1.mid"), &music_file_len);
  MidiHeader *music = midi_parse_header(&static_arena, music_file, music_file_len);
  // log_print("music->track_count: %u32", music->track_count);

  gl_set_resolution(640, 480);
}

usize i = 0;
void game_frame_tick(void) {
  arena_reset(&frame_arena);

  if (i == 50) {
    // audio_context_play_audio(&audio_context, taunt_audio, 1);
  }
  if (i == 90) {
    audio_context_play_audio(&audio_context, arrow_audio, 0.2);
  }

  // usize frame_index = 0;
  usize frame_index = 2*13 + (i/8) % 13;
  Image *frame = slp_shape_render(&frame_arena, slp, palette, frame_index, (SlpOptions) { .player_id = 1, .flipped = true });
  u32 texture_id = gl_texture_create(frame->buf, frame->width, frame->height);

  gl_sprite sprite = {
    // .x = 64 - slp->shape_table[frame_index].center_x,
    .x = 60 - slp->shape_table[frame_index].width + slp->shape_table[frame_index].center_x,
    .y = 80 - slp->shape_table[frame_index].center_y,
    .width = slp->shape_table[frame_index].width,
    .height = slp->shape_table[frame_index].height,
    .uv_x = 0,
    .uv_y = 0,
    .texture_id = texture_id,
  };
  gl_clear_screen();
  gl_draw_sprite(&sprite);

  gl_texture_destroy(texture_id);
  i += 1;
}

void game_audio_tick(void) {
  arena_reset(&audio_arena);
  audio_context_push_samples(&audio_arena, &audio_context);
}

void game_state_tick(void) {
}
