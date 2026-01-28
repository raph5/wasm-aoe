#include "src/assets/slp.h"
#include "src/base.h"

SlpRowOutline *slp_parse_outline_table(Arena *arena, u8 *bin, usize bin_len, i32 height) {
  SlpRowOutline *outline_table = arena_push(arena, height * sizeof(SlpRowOutline));

  for (usize i = 0; i < (u32) height; ++i) {
    outline_table[i].left_space = bin_read_little_u16(&bin, &bin_len);
    outline_table[i].right_space = bin_read_little_u16(&bin, &bin_len);
  }

  return outline_table;
}

u8 **slp_parse_cmd_table(Arena *arena, u8 *bin, usize bin_len, i32 height, u8 *start_bin) {
  u8 **cmd_table = arena_push(arena, height * sizeof(u8 *));

  for (usize i = 0; i < (u32) height; ++i) {
    cmd_table[i] = start_bin + bin_read_little_u32(&bin, &bin_len);
  }

  return cmd_table;
}

SlpHeader *slp_parse_header(Arena *arena, u8 *bin, usize bin_len) {
  u8 *start_bin = bin;
  usize start_bin_len = bin_len;

  u8 version[4];
  bin_read_bytes(version, 4, &bin, &bin_len);
  u32 shape_count = bin_read_little_u32(&bin, &bin_len);
  bin_skip_bytes(24, &bin, &bin_len);
  assert(mem_eq(version, "2.0N", 4));

  usize header_size = sizeof(SlpHeader) + shape_count * sizeof(SlpShape);
  SlpHeader *header = arena_push(arena, header_size);
  header->shape_count = shape_count;

  for (usize i = 0; i < shape_count; ++i) {
    u32 cmd_table_offset = bin_read_little_u32(&bin, &bin_len);
    u32 outline_table_offset = bin_read_little_u32(&bin, &bin_len);
    bin_skip_bytes(4, &bin, &bin_len);
    u32 properties = bin_read_little_u32(&bin, &bin_len);
    i32 width = bin_read_little_i32(&bin, &bin_len);
    i32 height = bin_read_little_i32(&bin, &bin_len);
    i32 center_x = bin_read_little_i32(&bin, &bin_len);
    i32 center_y = bin_read_little_i32(&bin, &bin_len);
    assert(outline_table_offset < start_bin_len);
    assert(cmd_table_offset < start_bin_len);

    u8 *outline_table_bin = start_bin + outline_table_offset;
    usize outline_table_bin_len = start_bin_len - outline_table_offset;
    SlpRowOutline *outline_table = slp_parse_outline_table(arena, outline_table_bin, outline_table_bin_len, height);

    u8 *cmd_table_bin = start_bin + cmd_table_offset;
    usize cmd_table_bin_len = start_bin_len - cmd_table_offset;
    u8 **cmd_table = slp_parse_cmd_table(arena, cmd_table_bin, cmd_table_bin_len, height, start_bin);

    header->shape_table[i] = (SlpShape) {
      .width = width,
      .height = height,
      .center_x = center_x,
      .center_y = center_y,
      .properties = properties,
      .cmd_table = cmd_table,
      .outline_table = outline_table,
    };
  }

  return header;
}

typedef struct {
  Image *image;
  usize index;
  usize size;
  b8 flipped;
} SlpStream;

void slp_stream_put(SlpStream *stream, PalPalette *palette, u8 color) {
  assert(stream->index + 1 <= stream->size);
  i32 width = stream->image->width;
  if (stream->flipped) {
    usize index = stream->index / width * width + width - (stream->index % width);
    stream->image->buf[4*index + 0] = palette->r[color];
    stream->image->buf[4*index + 1] = palette->g[color];
    stream->image->buf[4*index + 2] = palette->b[color];
    stream->image->buf[4*index + 3] = 255;
    stream->index += 1;
  } else {
    usize index = stream->index;
    stream->image->buf[4*index + 0] = palette->r[color];
    stream->image->buf[4*index + 1] = palette->g[color];
    stream->image->buf[4*index + 2] = palette->b[color];
    stream->image->buf[4*index + 3] = 255;
    stream->index += 1;
  }
}

void slp_stream_write(SlpStream *stream, PalPalette *palette, u8 *color_array, usize color_array_len) {
  assert(stream->index + 1 <= stream->size);
  i32 width = stream->image->width;
  if (stream->flipped) {
    for (usize i = 0; i < color_array_len; ++i) {
      usize index = stream->index / width * width + width - (stream->index % width);
      stream->image->buf[4*index + 0] = palette->r[color_array[i]];
      stream->image->buf[4*index + 1] = palette->g[color_array[i]];
      stream->image->buf[4*index + 2] = palette->b[color_array[i]];
      stream->image->buf[4*index + 3] = 255;
      stream->index += 1;
    }
  } else {
    for (usize i = 0; i < color_array_len; ++i) {
      usize index = stream->index;
      stream->image->buf[4*index + 0] = palette->r[color_array[i]];
      stream->image->buf[4*index + 1] = palette->g[color_array[i]];
      stream->image->buf[4*index + 2] = palette->b[color_array[i]];
      stream->image->buf[4*index + 3] = 255;
      stream->index += 1;
    }
  }
}

void slp_stream_fill(SlpStream *stream, PalPalette *palette, u8 color, usize len) {
  assert(stream->index + 1 <= stream->size);
  i32 width = stream->image->width;
  if (stream->flipped) {
    for (usize i = 0; i < len; ++i) {
      usize index = stream->index / width * width + width - (stream->index % width);
      stream->image->buf[4*index + 0] = palette->r[color];
      stream->image->buf[4*index + 1] = palette->g[color];
      stream->image->buf[4*index + 2] = palette->b[color];
      stream->image->buf[4*index + 3] = 255;
      stream->index += 1;
    }
  } else {
    for (usize i = 0; i < len; ++i) {
      usize index = stream->index;
      stream->image->buf[4*index + 0] = palette->r[color];
      stream->image->buf[4*index + 1] = palette->g[color];
      stream->image->buf[4*index + 2] = palette->b[color];
      stream->image->buf[4*index + 3] = 255;
      stream->index += 1;
    }
  }
}

void slp_stream_skip(SlpStream *stream, usize len) {
  assert(stream->index + len <= stream->size);
  stream->index += len;
}

void slp_shape_render_row(SlpStream *stream, SlpShape *shape, PalPalette *palette, SlpOptions options, i32 row) {
  SlpRowOutline row_outline = shape->outline_table[row];
  if (row_outline.left_space == 0x8000) {
    slp_stream_skip(stream, shape->width);
    return;
  }
  slp_stream_skip(stream, row_outline.left_space);

  u8 *cmd_bin = shape->cmd_table[row];
  i32 cmd_byte, next_byte, color_byte, pixel_count;
  b32 skip_next_cmd = false;
  while (1) {
    cmd_byte = *(cmd_bin++);

    switch (cmd_byte & 0x0f) {
      // lesser draw
      case 0x00:
      case 0x04:
      case 0x08:
      case 0x0c:
        pixel_count = cmd_byte >> 2;
        if (skip_next_cmd) {
          skip_next_cmd = false;
          cmd_bin += pixel_count;
        } else {
          slp_stream_write(stream, palette, cmd_bin, pixel_count);
          cmd_bin += pixel_count;
        }
        break;

      // lesser skip
      case 0x01:
      case 0x05:
      case 0x09:
      case 0x0d:
        pixel_count = cmd_byte >> 2;
        if (skip_next_cmd) {
          skip_next_cmd = false;
        } else {
          slp_stream_skip(stream, pixel_count);
        }
        break;

      // greater draw
      case 0x02:
        next_byte = *(cmd_bin++);
        pixel_count = ((cmd_byte & 0xf0) << 4) + next_byte;
        if (skip_next_cmd) {
          skip_next_cmd = false;
          cmd_bin += pixel_count;
        } else {
          slp_stream_write(stream, palette, cmd_bin, pixel_count);
          cmd_bin += pixel_count;
        }
        break;

      // greater skip
      case 0x03:
        next_byte = *(cmd_bin++);
        pixel_count = ((cmd_byte & 0xf0) << 4) + next_byte;
        if (skip_next_cmd) {
          skip_next_cmd = false;
        } else {
          slp_stream_skip(stream, pixel_count);
        }
        break;

      // player color draw
      case 0x06:
        pixel_count = cmd_byte >> 4;
        if (pixel_count == 0) {
          next_byte = *(cmd_bin++);
          pixel_count = next_byte;
        }
        if (skip_next_cmd) {
          skip_next_cmd = false;
          cmd_bin += pixel_count;
        } else {
          for (int i = 0; i < pixel_count; ++i) {
            color_byte = *(cmd_bin++);
            slp_stream_put(stream, palette, color_byte + 16 * options.player_id);
          }
        }
        break;

      // fill
      case 0x07:
        pixel_count = cmd_byte >> 4;
        if (pixel_count == 0) {
          next_byte = *(cmd_bin++);
          pixel_count = next_byte;
        }
        color_byte = *(cmd_bin++);
        if (skip_next_cmd) {
          skip_next_cmd = false;
        } else {
          slp_stream_fill(stream, palette, color_byte, pixel_count);
        }
        break;

      // player color fill
      // NOTE: The explanation here https://gist.github.com/phrohdoh/2bbfe1ef324db9d7f4d24d23c2568938#file-slp-txt-L101
      // is not the right one. I think cmd 0x0a is a player color fill like
      // stated here https://github.com/SFTtech/openage/blob/master/doc/media/slp-files.md
      case 0x0a:
        pixel_count = cmd_byte >> 4;
        if (pixel_count == 0) {
          next_byte = *(cmd_bin++);
          pixel_count = next_byte;
        }
        color_byte = *(cmd_bin++);
        if (skip_next_cmd) {
          skip_next_cmd = false;
        } else {
          for (int i = 0; i < pixel_count; ++i) {
            slp_stream_put(stream, palette, color_byte + 16 * options.player_id);
          }
        }
        break;
        

      // shadow
      // NOTE: I don't fully understand this command. For now I will interpret
      // it as fill of color pink for debugging
      // the shadow tables mentioned at https://gist.github.com/phrohdoh/2bbfe1ef324db9d7f4d24d23c2568938#file-slp-txt-L110
      // might be files 50900 to 50911 in Interfac.drs
      case 0x0b:
        pixel_count = cmd_byte >> 4;
        if (pixel_count == 0) {
          next_byte = *(cmd_bin++);
          pixel_count = next_byte;
        }
        if (skip_next_cmd) {
          skip_next_cmd = false;
        } else {
          slp_stream_fill(stream, palette, 12, pixel_count);
        }
        break;

      // extended commands
      case 0x0e:
        switch (cmd_byte & 0xf0) {
          // draw if not flipped
          case 0x00:
            if (options.flipped) {
              skip_next_cmd = true;
            }
            break;

          // draw if not flipped
          case 0x01:
            if (!options.flipped) {
              skip_next_cmd = true;
            }
            break;

          default:
            panic("slp_shape_render_row unknown command %u8", cmd_byte);
        }

      // end of line
      case 0x0f:
        slp_stream_skip(stream, row_outline.right_space);
        if (skip_next_cmd) {
          panic("slp_draw_row can skip eol at row %i32", row);
        }
        if (stream->index % stream->image->width != 0) {
          log_print("stream->index = %i32", stream->index);
          log_print("shape->width = %i32", shape->width);
          panic("slp_draw_row misalignment at row %i32", row);
        }
        return;

      default:
        panic("slp_shape_render_row unknown command %u8", cmd_byte);
    }
  }

  panic("unreachable");
}

Image *slp_shape_render(Arena *arena, SlpHeader *header, PalPalette *palette, u32 shape_index, SlpOptions options) {
  assert(shape_index < header->shape_count);
  SlpShape *shape = &header->shape_table[shape_index];

  usize image_size = sizeof(Image) + shape->width * shape->height * 4;
  Image *image = arena_push(arena, image_size);
  image->width = shape->width;
  image->height = shape->height;
  mem_zero(image->buf, shape->width * shape->height * 4);

  SlpStream stream = {
    .image = image,
    .flipped = options.flipped,
    .size = shape->width * shape->height,
    .index = 0,
  };

  for (usize row = 0; row < (usize) shape->height; ++row) {
    slp_shape_render_row(&stream, shape, palette, options, row);
  }

  assert(stream.index == stream.size);
  return image;
}
