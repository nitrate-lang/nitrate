subsystem MyAPI<v1>: dep1, dep2 {
    fn init();
    fn deinit();
    fn do_something_super_important(): i32;
}

pub fn MyAPI<v1>::init() {
    ret;
}

pub fn MyAPI<v1>::deinit() {
    ret;
}

pub fn MyAPI<v1>::do_something_super_important(): i32 {
    ret 0xdeadbeef;
}

fn main() {
    ret;
}