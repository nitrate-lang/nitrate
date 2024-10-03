#pragma once

#include <math.h>

namespace quix {
  typedef signed char i8;
  typedef unsigned char u8;
  typedef signed short i16;
  typedef unsigned short u16;
  typedef signed int i32;
  typedef unsigned int u32;
  typedef signed long long i64;
  typedef unsigned long long u64;
  typedef __int128 i128;
  typedef unsigned __int128 u128;

  typedef __fp16 f16;
  typedef float f32;
  typedef double f64;
  typedef __float128 f128;

  typedef u64 usize; 
  constexpr u64 usize_max = 0xffffffffffffffff;
}  // namespace quix
