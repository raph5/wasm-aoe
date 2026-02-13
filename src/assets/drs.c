/* Copyright (c) 2025, Raphaël Guyader
 * All rights reserved.
 *
 * This source code is licensed under the GPL-3.0 license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "src/assets/drs.h"

// NOTE: see docs/drs_format.html

// panics if the header isn't valid OR one of the files referenced in the header
// isn't in the provided buffer
DrsHeader *drs_header_parse(Arena *arena, u8 *bin, usize bin_len) {
  u8 *start_bin = bin;
  usize start_bin_len = bin_len;

  u8 version[4], type[12];
  bin_skip_bytes(40, &bin, &bin_len);
  bin_read_bytes(version, 4, &bin, &bin_len);
  bin_read_bytes(type, 12, &bin, &bin_len);
  u32 table_count = bin_read_little_u32(&bin, &bin_len);
  bin_skip_bytes(4, &bin, &bin_len);
  assert(mem_eq(version, "1.00", 4));
  assert(mem_eq(type, "tribe\0\0\0\0\0\0\0", 12));

  u8 *table_info_bin = bin;
  usize table_info_bin_len = bin_len;
  u32 total_file_count = 0;
  for (usize i = 0; i < table_count; ++i) {
    bin_skip_bytes(8, &table_info_bin, &table_info_bin_len);
    total_file_count += bin_read_little_u32(&table_info_bin, &table_info_bin_len);
  }

  usize header_size = sizeof(DrsHeader) + total_file_count * sizeof(DrsHeaderFile);
  DrsHeader *header = arena_push(arena, header_size);
  header->file_count = total_file_count;

  usize total_file_index = 0;
  for (usize i = 0; i < table_count; ++i) {
    u8 format[4];
    bin_read_bytes(format, 4, &bin, &bin_len);
    u32 table_offset = bin_read_little_u32(&bin, &bin_len);
    u32 file_count = bin_read_little_u32(&bin, &bin_len);

    u8 format_reversed[4] = { format[3], format[2], format[1], format[0] };
    String8 format_str = string8_buf(format_reversed);
    assert(format_str.buf[0] != ' ');
    if      (format_str.buf[1] == ' ') format_str.len = 1;
    else if (format_str.buf[2] == ' ') format_str.len = 2;
    else if (format_str.buf[3] == ' ') format_str.len = 3;

    u8 *table_bin = start_bin + table_offset;
    usize table_bin_len = start_bin_len - table_offset;
    assert(table_bin >= bin);
    for (usize j = 0; j < file_count; ++j) {
      u32 file_id = bin_read_little_u32(&table_bin, &table_bin_len);
      u32 file_offset = bin_read_little_u32(&table_bin, &table_bin_len);
      u32 file_size = bin_read_little_u32(&table_bin, &table_bin_len);

      assert(file_offset + file_size <= start_bin_len);

      String8 name = fmt_alloc(arena, "%u32.%s", file_id, format_str);
      header->file_tab[total_file_index].buf = start_bin + file_offset;
      header->file_tab[total_file_index].buf_len = file_size;
      header->file_tab[total_file_index].name = name;
      total_file_index += 1;
    }
  }

  return header;
}

// panics if there is no file named `name` in `header`
u8 *drs_file_get(DrsHeader *header, String8 name, usize *file_len) {
  for (usize i = 0; i < header->file_count; ++i) {
    if (string8_eq(header->file_tab[i].name, name)) {
      *file_len = header->file_tab[i].buf_len;
      return header->file_tab[i].buf;
    }
  }
  panic("dsr_file_get: file not found");
}
