@use "v1.0";

// @(fn print(msg) {
//   @qsys(0, msg);
// });

// @(fn metacode() {
//   for (let i = 0; i < 10; i++) {
//     @print(i, "Hello, World!");
//   }
// });

fn main() {
  @qsys(10, "/etc/passwd", "", "");
  @qsys(20, "/etc/passwd", "", "");
}