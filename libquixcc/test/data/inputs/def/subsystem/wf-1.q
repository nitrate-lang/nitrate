subsystem MyAPI<v1>: dep1, dep2 {
    fn init();

    subsystem Nested<another_version>: list_your_deps_here {
        pub let x: i32 = 0xdeadc0de;
    }
}

import "C" fn printf(format: *i8, ...);

fn main() {
    printf("The value is %x\n", MyAPI<v1>::Nested<another_version>::x);

    ret;
}