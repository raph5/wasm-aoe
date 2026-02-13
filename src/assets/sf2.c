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

Sf2SubChunk sf2_parse_sub_chunk(u8 **bin, usize *bin_len, u8 **smpl_bin, usize *smpl_bin_len) {
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
  *smpl_bin_len = bin_read_little_u32(bin, bin_len);
  *smpl_bin = *bin;
  bin_skip_bytes(*smpl_bin_len, bin, bin_len);
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
    u8 *smpl_bin;
    usize smpl_bin_len;
    Sf2SubChunk chunk_type = sf2_parse_sub_chunk(&info_bin, &info_bin_len, &smpl_bin, &smpl_bin_len);
    switch (chunk_type) {
      case SF2_SUB_CHUNK_IFIL:  // the version of the Sound Font RIFF file
        assert(smpl_bin_len == 4);
        u16 info_version_major = bin_read_little_u16(&smpl_bin, &smpl_bin_len);
        u16 info_version_minor = bin_read_little_u16(&smpl_bin, &smpl_bin_len);
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

void sf2_parse_pdat_chunk(u8 **bin, usize *bin_len) {
  // TODO:
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
  sf2_parse_pdat_chunk(&bin, &bin_len);

  return header;
}
