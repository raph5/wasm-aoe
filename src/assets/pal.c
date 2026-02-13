/* Copyright (c) 2025, Raphaël Guyader
 * All rights reserved.
 *
 * This source code is licensed under the GPL-3.0 license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "src/assets/pal.h"

PalPalette *pal_parse(Arena *arena, u8 *bin, usize bin_len) {
  PalPalette *pal = arena_push(arena, sizeof(PalPalette));

  bin_parse_fmt(&bin, &bin_len, "JASC-PAL\r\n0100\r\n256\r\n");
  for (usize i = 0; i < 256; ++i) {
    bin_parse_fmt(&bin, &bin_len, "%u8 %u8 %u8\r\n", pal->r + i, pal->g + i, pal->b + i);
  }
  return pal;
}
