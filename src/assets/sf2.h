/* Copyright (c) 2025, Raphaël Guyader
 * All rights reserved.
 *
 * This source code is licensed under the GPL-3.0 license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef H_ASSETS_SF2
#define H_ASSETS_SF2

#include "src/base.h"

typedef struct {
  u8 *smpl_buf;
  usize smpl_buf_len;
} Sf2Header;

Sf2Header *sf2_parse_header(Arena *arena, u8 *bin, usize bin_len);

#endif
