all: dev

CC := clang
TARGET := out.wasm
SRC := src/main.c data/assets.o
CFLAGS := --target=wasm32 -nostdlib -std=c99 -Wall -Wextra
LFLAGS := -Wl,--no-entry \
					-Wl,--export=game_init \
					-Wl,--export=game_update \
					-Wl,--export=game_input_ptr \
					-Wl,--export=game_output_ptr \
					-Wl,--allow-undefined \
					-Wl,-z,stack-size=8388608

dev:
	$(CC) -g $(CFLAGS) $(LFLAGS) $(SRC) -o $(TARGET)

build:
	$(CC) -O3 $(CFLAGS) $(LFLAGS) $(SRC) -o $(TARGET)
