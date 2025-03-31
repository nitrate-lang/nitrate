// Write a program that uses a single synchronous filesystem
// operation to read a file and print the number of newlines
// (\n) it contains to the console (stdout), similar to
// running cat file | wc -l.

@import sysio;

fn count_lines(filename: string): usize {
  let f = std::file::open<r|b>(filename, buffer: true);
  throwz f, @src() + f.errs();
  ret f.stream().count('\n');
}

fn main(v: [string]): i32 {
  retif v < 2, f"Usage: {args[0]} <filename>".e(), 1;

  ret @printn(count_lines(args[1])), 0;
}