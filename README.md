
This repo contains an open source reimplementation of Age of Empires I 1997.
This reimplementation is written in C to target WASM.

Assets of the original game are, off course, not provided. No revese
engeniering of any proprietary software was done by the authors of this
software. This reimplementation is based upon open source documentation of the
game assets formats.

# Some Documentation

## Age of empires Assets

Some documentation about the game file format can be found in the `docs`
directory.

### MIDI

Age of empires soundtrack is stored in MIDI files. This project embeds a custom
player that can read and play MIDI files given a SF2 soundfont file. For the
sake of simplicity the MIDI player does not support some MIDI/SF2 features:
- MIDI sysex events are not supported

## Custom pack archive format

Age of empires assets are packaged directly into the wasm binary following a
simple custom archive format:

```
[Header]
  magic[4] = "PACK"
  uint32 version
  uint32 file_count

[File Table]
  repeated file_count times:
    uint32 name_len
    char   name[name_len]
    uint64 offset
    uint64 size

[File Data Blob]
  raw file bytes concatenated
```

To PACK archive:
1. `cd` the directory you want to archive
2. `fd -t f > files` to list every files in this directory
3. remove files you don't want in the archive from `files`
4. run `utils/pack.py < files > out.pack`

## Wololo

*5051.wav*

# License

Copyright (c) 2025, Raphaël Guyader
All rights reserved.

This source code is licensed under the GPL-3.0 license found in the
LICENSE file in the root directory of this source tree.
