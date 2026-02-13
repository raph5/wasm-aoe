/* Copyright (c) 2025, Raphaël Guyader
 * All rights reserved.
 *
 * This source code is licensed under the GPL-3.0 license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef H_UI
#define H_UI

#include "base.h"

typedef enum {
  UI_EVENT_RESIZE,
} UiEventType;

typedef struct {
  UiEventType type;
  union {
    struct {
      i32 width;
      i32 height;
    } resize;
  } data;
} UiEvent;

typedef struct {
} UiState;

void ui_frame_set(UiState *state, i32 x, i32 y, Color c);
void ui_frame_resize(UiState *state, i32 width, i32 height);

void ui_event_process(UiState *state, UiEvent *event);

#endif
