all: dev

CC := clang
TARGET := out.wasm
SRC := src/main.c data/assets.o
CFLAGS := --target=wasm32 -nostdlib -fno-builtin -std=c99 -Wall -Wextra
LFLAGS := -Wl,--no-entry \
					-Wl,--export=game_init \
					-Wl,--export=game_state_tick \
					-Wl,--export=game_frame_tick \
					-Wl,--export=game_audio_tick \
					-Wl,--export=game_state_tick_input_ptr \
					-Wl,--import-undefined \
					-Wl,-z,stack-size=8388608

dev:
	$(CC) -O0 -g $(CFLAGS) $(LFLAGS) $(SRC) -o $(TARGET)

build:
	$(CC) -O3 $(CFLAGS) $(LFLAGS) $(SRC) -o $(TARGET)
