region Region {
  a: [u8; 9],
  b: i16,
}

struct Struct {
  a: [u8; 9],
  b: i16,
}

group Group {
  a: [u8; 9],
  b: i16,
}

union Union {
  a: [u8; 9],
  b: i16,
}

pub "C" fn main(argc: i32): i32 {
  let r: Region;
  let s: Struct;
  let g: Group;
  let u: Union;
}
