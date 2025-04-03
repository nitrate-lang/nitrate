#!nitrate run

@use "v1.0";

import "C" fn printf(fmt: string, ...): i32;

@(fn qsysmap(name: string): i32 {
    const map = {
        "read":  0,
        "write": 1,
    };

    retif map.has(name), map[name];

    @error("Failed to resolve metatopic system call: {name}.",
        hints: ["Are you using the correct language version?"]);
    @never();
});

@(fn pure write(fd: u64, buf: [u8], escape_bad: bool = false): isize {
    ret @qsys(@qsysmap("write"), fd, buf, true, escape_bad);
});


@(fn pure print(fmt: string, args: [any]...) {
    let !m = "";
    let !vi = 0;

    foreach (i: !usize, c: char in fmt) {
        if (c != '{') { m += c; cont; }                         ~> Append regular character to message.
        if (!fmt.nextis(i, '}')) { m += '{'; cont; }            ~> Append a regular '{' character.
        if (vi >= args.len()) =>
            @error("Not enough arguments to format string.");   ~> Error if not enough arguments.
        m += args[vi++].to_str(); i++;                          ~> Serialize the argument to a string and append.
    }
    for (; vi < args.len(); vi++) { m += args[vi].to_str(); }   ~> Append any remaining arguments.
    @write(1, m.to_bytes(), true);                              ~> Write the formatted message to stdout.
});

fn main(): i32 {
    @print("The answer is {}\n", 42);
    ret 0;
}
