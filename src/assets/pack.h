
#ifndef H_ASSET_PACK
#define H_ASSET_PACK

#include "src/base.h"

typedef struct {
  String8 name;
  usize buf_len;
  u8 *buf;
} PackHeaderFile;

typedef struct {
  u32 file_count;
  PackHeaderFile file_tab[];
} PackHeader;

// panics if the header isn't valid OR one of the files referenced in the header
// isn't in the provided buffer
PackHeader *pack_header_parse(Arena *arena, u8 *bin, usize bin_len);
// panics if there is no file named `name` in `header`
u8 *pack_file_get(PackHeader *header, String8 name, usize *file_len);

#endif
