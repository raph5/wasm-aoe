#include "src/base.h"
#include "src/base.c"
#include "src/ui.h"
#include "src/ui.c"
#include "src/assets/pack.h"
#include "src/assets/pack.c"
#include "src/assets/drs.h"
#include "src/assets/drs.c"
#include "src/assets/pal.h"
#include "src/assets/pal.c"
#include "src/assets/slp.h"
#include "src/assets/slp.c"


#define UI_EVENT_QUEUE_CAP 1024
typedef struct {
  u64 time;
  // usize ui_event_queue_len;
  // UiEvent ui_event_queue[UI_EVENT_QUEUE_CAP];
} GameUpdateInput;

// TODO: remove that if empty later
typedef struct {
} GameUpdateOutput;

GameUpdateInput game_input;
GameUpdateOutput game_output;
GameUpdateInput *game_input_ptr = &game_input;
GameUpdateOutput *game_output_ptr = &game_output;

extern const unsigned char *binary_assets_start;
extern const unsigned char *binary_assets_end;

SlpHeader *slp;
PalPalette *palette;
u32 music_wav_id, bird_wav_id;

void game_init(void) {
  Arena *arena = &blue_arena;

  PackHeader *pack_header = pack_header_parse(arena, (u8 *) binary_assets_start, binary_assets_end - binary_assets_start);

  usize interface_drs_len;
  u8 *interface_drs = pack_file_get(pack_header, string8_static("game/data/interfac.drs"), &interface_drs_len);
  DrsHeader *interface_drs_header = drs_header_parse(arena, interface_drs, interface_drs_len);

  usize graphics_drs_len;
  u8 *graphics_drs = pack_file_get(pack_header, string8_static("game/data/graphics.drs"), &graphics_drs_len);
  DrsHeader *graphics_drs_header = drs_header_parse(arena, graphics_drs, graphics_drs_len);

  usize sounds_drs_len;
  u8 *sounds_drs = pack_file_get(pack_header, string8_static("game/data/sounds.drs"), &sounds_drs_len);
  DrsHeader *sounds_drs_header = drs_header_parse(arena, sounds_drs, sounds_drs_len);

  usize wonder_slp_file_len;
  u8 *wonder_slp_file = drs_file_get(graphics_drs_header, string8_static("210.slp"), &wonder_slp_file_len);
  usize palette_file_len;
  u8 *palette_file = drs_file_get(interface_drs_header, string8_static("50500.bina"), &palette_file_len);
  palette = pal_parse(arena, palette_file, palette_file_len);
  slp = slp_parse_header(arena, wonder_slp_file, wonder_slp_file_len);

  // usize bird_wav_file_len;
  // u8 *bird_wav_file = drs_file_get(sounds_drs_header, string8_static("5201.wav"), &bird_wav_file_len);
  usize bird_wav_file_len;
  u8 *bird_wav_file = pack_file_get(pack_header, string8_static("game/sound/bird.wav"), &bird_wav_file_len);
  bird_wav_id = al_wav_create(bird_wav_file, bird_wav_file_len);

  usize music_wav_file_len;
  u8 *music_wav_file = pack_file_get(pack_header, string8_static("game/sound/music1.mid"), &music_wav_file_len);
  music_wav_id = al_wav_create(music_wav_file, music_wav_file_len);

  gl_set_resolution(640, 480);
}

usize i = 0;
void game_update(void) {
  ArenaTemp temp = arena_temp_get(&blue_arena);

  if (i == 50) {
    al_wav_play(bird_wav_id);
  }

  // usize frame_index = 0;
  usize frame_index = 2*13 + (i/8) % 13;
  Image *frame = slp_shape_render(temp.arena, slp, palette, frame_index, (SlpOptions) { .player_id = 1, .flipped = true });
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

  arena_temp_release(temp);
  gl_texture_destroy(texture_id);
  i += 1;
}
