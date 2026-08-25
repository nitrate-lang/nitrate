extern "C" {
    fn [no_mangle] printf(format: *const u8, ...) -> i32;
}

struct Point<T> {
    pub x: T,
    pub y: T,
}

fn foo<T>() -> Point<T> {
    Point { x: 10 as T, y: 20 as T }
}

extern "C" fn [no_mangle] main() {
    let x = foo::<i32>();
    let a = x;
    printf("x: %d\ny: %d\n\0", a.x, a.y);
}
