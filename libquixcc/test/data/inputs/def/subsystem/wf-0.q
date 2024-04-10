subsystem MyAPI<v1>: dep1, dep2 {
    fn init();
    fn deinit();
    fn do_something_super_important(): i32;
}

pub fn MyAPI<v1>::init() {
    return;
}

pub fn MyAPI<v1>::deinit() {
    return;
}

pub fn MyAPI<v1>::do_something_super_important(): i32 {
    return 0xdeadbeef;
}

fn main() {
    return;
}