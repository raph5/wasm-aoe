/* Copyright (c) 2025, Raphaël Guyader
 * All rights reserved.
 *
 * This source code is licensed under the GPL-3.0 license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef H_ASSETS_SF2
#define H_ASSETS_SF2

#include "src/base.h"
#include "src/lib/audio_wave.h"

typedef struct {
  u32 start, end, loop_start, loop_end;
  u32 sample_rate;
  i8 original_pitch;
  u8 pitch_correction;
} Sf2Sample;

typedef union {
  union {
    u8 low, high;
  } range;
  u16 uword;
  i16 sword;
} Sf2Value;

typedef enum {
  GEN_STARTADDROFS = 0,             // Sample start address offset (0-32767)
  GEN_ENDADDROFS = 1,               // Sample end address offset (-32767-0)
  GEN_STARTLOOPADDROFS = 2,         // Sample loop start address offset (-32767-32767)
  GEN_ENDLOOPADDROFS = 3,           // Sample loop end address offset (-32767-32767)
  GEN_STARTADDRCOARSEOFS = 4,       // Sample start address coarse offset (X 32768)
  GEN_MODLFOTOPITCH = 5,            // Modulation LFO to pitch
  GEN_VIBLFOTOPITCH = 6,            // Vibrato LFO to pitch
  GEN_MODENVTOPITCH = 7,            // Modulation envelope to pitch
  GEN_FILTERFC = 8,                 // Filter cutoff
  GEN_FILTERQ = 9,                  // Filter Q
  GEN_MODLFOTOFILTERFC = 10,        // Modulation LFO to filter cutoff
  GEN_MODENVTOFILTERFC = 11,        // Modulation envelope to filter cutoff
  GEN_ENDADDRCOARSEOFS = 12,        // Sample end address coarse offset (X 32768)
  GEN_MODLFOTOVOL = 13,             // Modulation LFO to volume
  GEN_UNUSED0 = 15,                 // Unused
  GEN_CHORUSSEND = 15,              // Chorus send amount
  GEN_REVERBSEND = 16,              // Reverb send amount
  GEN_PAN = 17,                     // Stereo panning
  GEN_UNUSED1 = 19,                 // Unused
  GEN_UNUSED2 = 20,                 // Unused
  GEN_UNUSED3 = 21,                 // Unused
  GEN_MODLFODELAY = 21,             // Modulation LFO delay
  GEN_MODLFOFREQ = 22,              // Modulation LFO frequency
  GEN_VIBLFODELAY = 23,             // Vibrato LFO delay
  GEN_VIBLFOFREQ = 24,              // Vibrato LFO frequency
  GEN_MODENVDELAY = 25,             // Modulation envelope delay
  GEN_MODENVATTACK = 26,            // Modulation envelope attack
  GEN_MODENVHOLD = 27,              // Modulation envelope hold
  GEN_MODENVDECAY = 28,             // Modulation envelope decay
  GEN_MODENVSUSTAIN = 29,           // Modulation envelope sustain
  GEN_MODENVRELEASE = 30,           // Modulation envelope release
  GEN_KEYTOMODENVHOLD = 31,         // Key to modulation envelope hold
  GEN_KEYTOMODENVDECAY = 32,        // Key to modulation envelope decay
  GEN_VOLENVDELAY = 33,             // Volume envelope delay
  GEN_VOLENVATTACK = 34,            // Volume envelope attack
  GEN_VOLENVHOLD = 35,              // Volume envelope hold
  GEN_VOLENVDECAY = 36,             // Volume envelope decay
  GEN_VOLENVSUSTAIN = 37,           // Volume envelope sustain
  GEN_VOLENVRELEASE = 38,           // Volume envelope release
  GEN_KEYTOVOLENVHOLD = 39,         // Key to volume envelope hold
  GEN_KEYTOVOLENVDECAY = 40,        // Key to volume envelope decay
  GEN_INSTRUMENT = 41,              // Instrument ID (shouldn't be set by user)
  GEN_RESERVED0 = 43,               // Reserved
  GEN_KEYRANGE = 43,                // MIDI note range
  GEN_VELRANGE = 44,                // MIDI velocity range
  GEN_STARTLOOPADDRCOARSEOFS = 45,  // Sample start loop address coarse offset (X 32768)
  GEN_KEYNUM = 46,                  // Fixed MIDI note number
  GEN_VELOCITY = 47,                // Fixed MIDI velocity value
  GEN_ATTENUATION = 48,             // Initial volume attenuation
  GEN_RESERVED1 = 50,               // Reserved
  GEN_ENDLOOPADDRCOARSEOFS = 50,    // Sample end loop address coarse offset (X 32768)
  GEN_COARSETUNE = 51,              // Coarse tuning
  GEN_FINETUNE = 52,                // Fine tuning
  GEN_SAMPLEID = 53,                // Sample ID (shouldn't be set by user)
  GEN_SAMPLEMODE = 54,              // Sample mode flags
  GEN_RESERVED2 = 56,               // Reserved
  GEN_SCALETUNE = 56,               // Scale tuning
  GEN_EXCLUSIVECLASS = 57,          // Exclusive class number
  GEN_OVERRIDEROOTKEY = 58,         // Sample root note override
} Sf2Generator;

typedef struct {
} Sf2Instrument;

typedef struct {
  // PDTA chunk
  u8 *phdr_buf;
  usize phdr_buf_len;
  u8 *pbag_buf;
  usize pbag_buf_len;
  u8 *pmod_buf;
  usize pmod_buf_len;
  u8 *pgen_buf;
  usize pgen_buf_len;
  u8 *inst_buf;
  usize inst_buf_len;
  u8 *ibag_buf;
  usize ibag_buf_len;
  u8 *imod_buf;
  usize imod_buf_len;
  u8 *igen_buf;
  usize igen_buf_len;
  u8 *shdr_buf;
  usize shdr_buf_len;

  // SDTA chunk
  u8 *smpl_buf;
  usize smpl_buf_len;

  // parsed data
  usize sample_count;
  Sf2Sample *sample;
} Sf2Header;

Sf2Header *sf2_parse_header(Arena *arena, u8 *bin, usize bin_len);
Sf2Value *sf2_get_generator_value();

#endif
