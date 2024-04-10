subsystem MyAPI<v1>: dep1, dep2 {
    fn init();

    subsystem Nested<another_version>: list_your_deps_here {
        pub let x: i32 = 0xdeadc0de;