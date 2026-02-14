/* Copyright (c) 2025, Raphaël Guyader
 * All rights reserved.
 *
 * This source code is licensed under the GPL-3.0 license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "src/assets/wav.h"

WavHeader *wav_parse_header(Arena *arena, u8 *bin, usize bin_len) {
  WavHeader *header = arena_push(arena, sizeof(WavHeader));

  bin_parse_fmt(&bin, &bin_len, "RIFF");
  bin_skip_bytes(4, &bin, &bin_len);
  bin_parse_fmt(&bin, &bin_len, "WAVE");

  u8 *format_bin = bin;
  bin_parse_fmt(&bin, &bin_len, "fmt ");
  u32 format_size = bin_read_little_u32(&bin, &bin_len);
  u16 format_audio_format = bin_read_little_u16(&bin, &bin_len);
  u16 format_channel_count = bin_read_little_u16(&bin, &bin_len);
  u32 format_sample_rate = bin_read_little_u32(&bin, &bin_len);
  u32 format_byte_rate = bin_read_little_u32(&bin, &bin_len);
  u16 format_byte_per_block = bin_read_little_u16(&bin, &bin_len);
  u16 format_bits_per_sample = bin_read_little_u16(&bin, &bin_len);
  assert(bin - format_bin - 8 <= (i32) format_size);
  bin_skip_bytes(format_size - (bin - format_bin - 8), &bin, &bin_len);

  if (format_audio_format != 1) {
    panic("wav_parse_header: format %u16 not supported", format_audio_format);
  }
  if (format_channel_count < 1 || format_channel_count > 2) {
    panic("wav_parse_header: more that two channels not supported", format_channel_count);
  }
  if (format_sample_rate > AL_RATE) {
    panic("wav_parse_header: sample rate higher than AL_RATE are not supported", format_sample_rate);
  }
  if (format_bits_per_sample != 8 && format_bits_per_sample != 16) {
    panic("wav_parse_header: bits_per_sample %u16 not supported", format_bits_per_sample);
  }
  assert(format_byte_rate == format_sample_rate * format_channel_count * format_bits_per_sample/8);
  assert(format_byte_per_block == format_channel_count * format_bits_per_sample/8);
  header->sample_rate = format_sample_rate;
  header->channel_count = format_channel_count;
  header->bits_per_sample = format_bits_per_sample;

  bin_parse_fmt(&bin, &bin_len, "data");
  u32 data_size = bin_read_little_u32(&bin, &bin_len);
  u8 *data = bin;

  assert(bin_len >= data_size);
  assert(data_size % format_byte_per_block == 0);
  header->sample_count = data_size / format_byte_per_block;
  header->data = data;

  return header;
}

Sample wav_decode_mono_16_bits(Arena *arena, WavHeader *header) {
  Sample sample = sample_alloc(arena, header->sample_rate, header->sample_count, header->channel_count);
  for (usize i = 0; i < header->sample_count; ++i) {
    // WAV data is little endian like wasm
    i16 n = ((i16 *) header->data)[i];
    sample.buf[i] = (f32) n / 32768.0f / 2;
  }
  return sample;
}

Sample wav_decode(Arena *arena, WavHeader *header) {
  if (header->channel_count == 1 && header->bits_per_sample == 16) {
    return wav_decode_mono_16_bits(arena, header);
  } else {
    // TODO: implement 8 bits wav decoder
    panic("wav_decode: not implemented");
  }
}
