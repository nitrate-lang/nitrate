import "C" fn printf(format: *i8, ...): i32;

fn fib(n: u32): u32 {
    retif n, n == 0 || n == 1;
    ret fib(n - 1) + fib(n - 2);
}

fn main(): i32 {
    printf("fib(10) = %d\n", fib(10));
    ret 0;
}