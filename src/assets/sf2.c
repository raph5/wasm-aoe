/* Copyright (c) 2025, Raphaël Guyader
 * All rights reserved.
 *
 * This source code is licensed under the GPL-3.0 license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "src/assets/sf2.h"

typedef enum {
  // INFO sub chunks
  SF2_SUB_CHUNK_IFIL,  // The version of the Sound Font RIFF file
  SF2_SUB_CHUNK_ISNG,  // The target Sound Engine
  SF2_SUB_CHUNK_INAM,  // The Sound Font Bank Name
  SF2_SUB_CHUNK_IROM,  // The Sound ROM Name
  SF2_SUB_CHUNK_IVER,  // The Sound ROM Version
  SF2_SUB_CHUNK_ICRD,  // The Date of Creation of the Bank
  SF2_SUB_CHUNK_IENG,  // Sound Designers and Engineers for the Bank
  SF2_SUB_CHUNK_IPRG,  // Product for which the Bank was intended
  SF2_SUB_CHUNK_ICOP,  // Copyright message
  SF2_SUB_CHUNK_ICMT,  // Comments on the Bank
  SF2_SUB_CHUNK_ISFT,  // The SoundFont tools used to create and alter the bank

  // sdta sub chunks
  SF2_SUB_CHUNK_SMPL,  // Digital Audio Samples

  // pdta sub chunks
  SF2_SUB_CHUNK_PHDR,  // The Preset Headers
  SF2_SUB_CHUNK_PBAG,  // The Preset Index list
  SF2_SUB_CHUNK_PMOD,  // The Preset Modulator list
  SF2_SUB_CHUNK_PGEN,  // The Preset Generator list
  SF2_SUB_CHUNK_INST,  // The Instrument Names and Indices
  SF2_SUB_CHUNK_IBAG,  // The Instrument Index list
  SF2_SUB_CHUNK_IMOD,  // The Instrument Modulator list
  SF2_SUB_CHUNK_IGEN,  // The Instrument Generator list
  SF2_SUB_CHUNK_SHDR,  // The Sample Headers

  SF2_SUB_CHUNK_UNKNOWN,
} Sf2SubChunk;

Sf2SubChunk sf2_parse_sub_chunk(u8 **bin, usize *bin_len, u8 **chunk_bin, usize *chunk_bin_len) {
  u8 sub_chunk[4];
  bin_read_bytes(sub_chunk, 4, bin, bin_len);
  Sf2SubChunk chunk_type;
  if (mem_eq(sub_chunk, "ifil", 4)) chunk_type = SF2_SUB_CHUNK_IFIL;
  else if (mem_eq(sub_chunk, "isng", 4)) chunk_type = SF2_SUB_CHUNK_ISNG;
  else if (mem_eq(sub_chunk, "INAM", 4)) chunk_type = SF2_SUB_CHUNK_INAM;
  else if (mem_eq(sub_chunk, "irom", 4)) chunk_type = SF2_SUB_CHUNK_IROM;
  else if (mem_eq(sub_chunk, "iver", 4)) chunk_type = SF2_SUB_CHUNK_IVER;
  else if (mem_eq(sub_chunk, "ICRD", 4)) chunk_type = SF2_SUB_CHUNK_ICRD;
  else if (mem_eq(sub_chunk, "IENG", 4)) chunk_type = SF2_SUB_CHUNK_IENG;
  else if (mem_eq(sub_chunk, "IPRG", 4)) chunk_type = SF2_SUB_CHUNK_IPRG;
  else if (mem_eq(sub_chunk, "ICOP", 4)) chunk_type = SF2_SUB_CHUNK_ICOP;
  else if (mem_eq(sub_chunk, "ICMT", 4)) chunk_type = SF2_SUB_CHUNK_ICMT;
  else if (mem_eq(sub_chunk, "ISFT", 4)) chunk_type = SF2_SUB_CHUNK_ISFT;
  else if (mem_eq(sub_chunk, "smpl", 4)) chunk_type = SF2_SUB_CHUNK_SMPL;
  else if (mem_eq(sub_chunk, "phdr", 4)) chunk_type = SF2_SUB_CHUNK_PHDR;
  else if (mem_eq(sub_chunk, "pbag", 4)) chunk_type = SF2_SUB_CHUNK_PBAG;
  else if (mem_eq(sub_chunk, "pmod", 4)) chunk_type = SF2_SUB_CHUNK_PMOD;
  else if (mem_eq(sub_chunk, "pgen", 4)) chunk_type = SF2_SUB_CHUNK_PGEN;
  else if (mem_eq(sub_chunk, "inst", 4)) chunk_type = SF2_SUB_CHUNK_INST;
  else if (mem_eq(sub_chunk, "ibag", 4)) chunk_type = SF2_SUB_CHUNK_IBAG;
  else if (mem_eq(sub_chunk, "imod", 4)) chunk_type = SF2_SUB_CHUNK_IMOD;
  else if (mem_eq(sub_chunk, "igen", 4)) chunk_type = SF2_SUB_CHUNK_IGEN;
  else if (mem_eq(sub_chunk, "shdr", 4)) chunk_type = SF2_SUB_CHUNK_SHDR;
  else return SF2_SUB_CHUNK_UNKNOWN;
  *chunk_bin_len = bin_read_little_u32(bin, bin_len);
  *chunk_bin = *bin;
  bin_skip_bytes(*chunk_bin_len, bin, bin_len);
  return chunk_type;
}

void sf2_parse_info_chunk(u8 **bin, usize *bin_len) {
  bin_parse_fmt(bin, bin_len, "LIST");
  u32 info_size = bin_read_little_u32(bin, bin_len);
  bin_parse_fmt(bin, bin_len, "INFO");
  u8 *info_bin = *bin;
  usize info_bin_len = info_size - 4;
  bin_skip_bytes(info_size - 4, bin, bin_len);

  while (info_bin_len > 0) {
    u8 *chunk_bin;
    usize chunk_bin_len;
    Sf2SubChunk chunk_type = sf2_parse_sub_chunk(&info_bin, &info_bin_len, &chunk_bin, &chunk_bin_len);
    switch (chunk_type) {
      case SF2_SUB_CHUNK_IFIL:  // the version of the Sound Font RIFF file
        assert(chunk_bin_len == 4);
        u16 info_version_major = bin_read_little_u16(&chunk_bin, &chunk_bin_len);
        u16 info_version_minor = bin_read_little_u16(&chunk_bin, &chunk_bin_len);
        assert(info_version_major == 2 && info_version_minor == 1);
        break;
      case SF2_SUB_CHUNK_INAM:  // the Sound Font Bank Name
        break;
      case SF2_SUB_CHUNK_ISNG:  // the target Sound Engine
        break;
      case SF2_SUB_CHUNK_IROM:  // the Sound ROM Name
        break;
      case SF2_SUB_CHUNK_IVER:  // the Sound ROM Version
        break;
      case SF2_SUB_CHUNK_ICRD:  // the Date of Creation of the Bank
        break;
      case SF2_SUB_CHUNK_IENG:  // Sound Designers and Engineers for the Bank
        break;
      case SF2_SUB_CHUNK_IPRG:  // Product for which the Bank was intended
        break;
      case SF2_SUB_CHUNK_ICOP:  // Copyright message
        break;
      case SF2_SUB_CHUNK_ICMT:  // Comments on the Bank
        break;
      case SF2_SUB_CHUNK_ISFT:  // The SoundFont tools used to create and alter the bank
        break;
      case SF2_SUB_CHUNK_UNKNOWN:
      default:
        panic("sf2 parser: unknown INFO sub chunk");
    }
  }
}

void sf2_parse_sdta_chunk(u8 **bin, usize *bin_len, Sf2Header *header) {
  bin_parse_fmt(bin, bin_len, "LIST");
  u32 sdta_size = bin_read_little_u32(bin, bin_len);
  bin_parse_fmt(bin, bin_len, "sdta");
  u8 *sdta_bin = *bin;
  usize sdta_bin_len = sdta_size - 4;
  bin_skip_bytes(sdta_size - 4, bin, bin_len);

  Sf2SubChunk chunk_type = sf2_parse_sub_chunk(&sdta_bin, &sdta_bin_len, &header->smpl_buf, &header->smpl_buf_len);
  assert(chunk_type == SF2_SUB_CHUNK_SMPL);
  assert(sdta_bin_len == 0);
}

void sf2_parse_shdr_chunk(Arena *arena, u8 *bin, usize bin_len, Sf2Header *header) {
  assert(bin_len % 46 == 0);
  header->sample_count = bin_len / 46 - 1;
  header->sample = arena_push(arena, header->sample_count * sizeof(Sf2Sample));
  for (usize i = 0; i < header->sample_count; ++i) {
    bin_skip_bytes(20, &bin, &bin_len);  // name
    u32 start = bin_read_little_u32(&bin, &bin_len);
    u32 end = bin_read_little_u32(&bin, &bin_len);
    u32 loop_start = bin_read_little_u32(&bin, &bin_len);
    u32 loop_end = bin_read_little_u32(&bin, &bin_len);
    u32 sample_rate = bin_read_little_u32(&bin, &bin_len);
    i8 original_pitch = bin_read_little_u8(&bin, &bin_len);
    u8 pitch_correction = bin_read_little_u8(&bin, &bin_len);
    bin_skip_bytes(2, &bin, &bin_len);  // sample_link
    u16 sample_type = bin_read_little_u16(&bin, &bin_len);
    if (sample_type != 1) {
      panic("sf2_parse_header: stereo sound fonts are not supported");
    }
    assert(start <= loop_start && loop_start <= loop_end && loop_end <= end);
    assert(end < header->smpl_buf_len);
    header->sample[i] = (Sf2Sample) {
      .start = start,
      .end = end,
      .loop_start = loop_start,
      .loop_end = loop_end,
      .sample_rate = sample_rate,
      .original_pitch = original_pitch,
      .pitch_correction = pitch_correction,
    };
  }
}

void sf2_parse_pdat_chunk(Arena *arena, u8 **bin, usize *bin_len, Sf2Header *header) {
  bin_parse_fmt(bin, bin_len, "LIST");
  u32 pdta_size = bin_read_little_u32(bin, bin_len);
  bin_parse_fmt(bin, bin_len, "pdta");
  u8 *pdta_bin = *bin;
  usize pdta_bin_len = pdta_size - 4;
  bin_skip_bytes(pdta_size - 4, bin, bin_len);

  while (pdta_bin_len > 0) {
    u8 *chunk_bin;
    usize chunk_bin_len;
    Sf2SubChunk chunk_type = sf2_parse_sub_chunk(&pdta_bin, &pdta_bin_len, &chunk_bin, &chunk_bin_len);
    switch (chunk_type) {
      case SF2_SUB_CHUNK_PHDR:  // The Preset Headers
        assert(chunk_bin_len % 38 == 0);
        header->phdr_buf = chunk_bin;
        header->phdr_buf_len = chunk_bin_len;
        break;
      case SF2_SUB_CHUNK_PBAG:  // The Preset Index list
        assert(chunk_bin_len % 4 == 0);
        header->pbag_buf = chunk_bin;
        header->pbag_buf_len = chunk_bin_len;
        break;
      case SF2_SUB_CHUNK_PMOD:  // The Preset Modulator list
        assert(chunk_bin_len % 10 == 0);
        header->pmod_buf = chunk_bin;
        header->pmod_buf_len = chunk_bin_len;
        break;
      case SF2_SUB_CHUNK_PGEN:  // The Preset Generator list
        assert(chunk_bin_len % 4 == 0);
        header->pgen_buf = chunk_bin;
        header->pgen_buf_len = chunk_bin_len;
        break;
      case SF2_SUB_CHUNK_INST:  // The Instrument Names and Indices
        assert(chunk_bin_len % 22 == 0);
        header->inst_buf = chunk_bin;
        header->inst_buf_len = chunk_bin_len;
        break;
      case SF2_SUB_CHUNK_IBAG:  // The Instrument Index list
        assert(chunk_bin_len % 4 == 0);
        header->ibag_buf = chunk_bin;
        header->ibag_buf_len = chunk_bin_len;
        break;
      case SF2_SUB_CHUNK_IMOD:  // The Instrument Modulator list
        assert(chunk_bin_len % 10 == 0);
        header->imod_buf = chunk_bin;
        header->imod_buf_len = chunk_bin_len;
        break;
      case SF2_SUB_CHUNK_IGEN:  // The Instrument Generator list
        assert(chunk_bin_len % 4 == 0);
        header->igen_buf = chunk_bin;
        header->igen_buf_len = chunk_bin_len;
        break;
      case SF2_SUB_CHUNK_SHDR:  // The Sample Headers
        assert(chunk_bin_len % 46 == 0);
        header->shdr_buf = chunk_bin;
        header->shdr_buf_len = chunk_bin_len;
        sf2_parse_shdr_chunk(arena, chunk_bin, chunk_bin_len, header);
        break;
      case SF2_SUB_CHUNK_UNKNOWN:
      default:
        panic("sf2 parser: unknown pdta sub chunk");
    }
  }

  if (header->pmod_buf_len > 10) {
    // pmod is empty in gm.sf2, let's not waist time supporting pmod
    panic("sf2 parser: the PMOD block is not supported in the provided soundfont, PMOD is not supported");
  }
  if (header->imod_buf_len > 10) {
    // imod is empty in gm.sf2, let's not waist time supporting imod
    panic("sf2 parser: the IMOD block is not supported in the provided soundfont, IMOD is not supported");
  }
}

Sf2Header *sf2_parse_header(Arena *arena, u8 *bin, usize bin_len) {
  Sf2Header *header = arena_push(arena, sizeof(Sf2Header));

  bin_parse_fmt(&bin, &bin_len, "RIFF");
  u32 sf2_size = bin_read_little_u32(&bin, &bin_len);
  assert(sf2_size == bin_len);
  bin_parse_fmt(&bin, &bin_len, "sfbk");

  // INFO Chunk
  sf2_parse_info_chunk(&bin, &bin_len);

  // sdta Chunk
  sf2_parse_sdta_chunk(&bin, &bin_len, header);

  // pdat Chunk
  sf2_parse_pdat_chunk(arena, &bin, &bin_len, header);

  return header;
}

// Wave sf2_decode_sample(Arena *arena, Sf2Header *soundfont, u32 sample_index) {
//   assert(sample_index < soundfont->sample_count);
//   Sf2Sample *sample = soundfont->sample[sample_index];
//   Wave wave = wave_alloc(arena, sample->, header->sample_count);
//   for (usize i = 0; i < header->sample_count; ++i) {
//     // WAV data is little endian like wasm
//     i16 n = ((i16 *) header->data)[i];
//     wave.buf[i] = (f32) n / 32768.0f / 2;
//   }
//   return wave;
// }
