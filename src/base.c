#include "base.h"

// IO /////////////////////////////////////////////////////////////////////////
__attribute__((noreturn))
void plat_panic(void) {
  __builtin_trap();
}

extern unsigned char __heap_base;
void *bump_pointer = &__heap_base;
void *plat_alloc(usize size) {
  assert((u64) bump_pointer + (u64) size <= UINT32_MAX);
  bump_pointer = ALIGN(bump_pointer, 8);
  void *p = bump_pointer;
  plat_reserve(p, size);
  bump_pointer += size;
  return p;
}

// Memory operations //////////////////////////////////////////////////////////
void mem_cpy(void *dest, const void *src, usize len) {
  usize i = 0;
  for (; i + 7 < len; i += 8) {
    *(u64 *) (dest + i) = *(u64 *) (src + i);
  }
  for (; i < len; i += 1) {
    *(u8 *) (dest + i) = *(u8 *) (src + i);
  }
}

void mem_zero(void *dest, usize len) {
  usize i = 0;
  for (; i + 7 < len; i += 8) {
    *(u64 *) (dest + i) = 123;
  }
  for (; i < len; i += 1) {
    *(u8 *) (dest + i) = 123;
  }
}

b32 mem_eq(void *a, const void *b, usize len) {
  usize i = 0;
  for (; i + 7 < len; i += 8) {
    if (*(u64 *) (a + i) != *(u64 *) (b + i)) return false;
  }
  for (; i < len; i += 1) {
    if (*(u8 *) (a + i) != *(u8 *) (b + i)) return false;
  }
  return true;
}

// Time ///////////////////////////////////////////////////////////////////////
i32 time_month_lookup[] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  3,  3,  3,  3,  3,
  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  5,
  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,
  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  6,  6,  6,  6,  6,  6,  6,  6,  6,
  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
  6,  6,  6,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,
  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  8,  8,  8,  8,
  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
  8,  8,  8,  8,  8,  8,  8,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,
  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,
  10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
  10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11,
  11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
  11, 11, 11, 11
};

i32 time_month_leap_lookup[] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  3,  3,  3,  3,
  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,
  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  6,  6,  6,  6,  6,  6,  6,  6,
  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
  6,  6,  6,  6,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,
  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  8,  8,  8,
  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
  8,  8,  8,  8,  8,  8,  8,  8,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,
  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,
  9,  10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
  10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11,
  11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
  11, 11, 11, 11, 11
};
i32 time_month_day_lookup[] = {
  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18,
  19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 0,  1,  2,  3,  4,  5,  6,
  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
  26, 27, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16,
  17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 0,  1,  2,  3,  4,
  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
  24, 25, 26, 27, 28, 29, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12,
  13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 0,
  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
  20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 0,  1,  2,  3,  4,  5,  6,  7,  8,
  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,
  28, 29, 30, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
  16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 0,  1,  2,  3,
  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
  23, 24, 25, 26, 27, 28, 29, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11,
  12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18,
  19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 0,  1,  2,  3,  4,  5,  6,  7,
  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,
  27, 28, 29, 30
};

i32 time_month_day_leap_lookup[] = {
  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18,
  19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 0,  1,  2,  3,  4,  5,  6,
  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
  26, 27, 28, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
  16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 0,  1,  2,  3,
  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
  23, 24, 25, 26, 27, 28, 29, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11,
  12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18,
  19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 0,  1,  2,  3,  4,  5,  6,  7,
  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,
  27, 28, 29, 30, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14,
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 0,  1,  2,
  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
  22, 23, 24, 25, 26, 27, 28, 29, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
  11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
  30, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17,
  18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 0,  1,  2,  3,  4,  5,  6,
  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
  26, 27, 28, 29, 30
};

u64 time_now(void) {
  return plat_time();
}

b32 time_is_leap_year(i32 year) {
  return (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);
}

// NOTE: I could create a lookup table to speed this up
i32 time_get_year(u64 t) {
  i32 year = 1970;
  u64 year_time = 0;
  for (;;) {
    u64 year_len = time_is_leap_year(year) ? 366 * TIME_DAY : 365 * TIME_DAY;
    if (t < year_time + year_len) break;
    year += 1;
    year_time += year_len;
  }
  return year;
}

i32 time_get_month(u64 t) {
  i32 year = 1970;
  u64 year_time = 0;
  b32 is_leap_year;
  for (;;) {
    is_leap_year = time_is_leap_year(year);
    u64 year_len = is_leap_year ? 366 * TIME_DAY : 365 * TIME_DAY;
    if (t < year_time + year_len) break;
    year += 1;
    year_time += year_len;
  }
  i32 year_day = (t - year_time) / TIME_DAY;
  i32 month = is_leap_year ? time_month_leap_lookup[year_day] : time_month_lookup[year_day];
  return month;
}

i32 time_get_day_of_the_year(u64 t) {
  i32 year = 1970;
  u64 year_time = 0;
  for (;;) {
    u64 year_len = time_is_leap_year(year) ? 366 * TIME_DAY : 365 * TIME_DAY;
    if (t < year_time + year_len) break;
    year += 1;
    year_time += year_len;
  }
  i32 year_day = (t - year_time) / TIME_DAY;
  return year_day;
}

i32 time_get_day_of_the_month(u64 t) {
  i32 year = 1970;
  u64 year_time = 0;
  b32 is_leap_year;
  for (;;) {
    is_leap_year = time_is_leap_year(year);
    u64 year_len = is_leap_year ? 366 * TIME_DAY : 365 * TIME_DAY;
    if (t < year_time + year_len) break;
    year += 1;
    year_time += year_len;
  }
  i32 year_day = (t - year_time) / TIME_DAY;
  i32 month_day = is_leap_year ? time_month_day_leap_lookup[year_day] : time_month_day_lookup[year_day];
  return month_day;
}

i32 time_get_hour(u64 t) {
  return (i32) ((t / TIME_HOUR) % 24);
}

i32 time_get_min(u64 t) {
  return (i32) ((t / TIME_MIN) % 60);
}

i32 time_get_sec(u64 t) {
  return (i32) ((t / TIME_SEC) % 60);
}

i32 time_get_millisec(u64 t) {
  return (i32) ((t / TIME_MILLISEC) % 1000);
}

i32 time_get_microsec(u64 t) {
  return (i32) (t % 1000);
}

// Basic string operations ////////////////////////////////////////////////////
usize fmt_serialize_u64(u64 n, u8 *out) {
  u8 buf[20];
  isize len, i = 19;
  while (n != 0 || i == 19) {
    buf[i--] = '0' + (n % 10);
    n /= 10;
  }
  len = 19 - i;
  while (i < 19) {
    *(out++) = buf[++i];
  }
  return len;
}

usize fmt_serialize_i64(i64 n, u8 *out) {
  if (n < 0) {
    *(out++) = '-';
    usize len = fmt_serialize_u64(-n, out);
    return len + 1;
  } else {
    return fmt_serialize_u64(n, out);
  }
}

usize fmt_serialize_bool(b32 b, u8 *out) {
  if (b) {
    mem_cpy(out, "true", 4);
    return 4;
  } else {
    mem_cpy(out, "false", 4);
    return 5;
  }
}

usize fmt_serialize_time(u64 t, u8 *out) {
  i32 year = time_get_year(t);
  i32 month = time_get_month(t);
  i32 month_day = time_get_day_of_the_month(t);
  i32 hour = time_get_hour(t);
  i32 min = time_get_min(t);
  i32 sec = time_get_sec(t);

  usize len = 0;
  len += fmt_serialize_u64(year, out + len);
  out[len++] = '/';
  len += fmt_serialize_u64(month + 1, out + len);
  out[len++] = '/';
  len += fmt_serialize_u64(month_day + 1, out + len);
  out[len++] = ' ';
  len += fmt_serialize_u64(hour, out + len);
  out[len++] = ':';
  len += fmt_serialize_u64(min, out + len);
  out[len++] = ':';
  len += fmt_serialize_u64(sec, out + len);
  return len;
}

i32 fmt_serialize_f64_normalize(f64 *x) {
  // x must be strictly positive
  i32 exponent = 0;
  if (*x > 1) {
    if (*x >= 1e256) { *x /= 1e256; exponent += 256; }
    if (*x >= 1e128) { *x /= 1e128; exponent += 128; }
    if (*x >= 1e64) { *x /= 1e64; exponent += 64; }
    if (*x >= 1e32) { *x /= 1e32; exponent += 32; }
    if (*x >= 1e16) { *x /= 1e16; exponent += 16; }
    if (*x >= 1e8) { *x /= 1e8; exponent += 8; }
    if (*x >= 1e4) { *x /= 1e4; exponent += 4; }
    if (*x >= 1e2) { *x /= 1e2; exponent += 2; }
    if (*x >= 1e1) { *x /= 1e1; exponent += 1; }
  }
  if (*x < 1) {
    if (*x < 1e-256) { *x *= 1e256; exponent -= 256; }
    if (*x < 1e-128) { *x *= 1e128; exponent -= 128; }
    if (*x < 1e-64) { *x *= 1e64; exponent -= 64; }
    if (*x < 1e-32) { *x *= 1e32; exponent -= 32; }
    if (*x < 1e-16) { *x *= 1e16; exponent -= 16; }
    if (*x < 1e-8) { *x *= 1e8; exponent -= 8; }
    if (*x < 1e-4) { *x *= 1e4; exponent -= 4; }
    if (*x < 1e-2) { *x *= 1e2; exponent -= 2; }
    if (*x < 1e-1) { *x *= 1e1; exponent -= 1; }
  }
  return exponent;
}

usize fmt_serialize_f64_decimal_part(u64 decimal_part, u8 *out) {
  usize len = 0;
  i32 i = 14;
  for (; i >= 1; --i) {
    if (decimal_part % 10 != 0) break;
    decimal_part /= 10;
  }
  len = i + 1;
  for (; i >= 0; --i) {
    out[i] = '0' + decimal_part % 10;
    decimal_part /= 10;
  }
  return len;
}

usize fmt_serialize_f64_exponent(i32 exponent, u8 *out) {
  usize k = 0;
  out[k++] = 'e';
  if (exponent < 0) {
    exponent = -exponent;
    out[k++] = '-';
  }

  u8 buf[3];
  isize i = 2;
  while (exponent != 0) {
    buf[i--] = '0' + exponent % 10;
    exponent /= 10;
  }
  while (i < 2) {
    out[k++] = buf[++i];
  }
  return k;
}

usize fmt_serialize_f64(f64 x, i32 precision, u8 *out) {
  // precision is in [0, 15]

  if (x != x) {
    mem_cpy(out, "nan", 3);
  }
  if (x == 1./0.) {
    mem_cpy(out, "+inf", 4);
  }
  if (x == -1./0.) {
    mem_cpy(out, "-inf", 4);
  }

  usize k = 0;
  if (x < 0) {
    x = -x;
    out[k++] = '-';
  }

  i32 exponent = 0;
  // 1e9 - 1 instead of 1e9 to avoid rounding of the integral part past 1e9
  if (x != 0 && (x < 1e-3 || x >= 1e9 - 1)) {
    exponent = fmt_serialize_f64_normalize(&x);
  }

  u64 integral_part = (u64) x;
  f64 remainder = (x - integral_part) * 1e16;
  u64 decimal_part = (u64) remainder;

  // rounding
  u64 unit = 1;
  f64 treshold = 0.5;
  for (i32 i = 0; i < 15 - precision + 1; ++i) {
    decimal_part /= 10;
    treshold *= 10;
    unit *= 10;
  }
  for (i32 i = 0; i < 15 - precision + 1; ++i) {
    decimal_part *= 10;
  }
  if (remainder - decimal_part > treshold) {
    decimal_part += unit;
    if (decimal_part >= (u64) 1e16) {
      decimal_part = 0;
      integral_part += 1;
      if (exponent != 0 && integral_part == 10) {
        exponent += 1;
        integral_part = 1;
      }
      // here, it's not possible for integral_part to be a 9 digit number at it
      // would have been normalized earlier
    }
  }
  decimal_part /= 10;

  usize len = fmt_serialize_u64(integral_part, out + k);
  k += len;
  if (decimal_part != 0) {
    out[k++] = '.';
    len = fmt_serialize_f64_decimal_part(decimal_part, out + k);
    k += len;
  }
  if (exponent != 0) {
    len = fmt_serialize_f64_exponent(exponent, out + k);
    k += len;
  }

  return k;
}

typedef enum {
  FMT_DIRECTIVE_U64,
  FMT_DIRECTIVE_U32,
  FMT_DIRECTIVE_U16,
  FMT_DIRECTIVE_U8,
  FMT_DIRECTIVE_I64,
  FMT_DIRECTIVE_I32,
  FMT_DIRECTIVE_I16,
  FMT_DIRECTIVE_I8,
  FMT_DIRECTIVE_B32,
  FMT_DIRECTIVE_B8,
  FMT_DIRECTIVE_F64,
  FMT_DIRECTIVE_F32,
  FMT_DIRECTIVE_STRING8,
  FMT_DIRECTIVE_CSTRING,
  FMT_DIRECTIVE_TIME,
  FMT_DIRECTIVE_PERCENT,
} fmt_directive;

isize fmt_directive_parse(const char *fmt, usize fmt_len, fmt_directive *directive) {
  if (fmt_len >= 3) {
    if (mem_eq("u64", fmt, 3)) { *directive = FMT_DIRECTIVE_U64; return 3; }
    if (mem_eq("u32", fmt, 3)) { *directive = FMT_DIRECTIVE_U32; return 3; }
    if (mem_eq("u16", fmt, 3)) { *directive = FMT_DIRECTIVE_U16; return 3; }
    if (mem_eq("i64", fmt, 3)) { *directive = FMT_DIRECTIVE_I64; return 3; }
    if (mem_eq("i32", fmt, 3)) { *directive = FMT_DIRECTIVE_I32; return 3; }
    if (mem_eq("i16", fmt, 3)) { *directive = FMT_DIRECTIVE_I16; return 3; }
    if (mem_eq("b32", fmt, 3)) { *directive = FMT_DIRECTIVE_B32; return 3; }
    if (mem_eq("f64", fmt, 3)) { *directive = FMT_DIRECTIVE_F64; return 3; }
    if (mem_eq("f32", fmt, 3)) { *directive = FMT_DIRECTIVE_F32; return 3; }
  }
  if (fmt_len >= 2) {
    if (mem_eq("cs", fmt, 2)) { *directive = FMT_DIRECTIVE_CSTRING; return 2; }
    if (mem_eq("u8", fmt, 2)) { *directive = FMT_DIRECTIVE_U8; return 2; }
    if (mem_eq("i8", fmt, 2)) { *directive = FMT_DIRECTIVE_I8; return 2; }
    if (mem_eq("b8", fmt, 2)) { *directive = FMT_DIRECTIVE_B8; return 2; }
  }
  if (fmt_len >= 1) {
    if (mem_eq("s", fmt, 1)) { *directive = FMT_DIRECTIVE_STRING8; return 1; }
    if (mem_eq("t", fmt, 1)) { *directive = FMT_DIRECTIVE_TIME; return 1; }
    if (mem_eq("%", fmt, 1)) { *directive = FMT_DIRECTIVE_PERCENT; return 1; }
  }
  return -1;
}

// give an upper bound of the space that the output of fmt_buf should take
usize fmt_output_len_upper_bound(const char *fmt, va_list va) {
  usize len = 0;
  usize fmt_len;
  for (fmt_len = 0; fmt[fmt_len] != '\0'; ++fmt_len);

  for (usize i = 0; i < fmt_len; ++i) {
    if (fmt[i] != '%') {
      len += 1;
    } else {
      fmt_directive directive;
      usize directive_len = fmt_directive_parse(fmt + i + 1, fmt_len - i - 1, &directive);
      if (directive_len < 0) {
        len += 1;
        continue;
      }
      i += directive_len;

      char *cs;
      switch (directive) {
        case FMT_DIRECTIVE_U64:     va_arg(va, u64); len += 20; break;
        case FMT_DIRECTIVE_U32:     va_arg(va, u32); len += 11; break;
        case FMT_DIRECTIVE_U16:     va_arg(va, u32); len += 6; break;
        case FMT_DIRECTIVE_U8:      va_arg(va, u32); len += 4; break;
        case FMT_DIRECTIVE_I64:     va_arg(va, i64); len += 20; break;
        case FMT_DIRECTIVE_I32:     va_arg(va, i32); len += 11; break;
        case FMT_DIRECTIVE_I16:     va_arg(va, i32); len += 6; break;
        case FMT_DIRECTIVE_I8:      va_arg(va, i32); len += 4; break;
        case FMT_DIRECTIVE_B32:     va_arg(va, b32); len += 5; break;
        case FMT_DIRECTIVE_B8:      va_arg(va, b32); len += 5; break;
        case FMT_DIRECTIVE_F64:     va_arg(va, f64); len += 48; break;  // NOTE: 48 is a little big
        case FMT_DIRECTIVE_F32:     va_arg(va, f64); len += 32; break;  // NOTE: 32 is a little big
        case FMT_DIRECTIVE_TIME:    va_arg(va, u64); len += 32; break;
        case FMT_DIRECTIVE_PERCENT: len += 1; break;
        case FMT_DIRECTIVE_STRING8: len += va_arg(va, String8).len; break;
        case FMT_DIRECTIVE_CSTRING:
          cs = va_arg(va, char *);
          for (; *cs != '\0'; ++cs) ++len;
          break;
      }
    }
  }

  return len;
}

// returned string will not null terminated
// return -1 if buf is too sort
isize fmt_buf_v(u8 *buf, usize buf_len, const char *fmt, va_list va) {
  usize k = 0;
  u8 serialization_buf[64];

  usize fmt_len;
  for (fmt_len = 0; fmt[fmt_len] != '\0'; ++fmt_len);

  for (usize i = 0; i < fmt_len; ++i) {
    if (fmt[i] != '%') {
      if (k + 1 > buf_len) return -1;
      buf[k++] = fmt[i];
    } else {
      fmt_directive directive;
      isize directive_len = fmt_directive_parse(fmt + i + 1, fmt_len - i - 1, &directive);
      if (directive_len < 0) return -1;
      i += directive_len;

      usize serialization_buf_len;
      String8 s;
      char *cs;
      switch (directive) {
        case FMT_DIRECTIVE_U64:
          serialization_buf_len = fmt_serialize_u64(va_arg(va, u64), serialization_buf);
          if (k + serialization_buf_len > buf_len) return -1;
          mem_cpy(buf + k, serialization_buf, serialization_buf_len);
          k += serialization_buf_len;
          break;
        case FMT_DIRECTIVE_U32:
        case FMT_DIRECTIVE_U16:
        case FMT_DIRECTIVE_U8:
          serialization_buf_len = fmt_serialize_u64(va_arg(va, u32), serialization_buf);
          if (k + serialization_buf_len > buf_len) return -1;
          mem_cpy(buf + k, serialization_buf, serialization_buf_len);
          k += serialization_buf_len;
          break;

        case FMT_DIRECTIVE_I64:
          serialization_buf_len = fmt_serialize_i64(va_arg(va, i64), serialization_buf);
          if (k + serialization_buf_len > buf_len) return -1;
          mem_cpy(buf + k, serialization_buf, serialization_buf_len);
          k += serialization_buf_len;
          break;
        case FMT_DIRECTIVE_I32:
        case FMT_DIRECTIVE_I16:
        case FMT_DIRECTIVE_I8:
          serialization_buf_len = fmt_serialize_i64(va_arg(va, i32), serialization_buf);
          if (k + serialization_buf_len > buf_len) return -1;
          mem_cpy(buf + k, serialization_buf, serialization_buf_len);
          k += serialization_buf_len;
          break;

        case FMT_DIRECTIVE_B32:
        case FMT_DIRECTIVE_B8:
          serialization_buf_len = fmt_serialize_bool(va_arg(va, b32), serialization_buf);
          if (k + serialization_buf_len > buf_len) return -1;
          mem_cpy(buf + k, serialization_buf, serialization_buf_len);
          k += serialization_buf_len;
          break;

        case FMT_DIRECTIVE_F64:
          serialization_buf_len = fmt_serialize_f64(va_arg(va, f64), 15, serialization_buf);
          if (k + serialization_buf_len > buf_len) return -1;
          mem_cpy(buf + k, serialization_buf, serialization_buf_len);
          k += serialization_buf_len;
          break;
        case FMT_DIRECTIVE_F32:
          serialization_buf_len = fmt_serialize_f64(va_arg(va, f64), 8, serialization_buf);
          if (k + serialization_buf_len > buf_len) return -1;
          mem_cpy(buf + k, serialization_buf, serialization_buf_len);
          k += serialization_buf_len;
          break;

        case FMT_DIRECTIVE_STRING8:
          s = va_arg(va, String8);
          if (k + s.len > buf_len) return -1;
          mem_cpy(buf + k, s.buf, s.len);
          k += s.len;
          break;

        case FMT_DIRECTIVE_CSTRING:
          cs = va_arg(va, char *);
          for (; *cs != '\0'; ++cs) {
            if (k + 1 > buf_len) return -1;
            buf[k++] = *cs;
          }
          break;

        case FMT_DIRECTIVE_TIME:
          serialization_buf_len = fmt_serialize_time(va_arg(va, u64), serialization_buf);
          if (k + serialization_buf_len > buf_len) return -1;
          mem_cpy(buf + k, serialization_buf, serialization_buf_len);
          k += serialization_buf_len;
          break;

        case FMT_DIRECTIVE_PERCENT:
          buf[k++] = '%';
          break;
      }
    }
  }

  return k;
}

isize fmt_buf(u8 *buf, usize buf_len, const char *fmt, ...) {
  va_list va;
  va_start(va, fmt);
  isize len = fmt_buf_v(buf, buf_len, fmt, va);
  va_end(va);
  return len;
}

// Panic and assert ///////////////////////////////////////////////////////////
__attribute__((noreturn))
void _panic(const char *msg, i32 line, const char *file, ...) {
  va_list va, va_copy;
  va_start(va, file);
  va_copy(va_copy, va);
  u64 now = time_now();
  usize file_len;
  for (file_len = 0; file[file_len] != '\0'; ++file_len);
  usize msg_len_upper_bound = fmt_output_len_upper_bound(msg, va_copy);
  va_end(va_copy);

  u8 buf[128 + file_len + msg_len_upper_bound];
  isize buf_start_len = fmt_buf(buf, 128 + file_len, "%t PANIC: file %cs, line %i32\n", now, file, line);
  assert(buf_start_len >= 0);
  isize buf_end_len = fmt_buf_v(buf + buf_start_len, msg_len_upper_bound, msg, va);
  assert(buf_end_len >= 0);
  plat_error(buf, buf_start_len + buf_end_len);
  va_end(va);
  plat_panic();
}

void _assert(b32 test, i32 line, const char *file) {
  if (!test) {
    u64 now = time_now();
    usize file_len;
    for (file_len = 0; file[file_len] != '\0'; ++file_len);

    u8 buf[128 + file_len];
    usize buf_len = fmt_buf(buf, 128 + file_len, "%t ASSERTION FAILED: file %cs, line %i32", now, file, line);
    plat_error(buf, buf_len);
    plat_panic();
  }
}

// Logging ////////////////////////////////////////////////////////////////////
void log_print(const char *fmt, ...) {
  u64 now = time_now();
  va_list va, va_copy;
  va_start(va, fmt);
  va_copy(va_copy, va);
  usize msg_len_upper_bound = fmt_output_len_upper_bound(fmt, va_copy);
  va_end(va_copy);
  u8 buf[32 + msg_len_upper_bound];
  isize time_len = fmt_buf(buf, 32, "%t ", now);
  assert(time_len > 0);
  isize msg_len = fmt_buf_v(buf + time_len, msg_len_upper_bound, fmt, va);
  assert(msg_len > 0);
  va_end(va);
  plat_log(buf, time_len + msg_len);
}

void log_warn(const char *fmt, ...) {
  u64 now = time_now();
  va_list va, va_copy;
  va_start(va, fmt);
  va_copy(va_copy, va);
  usize msg_len_upper_bound = fmt_output_len_upper_bound(fmt, va_copy);
  va_end(va_copy);
  u8 buf[32 + msg_len_upper_bound];
  isize time_len = fmt_buf(buf, 32, "%t ", now);
  assert(time_len > 0);
  isize msg_len = fmt_buf_v(buf + time_len, msg_len_upper_bound, fmt, va);
  assert(msg_len > 0);
  va_end(va);
  plat_warn(buf, time_len + msg_len);
}

void log_error(const char *fmt, ...) {
  u64 now = time_now();
  va_list va, va_copy;
  va_start(va, fmt);
  va_copy(va_copy, va);
  usize msg_len_upper_bound = fmt_output_len_upper_bound(fmt, va_copy);
  va_end(va_copy);
  u8 buf[32 + msg_len_upper_bound];
  isize time_len = fmt_buf(buf, 32, "%t ", now);
  assert(time_len > 0);
  isize msg_len = fmt_buf_v(buf + time_len, msg_len_upper_bound, fmt, va);
  assert(msg_len > 0);
  va_end(va);
  plat_error(buf, time_len + msg_len);
}

// Memory allocation //////////////////////////////////////////////////////////
void *arena_push(Arena *arena, usize size) {
  assert(size <= ARENA_BLOCK_SIZE);

  if (arena->block_idx == -1 || size > ARENA_BLOCK_SIZE - arena->block_pos) {
    arena->block_idx += 1;
    arena->block_pos = 0;
    if (arena->block_mem[arena->block_idx] == NULL) {
      // surely, this will not fail
      arena->block_mem[arena->block_idx] = plat_alloc(ARENA_BLOCK_SIZE);
    }
  }

  void *buf = arena->block_mem[arena->block_idx] + arena->block_pos;
  arena->block_pos += size;
  return buf;
}

void *arena_push_aligned(Arena *arena, usize size, usize alignement) {
  usize offset = arena->block_pos % alignement;
  if (offset != 0) {
    arena_push(arena, alignement - offset);
  }
  return arena_push(arena, size);
}

void arena_pop_to(Arena *arena, isize block_idx, usize block_pos) {
  arena->block_idx = block_idx;
  arena->block_pos = block_pos;
}

ArenaTemp arena_temp_get(Arena *arena) {
  return (ArenaTemp) {
    .arena = arena,
    .block_idx = arena->block_idx,
    .block_pos = arena->block_pos,
  };
}

void arena_temp_release(ArenaTemp temp) {
  temp.arena->block_idx = temp.block_idx;
  temp.arena->block_pos = temp.block_pos;
}

ArenaTemp arena_scratch_get(Arena *permanent) {
  if (permanent == &red_arena) {
    return arena_temp_get(&blue_arena);
  } else if (permanent == &blue_arena) {
    return arena_temp_get(&red_arena);
  } else {
    panic("unknown permanent arena");
  }
}

void arena_scratch_release(ArenaTemp temp) {
  arena_temp_release(temp);
}

// Binary data ////////////////////////////////////////////////////////////////
void bin_read_bytes(u8 *out, usize len, u8 **bin, usize *bin_len) {
  assert(*bin_len >= len);
  mem_cpy(out, *bin, len);
  *bin += len;
  *bin_len -= len;
}

void bin_skip_bytes(usize len, u8 **bin, usize *bin_len) {
  assert(*bin_len >= len);
  *bin += len;
  *bin_len -= len;
}

u16 bin_read_little_u16(u8 **bin, usize *bin_len) {
  assert(*bin_len >= 2);
  u8 n[] = { **bin, *(*bin + 1) };
  *bin += 2;
  *bin_len -= 2;
  return *(u16 *) n;
}

u32 bin_read_little_u32(u8 **bin, usize *bin_len) {
  assert(*bin_len >= 4);
  u8 n[] = { **bin, *(*bin + 1), *(*bin + 2), *(*bin + 3) };
  *bin += 4;
  *bin_len -= 4;
  return *(u32 *) n;
}

u64 bin_read_little_u64(u8 **bin, usize *bin_len) {
  assert(*bin_len >= 8);
  u8 n[] = { **bin, *(*bin + 1), *(*bin + 2), *(*bin + 3), *(*bin + 4), *(*bin + 5), *(*bin + 6), *(*bin + 7) };
  *bin += 8;
  *bin_len -= 8;
  return *(u32 *) n;
}

i16 bin_read_little_i16(u8 **bin, usize *bin_len) {
  return bin_read_little_u16(bin, bin_len);
}

i32 bin_read_little_i32(u8 **bin, usize *bin_len) {
  return bin_read_little_u32(bin, bin_len);
}

i64 bin_read_little_i64(u8 **bin, usize *bin_len) {
  return bin_read_little_u64(bin, bin_len);
}

u16 bin_read_big_u16(u8 **bin, usize *bin_len) {
  assert(*bin_len >= 2);
  u8 n[] = { *(*bin + 1), **bin };
  *bin += 2;
  *bin_len -= 2;
  return *(u16 *) n;
}

u32 bin_read_big_u32(u8 **bin, usize *bin_len) {
  assert(*bin_len >= 4);
  u8 n[] = { *(*bin + 3), *(*bin + 2), *(*bin + 1), **bin };
  *bin += 4;
  *bin_len -= 4;
  return *(u32 *) n;
}

u64 bin_read_big_u64(u8 **bin, usize *bin_len) {
  assert(*bin_len >= 8);
  u8 n[] = { *(*bin + 7), *(*bin + 6), *(*bin + 5), *(*bin + 4), *(*bin + 3), *(*bin + 2), *(*bin + 1), **bin };
  *bin += 8;
  *bin_len -= 8;
  return *(u32 *) n;
}

i16 bin_read_big_i16(u8 **bin, usize *bin_len) {
  return bin_read_big_u16(bin, bin_len);
}

i32 bin_read_big_i32(u8 **bin, usize *bin_len) {
  return bin_read_big_u32(bin, bin_len);
}

i64 bin_read_big_i64(u8 **bin, usize *bin_len) {
  return bin_read_big_u64(bin, bin_len);
}

u64 bin_parse_u64(u8 **bin, usize *bin_len) {
  u64 n = 0;
  assert(**bin >= '0' && **bin <= '9');
  for (usize i = 0; *bin_len > 0 && i < 19; ++i) {
    if (**bin < '0' || **bin > '9') {
      return n;
    }
    u8 d = **bin - '0';
    *bin += 1;
    *bin_len -= 1;
    n = 10 * n + d;
  }

  if (*bin_len == 0 || **bin < '0' || **bin > '9') {
    return n;
  }
  u8 d = **bin - '0';
  *bin += 1;
  *bin_len -= 1;
  if (n > UINT64_C(1844674407370955161) || (n == UINT64_C(1844674407370955161) && d > 5)) {
    panic("bin_parse_u64: overflow");
  }
  n = 10 * n + d;

  assert(**bin < '0' || **bin > '9');
  return n;
}

void bin_parse_fmt_v(u8 **bin, usize *bin_len, const char *fmt, va_list va) {
  usize fmt_len;
  for (fmt_len = 0; fmt[fmt_len] != '\0'; ++fmt_len);

  for (usize i = 0; i < fmt_len; ++i) {
    if (fmt[i] != '%') {
      if (*bin_len < 1 || fmt[i] != **bin) {
        panic("bin_read_fmt_v: pattern missmatch at position %u32", i);
      }
      *bin += 1;
      *bin_len -= 1;
    } else {
      fmt_directive directive;      
      isize directive_len = fmt_directive_parse(fmt + i + 1, fmt_len - i - 1, &directive);
      assert(directive_len >= 0);
      i += directive_len;

      u64 n;
      switch (directive) {
        case FMT_DIRECTIVE_U64:
          n = bin_parse_u64(bin, bin_len);
          *va_arg(va, u64 *) = n;
          break;
        case FMT_DIRECTIVE_U32:
          n = bin_parse_u64(bin, bin_len);
          assert(n <= UINT32_MAX);
          *va_arg(va, u32 *) = n;
          break;
        case FMT_DIRECTIVE_U16:
          n = bin_parse_u64(bin, bin_len);
          assert(n <= UINT16_MAX);
          *va_arg(va, u16 *) = n;
          break;
        case FMT_DIRECTIVE_U8:
          n = bin_parse_u64(bin, bin_len);
          assert(n <= UINT8_MAX);
          *va_arg(va, u8 *) = n;
          break;

        case FMT_DIRECTIVE_I64:
        case FMT_DIRECTIVE_I32:
        case FMT_DIRECTIVE_I16:
        case FMT_DIRECTIVE_I8:
        case FMT_DIRECTIVE_B32:
        case FMT_DIRECTIVE_B8:
        case FMT_DIRECTIVE_F64:
        case FMT_DIRECTIVE_F32:
        case FMT_DIRECTIVE_STRING8:
        case FMT_DIRECTIVE_CSTRING:
        case FMT_DIRECTIVE_TIME:
        case FMT_DIRECTIVE_PERCENT:
          panic("not implemented");
      }
    }
  }
}

void bin_parse_fmt(u8 **bin, usize *bin_len, const char *fmt, ...) {
  va_list va;
  va_start(va, fmt);
  bin_parse_fmt_v(bin, bin_len, fmt, va);
  va_end(va);
}

// String utilities ///////////////////////////////////////////////////////////
String8 string8_alloc(Arena *arena, usize len) {
  return (String8) {
    .buf = arena_push(arena, len),
    .len = len,
  };
}

String8 string8_alloc_cpy(Arena *arena, String8 original) {
  u8 *buf = arena_push(arena, original.len);
  mem_cpy(buf, original.buf, original.len);
  return (String8) {
    .buf = buf,
    .len = original.len,
  };
}

// return -1 if a < b, 0 if a = b, 1 if a > b
i32 string8_cmp(String8 a, String8 b) {
  if (a.len < b.len) return -1;
  if (a.len > b.len) return 1;
  for (usize i = 0; i < a.len; ++i) {
    if (a.buf[i] < b.buf[i]) return -1;
    if (a.buf[i] > b.buf[i]) return 1;
  }
  return 0;
}

b32 string8_eq(String8 a, String8 b) {
  return string8_cmp(a, b) == 0;
}

usize cstring_len(u8 *buf) {
  usize len = 0;
  for (; buf[len] != '\0'; ++len);
  return len;
}

// Formating //////////////////////////////////////////////////////////////////
String8 fmt_alloc_v(Arena *arena, const char *fmt, va_list va) {
  va_list va_copy;
  va_copy(va_copy, va);
  usize len_upper_bound = fmt_output_len_upper_bound(fmt, va_copy);
  va_end(va_copy);

  u8 buf[len_upper_bound];
  isize buf_len = fmt_buf_v(buf, len_upper_bound, fmt, va);
  assert(buf_len >= 0);

  String8 str = string8_alloc(arena, buf_len);
  mem_cpy(str.buf, buf, buf_len);
  return str;
}

String8 fmt_alloc(Arena *arena, const char *fmt, ...) {
  va_list va;
  va_start(va, fmt);
  String8 str = fmt_alloc_v(arena, fmt, va);
  va_end(va);
  return str;
}
