
# Custom pack archive format

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
