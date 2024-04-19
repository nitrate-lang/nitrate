
import "C" {
    fn printf(format: *i8, ...): i32;
}

fn fib(n: u32): u32 {
    if n == 0 || n == 1 {
        ret n;
    }
    
    ret fib(n - 1) + fib(n - 2);
}

fn main(): i32 {
    printf("fib(10) = %d\n", fib(10));
    printf("fib(20) = %d\n", fib(20));
    printf("fib(30) = %d\n", fib(30));
}