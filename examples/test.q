pub "c" fn printf(s: *i8 = "", ...): i32;

fn main(args: [string]) {
  printf("Hello, world!\n");
  ret 0;
}

///=============================================================================

; Module: examples/test.q
; Passes: [ds-acyclic, ds-nullchk, ds-chtype, ds-acyclic, ds-nullchk, ds-resolv, ds-acyclic,
;          ds-nullchk, ns-flatten, ds-acyclic, ds-nullchk, fnflatten, ds-acyclic,
;          ds-nullchk, tyinfer, ds-acyclic, ds-nullchk, nm-premangle]
; Timestamp: 2024-09-02 19:27:21
; Compiler: [undefined] [x86_64-linux-clang] [debug]
; Copyright (C) 2024 Wesley Jones


seq {
  extern "c" local printf = fn (i8*,...): void,
  fn main([string]) seq {
    printf("Hello, world!\n"),
    ret 0,
  },
}