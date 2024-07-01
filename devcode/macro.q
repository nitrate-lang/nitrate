@use "v1.0";


@(fn macro_1(): *i8 {
  ret "";
});

@(fn macro_2(): *i8 {
  ret "";
});

@(fn macro_3(): *i8 {
  ret "";
});

fn main() {
  let mac1 = @macro_1();
  let mac2 = @macro_2();
  let mac3 = @macro_3();
}
