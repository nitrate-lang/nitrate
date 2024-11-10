region Region {
  sec a: [u8; 9],
  sec b: i16,
};

struct Struct {
  sec a: [u8; 9],
  sec b: i16,
};

group Group {
  sec a: [u8; 9],
  sec b: i16,
};

union Union {
  sec a: [u8; 9],
  sec b: i16,
};

pub "c" fn main(argc: i32): i32 {
  let r: Region;
  let s: Struct;
  let g: Group;
  let u: Union;
}
