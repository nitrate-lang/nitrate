/**
 * @brief Forward Meta Static Arguments
 */
@(fn forward(args: [string]): plaintext => return res.join(", "); );

/**
 * @brief Interface to QSys API
 */
@(fn qsys(qnum: u32, ...) => nit.api.qsys(@forward(args)); )

/**
 * @brief Print Function
 */
@(fn print(args: [string]) {
  for (let i = 0; i < args.len(); i++) {
    @qsys(1, args[i]);
  }
})

/**
 * @brief Print Function with Newline
 */
@(fn printn(args: [string]) {
  for (let i = 0; i < args.len(); i++) {
    @qsys(1, args[i]);
  }
  @qsys(1, "\n");
})

fn main() {
  @printn("Hello, World!");
}
