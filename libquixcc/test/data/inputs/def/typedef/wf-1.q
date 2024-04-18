type callback = *fn _(x: i32): i32;

fn impl(x: i32): i32 {
    ret x;
}

pub let f: callback = impl;