import "C" fn printf(format: *i8, ...): i32;

fn do_inline_asm(): i32 {
    let x: i32;

    __asm__("mov $$42, $0", {"=r": &x}, {}, ["esi"]);

    return x;
}

fn main(): void {
    let x: i32 = do_inline_asm();

    printf("x = %d\n", x);

    ret;
}