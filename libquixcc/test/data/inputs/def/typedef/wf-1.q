type callback = *fn _(x: i32): i32;

fn impl(x: i32): i32 {
    return x;
}

pub let f: callback = impl;