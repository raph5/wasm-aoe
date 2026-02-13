/* Copyright (c) 2025, Raphaël Guyader
 * All rights reserved.
 *
 * This source code is licensed under the GPL-3.0 license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef H_ASSETS_SLP
#define H_ASSETS_SLP

#include "src/base.h"
#include "src/assets/pal.h"

typedef struct {
  u16 left_space;
  u16 right_space;
} SlpRowOutline;

typedef struct {
  i32 width;
  i32 height;
  i32 center_x;
  i32 center_y;
  u32 properties;  // see: docs/slp_format.txt line 46
  u8 **cmd_table;
  SlpRowOutline *outline_table;
} SlpShape;

typedef struct {
  u32 shape_count;
  SlpShape shape_table[];
} SlpHeader;

typedef struct {
  u8 player_id;
  b8 flipped;
} SlpOptions;

SlpHeader *slp_parse_header(Arena *arena, u8 *bin, usize bin_len);
Image *slp_shape_render(Arena *arena, SlpHeader *header, PalPalette *palette, u32 shape_index, SlpOptions options);

#endif
