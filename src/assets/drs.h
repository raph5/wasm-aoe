
#ifndef H_ASSET_DRS
#define H_ASSET_DRS

#include "src/base.h"

// NOTE: see docs/drs_format.html

typedef struct {
  String8 name;
  usize buf_len;
  u8 *buf;
} DrsHeaderFile;

typedef struct {
  u32 file_count;
  DrsHeaderFile file_tab[];
} DrsHeader;

// panics if the header isn't valid OR one of the files referenced in the header
// isn't in the provided buffer
DrsHeader *drs_header_parse(Arena *arena, u8 *drs, usize drs_len);
// panics if there is no file named `name` in `header`
u8 *drs_file_get(DrsHeader *header, String8 name, usize *file_len);

#endif
