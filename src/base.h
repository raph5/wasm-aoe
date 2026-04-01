/* Copyright (c) 2025, Raphaël Guyader
 * All rights reserved.
 *
 * This source code is licensed under the GPL-3.0 license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef H_BASE
#define H_BASE

#include <stdint.h>
#include <stdarg.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;

typedef float f32;
typedef double f64;

typedef u32 b32;
typedef u8 b8;

// wasm32 as a 32 bits address space
typedef u32 usize;
typedef i32 isize;

// Marcros ////////////////////////////////////////////////////////////////////
#define KiB(n)  (((u64)(n)) << 10)
#define MiB(n)  (((u64)(n)) << 20)
#define GiB(n)  (((u64)(n)) << 30)

#define NULL (void *) 0
#define true 1
#define false 0

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define ALIGN(p, n) ((void *) (((uintptr_t) (p) + (n) - 1) & ~(uintptr_t) ((n) - 1)))

#define static_assert _Static_assert

// Vector /////////////////////////////////////////////////////////////////////
typedef struct {
  u8 r, g, b;  
} Color;

// Math ///////////////////////////////////////////////////////////////////////
static inline f32 f32_abs(f32 x) {
    union { f32 f; u32 i; } u = { x };
    u.i &= 0x7fffffff;
    return u.f;
}

// IO /////////////////////////////////////////////////////////////////////////
extern void plat_log(u8 *buf, usize len);
extern void plat_warn(u8 *buf, usize len);
extern void plat_error(u8 *buf, usize len);
extern u64 plat_time(void);  // in microsec since epoch
extern void plat_reserve(void *start, usize len);
__attribute__((noreturn))
void plat_panic(void);
void *plat_alloc(usize size);

// Graphics ///////////////////////////////////////////////////////////////////
typedef struct {
  i32 x, y, width, height, uv_x, uv_y;
  u32 texture_id;
} gl_sprite;
static_assert(sizeof(gl_sprite) == 28, "JS might parse that struct wrong");

extern void gl_set_resolution(i32 width, i32 height);
extern u32 gl_texture_create(u8 *buf, i32 width, i32 height);  // takes rgba buffer, return a texture id
extern void gl_texture_destroy(u32 texture_id);
extern void gl_draw_sprite(gl_sprite *sprite);
extern void gl_clear_screen(void);

// Sounds /////////////////////////////////////////////////////////////////////
#define AL_RATE 32000
#define AL_BATCH_SIZE 2048

// pushes 2048 audio samples from each channels to the host
// `first_channel` and `second_channel` can be equal
// TODO: remove support for stereo
extern void al_push_samples(f32 *first_channel, f32 *second_channel);

// Memory operations //////////////////////////////////////////////////////////
void mem_cpy(void *dest, const void *src, usize len);
void mem_zero(void *dest, usize len);
b32 mem_eq(void *a, const void *b, usize len);

// String type ////////////////////////////////////////////////////////////////
typedef struct {
  u8 *buf;
  usize len;
} String8;

#define string8_static(s) (String8) { .buf = (u8 *) s, .len = sizeof(s) - 1 }
#define string8_buf(s) (String8) { .buf = (u8 *) s, .len = sizeof(s) }

// Time ///////////////////////////////////////////////////////////////////////
#define TIME_MICROSEC UINT64_C(1)
#define TIME_MILLISEC UINT64_C(1000)
#define TIME_SEC      UINT64_C(1000000)
#define TIME_MIN      UINT64_C(60000000)
#define TIME_HOUR     UINT64_C(3600000000)
#define TIME_DAY      UINT64_C(86400000000)
u64 time_now(void);  // WARN: should not be used for game time
b32 time_is_year(i32 year);
i32 time_get_year(u64 t);              // from 1970 to ...
i32 time_get_month(u64 t);             // from 0 to 11
i32 time_get_day_of_the_year(u64 t);   // from 0 to 365
i32 time_get_day_of_the_month(u64 t);  // from 0 to 30
i32 time_get_hour(u64 t);              // from 0 to 23
i32 time_get_min(u64 t);               // from 0 to 59
i32 time_get_sec(u64 t);               // from 0 to 59
i32 time_get_millisec(u64 t);          // from 0 to 999
i32 time_get_microsec(u64 t);          // from 0 to 999

// Basic formating ////////////////////////////////////////////////////////////
// returned string will not null terminated
// return -1 if buf is too sort
isize fmt_buf_v(u8 *buf, usize buf_len, const char *fmt, va_list va);
isize fmt_buf(u8 *buf, usize buf_len, const char *fmt, ...);

// Panic and assert ///////////////////////////////////////////////////////////
#define panic(msg, ...) _panic(msg, __LINE__, __FILE__ __VA_OPT__(,) __VA_ARGS__)
#define assert(test) _assert((b32) (test), __LINE__, __FILE__)
__attribute__((noreturn))
void _panic(const char *msg, i32 line, const char *file, ...);
void _assert(b32 test, i32 line, const char *file);

// Logging ////////////////////////////////////////////////////////////////////
void log_print(const char *fmt, ...);
void log_warn(const char *fmt, ...);
void log_error(const char *fmt, ...);

// Memory allocation //////////////////////////////////////////////////////////
#define GPA_ALLOCATION_BLOCK_SIZE KiB(64)
#define GPA_LIMIT MiB(512)
#define GPA_FREE_LIST_NODE_CREATION_THRESHOLD 512
typedef struct GpaFreeListNode GpaFreeListNode;
struct GpaFreeListNode {
  struct GpaFreeListNode *next;
  usize block_size;
};

typedef struct {
  usize block_size;
  u32 margin_before;
  u32 margin_after;
} GpaAllocationHeader;

typedef struct {
  void *start;
  void *end;
  GpaFreeListNode *free_list;
} Gpa;

#define ARENA_REGION_SIZE_DEFAULT KiB(64)
typedef struct _ArenaRegion ArenaRegion;
struct _ArenaRegion {
  struct _ArenaRegion *next;
  usize size;
  usize pos;
  u64 _[];  // to insure 8 bytes alignement
};

typedef struct {
  Gpa *allocator;
  usize region_size;
  usize region_count;
  ArenaRegion *begin, *end;
} Arena;

typedef struct {
  Arena *arena;
  ArenaRegion *region;
  usize pos;
} ArenaTemp;

void gpa_init(Gpa *gpa);  // gpa must be initialized
void *gpa_alloc(Gpa *gpa, usize size);
void gpa_free(Gpa *gpa, void *ptr);
f32 gpa_get_memory_fragmentation_coefficient(Gpa *gpa);

void arena_destroy(Arena *arena);
void arena_reset(Arena *arena);
void *arena_push(Arena *arena, usize size);

ArenaTemp arena_temp_get(Arena *arena);
void arena_temp_release(ArenaTemp temp);

// Binary data ////////////////////////////////////////////////////////////////
u8 bin_read_byte(u8 **bin, usize *bin_len);
void bin_read_bytes(u8 *out, usize len, u8 **bin, usize *bin_len);
void bin_skip_bytes(usize len, u8 **bin, usize *bin_len);
u8 bin_read_little_u8(u8 **bin, usize *bin_len);    // little endian
u16 bin_read_little_u16(u8 **bin, usize *bin_len);  // little endian
u32 bin_read_little_u32(u8 **bin, usize *bin_len);  // little endian
u64 bin_read_little_u64(u8 **bin, usize *bin_len);  // little endian
u8 bin_read_big_u8(u8 **bin, usize *bin_len);    // big endian
u16 bin_read_big_u16(u8 **bin, usize *bin_len);  // big endian
u32 bin_read_big_u32(u8 **bin, usize *bin_len);  // big endian
u64 bin_read_big_u64(u8 **bin, usize *bin_len);  // big endian

void bin_parse_fmt_v(u8 **bin, usize *bin_len, const char *fmt, va_list va);  // equivalent to scanf
void bin_parse_fmt(u8 **bin, usize *bin_len, const char *fmt, ...);  // equivalent to scanf

// String utilities ///////////////////////////////////////////////////////////
String8 string8_alloc(Arena *arena, usize len);
String8 string8_alloc_cpy(Arena *arena, String8 original);
i32 string8_cmp(String8 a, String8 b);  // return -1 if a < b, 0 if a = b, 1 if a > b
b32 string8_eq(String8 a, String8 b);  // return a == b

usize cstring_len(u8 *buf);

// Formating //////////////////////////////////////////////////////////////////
String8 fmt_alloc_v(Arena *arena, const char *fmt, va_list va);
String8 fmt_alloc(Arena *arena, const char *fmt, ...);

// Image //////////////////////////////////////////////////////////////////////
typedef struct {
  i32 width;
  i32 height;
  u8 buf[];
} Image;

#endif
