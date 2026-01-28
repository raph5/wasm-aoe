#!/usr/bin/env python3

import sys
import os
import struct

VERSION = 1

file_list = []
for line in sys.stdin:
    if line == "\n": continue
    file_name = line[0:-1]
    assert all(ord(c) < 128 for c in file_name)
    file_list.append(file_name)

sys.stdout.buffer.write(b"PACK")
sys.stdout.buffer.write(struct.pack("!I", VERSION))
sys.stdout.buffer.write(struct.pack("!I", len(file_list)))

header_offset = 12
for file_name in file_list:
    header_offset += 20 + len(file_name)

offset_table = []
body_offset = 0
for file_name in file_list:
    file_name_lower = file_name.lower()
    offset = header_offset + body_offset
    size = os.path.getsize(file_name)
    body_offset += size
    offset_table.append(offset)
    sys.stdout.buffer.write(struct.pack("!I", len(file_name_lower)))
    sys.stdout.buffer.write(bytes(file_name_lower, "ascii"))
    sys.stdout.buffer.write(struct.pack("!Q", offset))
    sys.stdout.buffer.write(struct.pack("!Q", size))

for idx, file_name in enumerate(file_list):
    print(offset_table[idx], sys.stdout.buffer.tell())
    file = open(file_name, "rb")
    while chunk := file.read(8192):
        sys.stdout.buffer.write(chunk)
    file.close()
