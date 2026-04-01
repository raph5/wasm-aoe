/* Copyright (c) 2025, Raphaël Guyader
 * All rights reserved.
 *
 * This source code is licensed under the GPL-3.0 license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "src/assets/midi.h"
#include "src/base.h"

// read variable length quantity
u32 bin_read_vlq(u8 **bin, usize *bin_len) {
  u32 n;
  assert(*bin_len >= 1);
  if (((*bin)[0] & 0x80) == 0) {
    n = **bin;
    *bin += 1;
    *bin_len -= 1;
    return n;
  }
  assert(*bin_len >= 2);
  if (((*bin)[1] & 0x80) == 0) {
    n = (((*bin)[0] & 0x7f) << 7) + (*bin)[1];
    *bin += 2;
    *bin_len -= 2;
    return n;
  }
  assert(*bin_len >= 3);
  if (((*bin)[2] & 0x80) == 0) {
    n = (((*bin)[0] & 0x7f) << 14) + (((*bin)[1] & 0x7f) << 7) + (*bin)[2];
    *bin += 3;
    *bin_len -= 3;
    return n;
  }
  assert(*bin_len >= 4);
  assert(((*bin)[3] & 0x80) == 0);
  n = (((*bin)[0] & 0x7f) << 21) + (((*bin)[1] & 0x7f) << 14) + (((*bin)[2] & 0x7f) << 7) + (*bin)[3];
  *bin += 4;
  *bin_len -= 4;
  return n;
}

void test_bin_read_vlq() {
  u8 buf[4], *buf_ptr;
  usize buf_len;
  mem_zero(buf, 4);
  mem_cpy(buf, "\x00", 1);
  buf_ptr = buf;
  buf_len = 1;
  assert(bin_read_vlq(&buf_ptr, &buf_len) == 00000000);
  mem_zero(buf, 4);
  mem_cpy(buf, "\x40", 1);
  buf_ptr = buf;
  buf_len = 1;
  assert(bin_read_vlq(&buf_ptr, &buf_len) == 0x00000040);
  mem_zero(buf, 4);
  mem_cpy(buf, "\x7F", 1);
  buf_ptr = buf;
  buf_len = 1;
  assert(bin_read_vlq(&buf_ptr, &buf_len) == 0x0000007F);
  mem_zero(buf, 4);
  mem_cpy(buf, "\x81\x00", 2);
  buf_ptr = buf;
  buf_len = 2;
  assert(bin_read_vlq(&buf_ptr, &buf_len) == 0x00000080);
  mem_zero(buf, 4);
  mem_cpy(buf, "\xC0\x00", 2);
  buf_ptr = buf;
  buf_len = 2;
  assert(bin_read_vlq(&buf_ptr, &buf_len) == 0x00002000);
  mem_zero(buf, 4);
  mem_cpy(buf, "\xFF\x7F", 2);
  buf_ptr = buf;
  buf_len = 2;
  assert(bin_read_vlq(&buf_ptr, &buf_len) == 0x00003FFF);
  mem_zero(buf, 4);
  mem_cpy(buf, "\x81\x80\x00", 3);
  buf_ptr = buf;
  buf_len = 3;
  assert(bin_read_vlq(&buf_ptr, &buf_len) == 0x00004000);
  mem_zero(buf, 4);
  mem_cpy(buf, "\xC0\x80\x00", 3);
  buf_ptr = buf;
  buf_len = 3;
  assert(bin_read_vlq(&buf_ptr, &buf_len) == 0x00100000);
  mem_zero(buf, 4);
  mem_cpy(buf, "\xFF\xFF\x7F", 3);
  buf_ptr = buf;
  buf_len = 3;
  assert(bin_read_vlq(&buf_ptr, &buf_len) == 0x001FFFFF);
  mem_zero(buf, 4);
  mem_cpy(buf, "\x81\x80\x80\x00", 4);
  buf_ptr = buf;
  buf_len = 4;
  assert(bin_read_vlq(&buf_ptr, &buf_len) == 0x00200000);
  mem_zero(buf, 4);
  mem_cpy(buf, "\xC0\x80\x80\x00", 4);
  buf_ptr = buf;
  buf_len = 4;
  assert(bin_read_vlq(&buf_ptr, &buf_len) == 0x08000000);
  mem_zero(buf, 4);
  mem_cpy(buf, "\xFF\xFF\xFF\x7F", 4);
  buf_ptr = buf;
  buf_len = 4;
  assert(bin_read_vlq(&buf_ptr, &buf_len) == 0x0FFFFFFF);
}

MidiHeader *midi_parse_header(Arena *arena, u8 *bin, usize bin_len) {
  bin_parse_fmt(&bin, &bin_len, "MThd");
  u32 size = bin_read_big_u32(&bin, &bin_len);
  assert(bin_len >= size);
  u16 format = bin_read_big_u16(&bin, &bin_len);
  u16 track_count = bin_read_big_u16(&bin, &bin_len);
  u16 division = bin_read_big_u16(&bin, &bin_len);
  assert(format == 0 || format == 1);
  assert((division & 0x8000) == 0);
  assert(track_count <= MIDI_TRACK_MAX);

  MidiHeader *header = arena_push(arena, sizeof(MidiHeader));
  header->tick_per_quater_note = division;
  header->track_count = track_count;

  for (usize i = 0; i < track_count; ++i) {
    bin_parse_fmt(&bin, &bin_len, "MTrk");
    u32 track_size = bin_read_big_u32(&bin, &bin_len);
    header->track[i].buf = bin;
    header->track[i].buf_len = track_size;
  }

  return header;
}

void midi_print_track(MidiHeader *header, usize track_index) {
  assert(track_index < header->track_count);
  u8 *bin = header->track[track_index].buf;
  usize bin_len = header->track[track_index].buf_len;

  u32 time = 0;
  u8 running_status = 0;
  while (bin_len > 0) {
    u32 delta_time = bin_read_vlq(&bin, &bin_len);
    time += delta_time;
    u8 byte = bin_read_byte(&bin, &bin_len);
    if (byte & 0x80) {
      if (byte == 0xf0 || byte == 0xf7) {
        u32 sysex_len = bin_read_vlq(&bin, &bin_len);
        bin_skip_bytes(sysex_len, &bin, &bin_len);
        log_warn("midi player: sysex events are not supported");
        continue;
      } else if (byte == 0xff) {
        u8 meta_type = bin_read_byte(&bin, &bin_len);
        u32 meta_len = bin_read_vlq(&bin, &bin_len);
        bin_skip_bytes(meta_len, &bin, &bin_len);
        log_print("%u32: meta event (length %u32, type %u8)", time, meta_len, meta_type);
        continue;
      } else {
        running_status = byte;
        byte = bin_read_byte(&bin, &bin_len);
      }
    }

    u8 channel = running_status & 0x0f;
    switch (running_status & 0xf0) {
      case 0x80: {
        u8 key = byte;
        u8 velocity = bin_read_byte(&bin, &bin_len);
        assert(!(velocity & 0x80));
        log_print("%u32: note off event (key %u8, velocity %u8)", time, key, velocity);
        break;
      }
      case 0x90: {
        u8 key = byte;
        u8 velocity = bin_read_byte(&bin, &bin_len);
        assert(!(velocity & 0x80));
        log_print("%u32: note on event (channel %u8, key %u8, velocity %u8)", channel, time, key, velocity);
        break;
      }
      case 0xa0: {
        u8 key = byte;
        u8 pressure = bin_read_byte(&bin, &bin_len);
        assert(!(pressure & 0x80));
        log_print("%u32: key pressure event (channel %u8, key %u8, pressure %u8)", channel, time, key, pressure);
        break;
      }
      case 0xb0: {
        u8 controller = byte;
        u8 value = bin_read_byte(&bin, &bin_len);
        assert(!(value & 0x80));
        log_print("%u32: control change event (channel %u8, controller %u8, value %u8)", channel, time, controller, value);
        break;
      }
      case 0xc0: {
        u8 program = byte;
        log_print("%u32: program change event (channel %u8, program %u8)", channel, time, program);
        break;
      }
      case 0xd0: {
        u8 pressure = byte;
        log_print("%u32: polyphonic pressure event (channel %u8, pressure %u8)", channel, time, pressure);
        break;
      }
      case 0xe0: {
        u8 least = byte;
        u8 most = bin_read_byte(&bin, &bin_len);
        assert(!(most & 0x80));
        log_print("%u32: pitch change event (channel %u8, pitch %u8)", channel, time, least + (most << 7));
        break;
      }
      default:
        panic("invalid running status %u8", running_status);
    }
  }
}

// Yields the first even that happens before the time indicated my `before`. 
// `before` is expressed in microseconds since the begining of the song. If
// no event can be yield, midi_player_yield_event returns false.
// `ended` is set to true if all events of the MidiPlayer have been yeilded,
// `ended` is set to false otherwise.
b32 midi_player_yield_event(MidiPlayer *player, MidiEvent *event, u64 before, b32 *ended) {
  u8 *track_buf = player->header->track[player->track_index].buf;
  usize track_buf_len = player->header->track[player->track_index].buf_len;
  if (player->buf_index >= track_buf_len) {
    *ended = true;
    return false;
  }
  u8 *bin = track_buf + player->buf_index;
  usize bin_len = track_buf_len - player->buf_index;

  u32 delta_tick = bin_read_vlq(&bin, &bin_len);
  u32 tick = player->time + delta_tick;
  u64 time = 0;
  if (tick != 0) {
    assert(player->microsec_per_quater_note != 0);
    time = (u64) tick * (u64) player->microsec_per_quater_note / player->header->tick_per_quater_note;
    if (time >= before) {
      *ended = false;
      return false;
    }
  } 

  u8 byte = bin_read_byte(&bin, &bin_len);
  if (byte & 0x80) {
    if (byte == 0xf0 || byte == 0xf7) {
      u32 sysex_len = bin_read_vlq(&bin, &bin_len);
      bin_skip_bytes(sysex_len, &bin, &bin_len);
      panic("midi player: sysex events are not supported");
    } else if (byte == 0xff) {
      u8 meta_type = bin_read_byte(&bin, &bin_len);
      u32 meta_len = bin_read_vlq(&bin, &bin_len);
      assert(meta_len < KiB(1));

      switch (meta_type) {
        case 0x51: {  // Set Tempo
          assert(meta_len == 3);
          u8 meta_bytes[3];
          bin_read_bytes(meta_bytes, meta_len, &bin, &bin_len);
          player->microsec_per_quater_note = meta_bytes[2] + (meta_bytes[1] << 8) + (meta_bytes[0] << 16);
          break;
        }

        case 0x2f: {  // End of track
          assert(meta_len == 0);
          *ended = true;
          return false;
        }

        case 0x03:  // Sequence/Track Name (ascii text)
        case 0x06:  // Marker (ascii text)
        case 0x54:  // SMPTE Offset (I don't realy know what that mean)
        case 0x58:  // Time Signature
        case 0x59:  // Key Signature
          bin_skip_bytes(meta_len, &bin, &bin_len);
          break;
        default:
          panic("midi player: unsupported meta event type %u8", meta_type);
      }

      return midi_player_yield_event(player, event, before, ended);
    } else {
      player->running_status = byte;
      byte = bin_read_byte(&bin, &bin_len);
    }
  }

  *ended = false;
  *event = (MidiEvent) {
    .time = time,
    .channel = player->running_status & 0x0f,
  };
  switch (player->running_status & 0xf0) {
    case 0x80: {  // Note Off event
      panic("midi player: note off event not supported");
      break;
    }
    case 0x90: {  // Note On Event
      u8 key = byte;
      u8 velocity = bin_read_byte(&bin, &bin_len);
      assert(!(key & 0x80) && !(velocity & 0x80));
      event->type = ME_NOTE_ON;
      event->data.note_on.key = key;
      event->data.note_on.velocity = velocity;
      break;
    }
    case 0xa0: {  // Polyphonic Key Pressure (Aftertouch)
      panic("midi player: polyphonic key pressure event not supported");
      break;
    }
    case 0xb0: {  // Control Change
      u8 controller = byte;
      u8 value = bin_read_byte(&bin, &bin_len);
      assert(!(controller & 0x80) && !(value & 0x80));
      event->type = ME_CONTROL_CHANGE;
      event->data.control_change.controller = controller;
      event->data.control_change.value = value;
      break;
    }
    case 0xc0: {  // Program Change
      u8 program = byte;
      assert(!(program & 0x80));
      event->type = ME_PROGRAM_CHANGE;
      event->data.program_change.program = program;
      break;
    }
    case 0xd0: {  // Channel Pressure (After-touch)
      panic("midi player: channel pressure event not supported");
    }
    case 0xe0: {  // Pitch Wheel Change
      u8 least = byte;
      u8 most = bin_read_byte(&bin, &bin_len);
      assert(!(least & 0x80) && !(most & 0x80));
      event->type = ME_PITCH_CHANGE;
      event->data.pitch_change.pitch = least + (most << 7);
      break;
    }
    default:
      panic("midi player: invalid running status %u8", player->running_status);
  }
  return true;
}
