pub "C" fn printn(x: string);

fn std::spawn(f: fn()) {

}

fn main() {
  std::spawn(fn() {
    let x = 0;
    while {
      printn(f"Hello, world! {x++}");
    }
  });
}