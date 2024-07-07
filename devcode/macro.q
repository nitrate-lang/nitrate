@use "v1.0";

// @(fn add(x: i32, y: i32): i32 {
//   ret x + y;
// })

// @add(1, 20);

group Universe {
  pub fn start() {

  }

  pub fn stop() {

  }
};

fn main() {
  let u: Universe;

  u.start();

  /// Play God

  u.stop();

  /// End of the world
  printn(u.to_str());
}