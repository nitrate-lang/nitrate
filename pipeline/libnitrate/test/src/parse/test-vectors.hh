#pragma once

#include <string_view>

namespace test::vector {
  constexpr static inline std::string_view kAstExecise =
      R"SOURCE(################################################################################
# NITRATE PIPELINE TEST VECTOR                                                 #
# Date: Jan 13, 2024                                                           #
################################################################################
#
################################################################################
# Statements                                                                   #
################################################################################
if true {};

if false {} else {};

if false {} else if false {} else {};;

retif true, 0;

switch 0 {
  0 => 0;
  1 => 1;
  _ => 2;
}

ret 0;
ret;

break;

continue;

while true {};

for (let i = 0; (i < 10); (i++)) {};

for (;;) {};

foreach (i, v in [0, 1, 2]) {};

foreach (v in [0, 1, 2]) {};

# Inline assembly is not implemented yet
10;

type MyTypeDef = void;

struct [Special, 10] MyStruct<x: i32 = 0, i = 0, t>: Object, MyTrait {
  pub a: u32 = 0,
  pub b: i32,
  pro c: f32 = 0.0,
  pro d: f64,
  sec e: u8 = 0,
  sec f: i8,
  pub fn foo();
  pro fn bar();
  sec fn baz();
  pub static fn foo();
  pro static fn bar();
  sec static fn baz();
}

region [Special, 10] MyStruct<x: i32 = 0, i = 0, t>: Object, MyTrait {
  pub a: u32 = 0,
  pub b: i32,
  pro c: f32 = 0.0,
  pro d: f64,
  sec e: u8 = 0,
  sec f: i8,
  pub fn foo();
  pro fn bar();
  sec fn baz();
  pub static fn foo();
  pro static fn bar();
  sec static fn baz();
}

class [Special, 10] MyStruct<x: i32 = 0, i = 0, t>: Object, MyTrait {
  pub a: u32 = 0,
  pub b: i32,
  pro c: f32 = 0.0,
  pro d: f64,
  sec e: u8 = 0,
  sec f: i8,
  pub fn foo();
  pro fn bar();
  sec fn baz();
  pub static fn foo();
  pro static fn bar();
  sec static fn baz();
}

group [Special, 10] MyStruct<x: i32 = 0, i = 0, t>: Object, MyTrait {
  pub a: u32 = 0,
  pub b: i32,
  pro c: f32 = 0.0,
  pro d: f64,
  sec e: u8 = 0,
  sec f: i8,
  pub fn foo();
  pro fn bar();
  sec fn baz();
  pub static fn foo();
  pro static fn bar();
  sec static fn baz();
}

union [Special, 10] MyStruct<x: i32 = 0, i = 0, t>: Object, MyTrait {
  pub a: u32 = 0,
  pub b: i32,
  pro c: f32 = 0.0,
  pro d: f64,
  sec e: u8 = 0,
  sec f: i8,
  pub fn foo();
  pro fn bar();
  sec fn baz();
  pub static fn foo();
  pro static fn bar();
  sec static fn baz();
}

enum hello: u32 {
  A = 0,
  B,
  C = 2,
}

enum hello2 {
  A = 0,
  B,
  C = 2,
}

scope nested => pub "abi" [attr1, attr2] fn DaBlock<a:i32=0,oo:i8>(a=20,...) {
  let var1: u32 = 0;
  let var2 = 0;
  let var2: u8;
}

################################################################################
# Expressions                                                                  #
################################################################################
################################################################################
# Types                                                                        #
################################################################################
type _0 = u1: [0:1]: 2;
type _1 = u8: [0:1]: 2;
type _2 = u16: [0:1]: 2;
type _3 = u32: [0:1]: 2;
type _4 = u64: [0:1]: 2;
type _5 = u128: [0:1]: 2;
type _6 = i8: [0:1]: 2;
type _7 = i16: [0:1]: 2;
type _8 = i32: [0:1]: 2;
type _9 = i64: [0:1]: 2;
type _10 = i128: [0:1]: 2;
type _11 = f16: [0:1]: 2;
type _12 = f32: [0:1]: 2;
type _13 = f64: [0:1]: 2;
type _14 = f128: [0:1]: 2;
type _15 = void: [0:1]: 2;
type _16 = ?: [0:1]: 2;
type _17 = opaque(T): [0:1]: 2;
type _18 = T: [0:1]: 2;
type _19 = &T: [0:1]: 2;
type _20 = *T: [0:1]: 2;
type _21 = [T; 10]: [0:1]: 2;
type _22 = (T, U, V): [0:1]: 2;
type _23 = T<x: i32, i32>: [0:1]: 2;
type _24 = fn[[
  align(16)
]]  quasi(x: i32 = 0, x, ...): void: [0:1]: 2;
)SOURCE";
};