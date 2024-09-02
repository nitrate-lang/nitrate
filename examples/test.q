pub fn pure pack4(a: [u8; 4]): u32 {
  let res: u32 = 0;
  res |= a[0] as u32 << 00;
  res |= a[1] as u32 << 08;
  res |= a[2] as u32 << 16;
  res |= a[3] as u32 << 24;

  ret res;
}

//==============================================================================

; Module: examples/test.q
; Passes: [ds-acyclic, ds-nilchk, ds-chtype, ds-acyclic, ds-nilchk, ds-resolv, ds-acyclic, 
;          ds-nilchk, ns-flatten, ds-acyclic, ds-nilchk, fnflatten, ds-acyclic, 
;          ds-nilchk, tyinfer, ds-acyclic, ds-nilchk, nm-premangle]
; Timestamp: 2024-09-01 19:49:55
; Compiler: [undefined] [x86_64-linux-clang] [debug]
; Copyright (C) 2024 Wesley Jones


seq {
  export[] __pub0 = seq {
    fn pack4([u8; const 4]) seq {
      local res = (0 cast_as u32),
      (res = (res | ((a[0] cast_as u32) << 0))),
      (res = (res | ((a[1] cast_as u32) << 8))),
      (res = (res | ((a[2] cast_as u32) << 16))),
      (res = (res | ((a[3] cast_as u32) << 24))),
      ret res,
    },
  },
}