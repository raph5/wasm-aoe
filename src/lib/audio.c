/* Copyright (c) 2025, Raphaël Guyader
 * All rights reserved.
 *
 * This source code is licensed under the GPL-3.0 license found in the
 * LICENSE file in the root directory of this source tree.
 */

/*
 * NOTE: About audio resampling:
 * I tested two different approaches to for audio resampling: cubic
 * interpolation and sinc interpolation (also called bandlimited interpolation).
 * The sinc interpolation method was more complicated to use because it
 * required a padding of 13 passed and future samples.
 */

#include "src/lib/audio.h"
#include "src/assets/midi.h"
#include "src/base.h"

f32 *audio_yield_batch(Arena *arena, AudioPlayer *audio, u32 batch_len, b32 *ended) {
  u32 remaining_samples = audio->wave.sample_count - audio->index;
  if (remaining_samples >= batch_len) {
    f32 *out = audio->wave.buf + audio->index;
    audio->index += batch_len;
    *ended = audio->index >= audio->wave.sample_count;
    return out;
  } else {
    f32 *out = arena_push(arena, batch_len * sizeof(f32));
    mem_cpy(out, audio->wave.buf + audio->index, remaining_samples * sizeof(f32));
    mem_zero(out + remaining_samples, (batch_len - remaining_samples) * sizeof(f32));
    audio->index += remaining_samples;
    *ended = true;
    return out;
  }
}

void synth_oscillator_yeild_batch(Arena *arena, SynthOscillator *oscillator, u32 batch_len, b32 *ended);

f32 *synth_yield_batch(Arena *arena, Synth *synth, u32 batch_len, b32 *ended) {
  u64 batch_end = synth->time + AL_BATCH_SIZE * (TIME_SEC / AL_RATE);

  MidiEvent event;
  while (midi_player_yield_event(&synth->midi_player, &event, batch_end, ended)) {
    switch (event.type) {
      case ME_PROGRAM_CHANGE: {
      } break;

      case ME_CONTROL_CHANGE: {
      } break;

      case ME_NOTE_ON: {
      } break;

      case ME_PITCH_CHANGE: {
        // TODO:
      } break;
    }
  }
}

void music_player_init(MusicPlayer *player, Sf2Header *soundfont, MidiHeader *midi) {
  player->track_count = midi->track_count;
  for (usize i = 0; i < midi->track_count; ++i) {
    player->track[i] = (Synth) {
      .soundfont = soundfont,
      .midi_player = { .header = midi },
    };
  }
}

f32 *music_player_yield_batch(Arena *arena, MusicPlayer *player, u32 batch_len, b32 *ended) {
  // TODO: support multitract musics
  assert(player->track_count == 1);
  return synth_yield_batch(arena, &player->track[0], batch_len, ended);
}

void audio_context_play_audio(AudioContext *context, AudioPlayer audio, f32 gain) {
  assert(context->audio_count + 1 <= AC_AUDIO_CAP);
  assert(audio.wave.sample_rate == AL_RATE);
  context->audio[context->audio_count] = audio;
  context->audio_gain[context->audio_count] = gain;
  context->audio_count += 1;
}

void audio_context_remove_audio(AudioContext *context, usize index) {
  assert(index < context->audio_count);
  context->audio[index] = context->audio[context->audio_count-1];
  context->audio_gain[index] = context->audio_gain[context->audio_count-1];
  context->audio_count -= 1;
}

void audio_context_push_samples(Arena *arena, AudioContext *context) {
  ArenaTemp temp = arena_temp_get(arena);

  f32 *buf = arena_push(temp.arena, sizeof(f32) * AL_BATCH_SIZE);
  mem_zero(buf, sizeof(f32) * AL_BATCH_SIZE);

  // mixing
  for (usize i = 0; i < context->audio_count; ++i) {
    b32 audio_ended;
    f32 *audio_samples = audio_yield_batch(temp.arena, &context->audio[i], AL_BATCH_SIZE, &audio_ended);
    f32 audio_gain = context->audio_gain[i];

    for (usize j = 0; j < AL_BATCH_SIZE; ++j) {
      buf[j] += audio_samples[j] * audio_gain;
    }

    if (audio_ended) {
      audio_context_remove_audio(context, i);
      i -= 1;
    }
  }

  // hard limiter
  for (usize i = 0; i < AL_BATCH_SIZE; ++i) {
    if (f32_abs(buf[i]) > 1) {
      buf[i] = buf[i] < 0 ? -1 : 1;
    }
  }

  al_push_samples(buf, buf);
  arena_temp_release(temp);
}
