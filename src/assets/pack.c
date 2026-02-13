/* Copyright (c) 2025, Raphaël Guyader
 * All rights reserved.
 *
 * This source code is licensed under the GPL-3.0 license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "src/assets/pack.h"

// panics if the header isn't valid OR one of the files referenced in the header
// isn't in the provided buffer
PackHeader *pack_header_parse(Arena *arena, u8 *bin, usize bin_len) {
  u8 *start_bin = bin;
  usize start_bin_len = bin_len;

  u8 magic[4];
  bin_read_bytes(magic, 4, &bin, &bin_len);
  u32 version = bin_read_big_u32(&bin, &bin_len);
  u32 file_count = bin_read_big_u32(&bin, &bin_len);
  assert(mem_eq(magic, "PACK", 4));
  assert(version == 1);

  usize header_size = sizeof(PackHeader) + file_count * sizeof(PackHeaderFile);
  PackHeader *header = arena_push(arena, header_size);
  header->file_count = file_count;

  for (usize i = 0; i < file_count; ++i) {
    u32 name_len = bin_read_big_u32(&bin, &bin_len);
    // NOTE: copying the string is kinda useless because we keep the PACK
    // file buffer in memory
    String8 name = string8_alloc(arena, name_len);
    bin_read_bytes(name.buf, name_len, &bin, &bin_len);
    u64 offset = bin_read_big_u64(&bin, &bin_len);
    u64 size = bin_read_big_u64(&bin, &bin_len);

    assert(offset + size <= start_bin_len);

    header->file_tab[i].buf = start_bin + offset;
    header->file_tab[i].buf_len = size;
    header->file_tab[i].name = name;
  }

  return header;
}

// panics if there is no file named `name` in `header`
u8 *pack_file_get(PackHeader *header, String8 name, usize *file_len) {
  for (usize i = 0; i < header->file_count; ++i) {
    if (string8_eq(header->file_tab[i].name, name)) {
      *file_len = header->file_tab[i].buf_len;
      return header->file_tab[i].buf;
    }
  }
  panic("pack_file_get: file not found");
}
