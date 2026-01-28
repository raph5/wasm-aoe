
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

#define NULL 0
#define true 1
#define false 0

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define static_assert _Static_assert

// Vector /////////////////////////////////////////////////////////////////////
typedef struct {
  u8 r, g, b;  
} Color;

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
extern u32 al_wav_create(u8 *buf, u32 buf_len);  // take wav file, return a wav id
extern void al_wav_destroy(u32 wav_id);
extern void al_wav_play(u32 wav_id);

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

// The Memory Allocation Strategy:
// Memory allocation happens via two permanent arenas, a blue and a red.
// Underlying memory of an arena is divided into 16MiB blocks that are
// dynamicaly allocated as the arena grows. This allows us to grow de memory
// linearly instead of allocating the two arena upfront.
//
// A temporary arena can be created from a permanent arena by remembering an
// initial block_idx and block_pos.
// A temporary arena can extand on multiple blocks.
// A temporary arena is then released by setting it permanent arena back to
// back to its initial state (block_idx and block_pos).
// Having two arenas and temporary arenas allows us to use a *scratch arena*
// mechanism.
//
// See: https://www.rfleury.com/p/untangling-lifetimes-the-arena-allocator
//
// WARN: Having a scratch arena mechanism mean that you can't pass two
// separate arenas as arguments to a function.

#define ARENA_BLOCK_COUNT 128
#define ARENA_BLOCK_SIZE MiB(16)
typedef struct {
  isize block_idx;
  usize block_pos;
  void *block_mem[ARENA_BLOCK_COUNT];
} Arena;

typedef struct {
  Arena *arena;
  isize block_idx;
  usize block_pos;
} ArenaTemp;

Arena red_arena = { .block_idx = -1 };
Arena blue_arena = { .block_idx = -1 };

void *arena_push(Arena *arena, usize size);
void arena_pop_to(Arena *arena, isize block_idx, usize block_pos);

ArenaTemp arena_temp_get(Arena *arena);
void arena_temp_release(ArenaTemp temp);

ArenaTemp arena_scratch_get(Arena *permanent);
void arena_scratch_release(ArenaTemp temp);

// Binary data ////////////////////////////////////////////////////////////////
void bin_read_bytes(u8 *out, usize len, u8 **bin, usize *bin_len);
void bin_skip_bytes(usize len, u8 **bin, usize *bin_len);
u16 bin_read_little_u16(u8 **bin, usize *bin_len);  // little endian
u32 bin_read_little_u32(u8 **bin, usize *bin_len);  // little endian
u64 bin_read_little_u64(u8 **bin, usize *bin_len);  // little endian
i16 bin_read_little_i16(u8 **bin, usize *bin_len);  // little endian
i32 bin_read_little_i32(u8 **bin, usize *bin_len);  // little endian
i64 bin_read_little_i64(u8 **bin, usize *bin_len);  // little endian
u16 bin_read_big_u16(u8 **bin, usize *bin_len);  // big endian
u32 bin_read_big_u32(u8 **bin, usize *bin_len);  // big endian
u64 bin_read_big_u64(u8 **bin, usize *bin_len);  // big endian
i16 bin_read_big_i16(u8 **bin, usize *bin_len);  // big endian
i32 bin_read_big_i32(u8 **bin, usize *bin_len);  // big endian
i64 bin_read_big_i64(u8 **bin, usize *bin_len);  // big endian

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
