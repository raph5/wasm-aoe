
#ifndef H_ASSETS_PAL
#define H_ASSETS_PAL

#include "src/base.h"

typedef struct {
  u8 r[256], g[256], b[256];
} PalPalette;

PalPalette *pal_parse(Arena *arena, u8 *bin, usize bin_len);

#endif
