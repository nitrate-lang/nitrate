//! Lexer throughput benchmark tool.
//!
//! This benchmark measures lexer throughput (MB/s) across various input types
//! and sizes. It is a standalone binary that can be run with:
//!
//!     cargo run --release --bench throughput_bench
//!
//! It generates synthetic Nitrate source code, lexes all tokens, and reports
//! throughput as MB/s (wall-clock time).

use std::hint::black_box;
use std::time::Instant;

use nitrate_token::*;
use nitrate_token_lexer::*;

// -----------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------

/// Run the lexer over `source` and consume all tokens.
fn lex_all(source: &[u8]) {
    let mut lexer = Lexer::new(source, None).expect("source too big for lexer");
    // Disable trivia for a more realistic "meaningful token" throughput measure.
    lexer.disable_trivia();
    while lexer.next_tok().token != Token::Eof {
        black_box(());
    }
}

/// Run the lexer over `source` and count tokens (trivia disabled).
fn lex_count(source: &[u8]) -> usize {
    let mut lexer = Lexer::new(source, None).expect("source too big for lexer");
    lexer.disable_trivia();
    let mut count = 0;
    while lexer.next_tok().token != Token::Eof {
        count += 1;
    }
    count
}

/// Format a duration as a human-readable string.
fn fmt_duration(d: std::time::Duration) -> String {
    if d.as_secs() > 0 {
        format!("{}.{:03}s", d.as_secs(), d.subsec_millis())
    } else if d.as_millis() > 0 {
        format!("{}ms", d.as_millis())
    } else if d.as_micros() > 0 {
        format!("{}µs", d.as_micros())
    } else {
        format!("{}ns", d.as_nanos())
    }
}

fn fmt_mb_per_sec(bytes: usize, duration: std::time::Duration) -> String {
    let secs = duration.as_secs_f64();
    if secs <= 0.0 {
        "inf".into()
    } else {
        let mb_per_sec = (bytes as f64) / (1024.0 * 1024.0) / secs;
        format!("{:.1} MB/s", mb_per_sec)
    }
}

/// Run a single benchmark iteration, printing results.
fn bench(source: &[u8], label: &str) {
    // Warmup
    for _ in 0..10 {
        lex_all(source);
    }

    let start = Instant::now();
    for _ in 0..100 {
        lex_all(source);
    }
    let elapsed = start.elapsed() / 100;

    let count = lex_count(source);
    let rate = fmt_mb_per_sec(source.len(), elapsed);
    println!(
        "  {:36} {:>12} {:>12} {:>10}",
        label,
        fmt_duration(elapsed),
        rate,
        count,
    );
}

// -----------------------------------------------------------------------
// Source generators
// -----------------------------------------------------------------------

/// Generate a source file with repeating identifiers separated by operators.
fn gen_repeating_identifiers(count: usize) -> Vec<u8> {
    let mut src = String::with_capacity(count * 8);
    for i in 0..count {
        if i > 0 {
            src.push_str(" + ");
        }
        src.push_str("x");
    }
    src.into_bytes()
}

/// Generate a source file with many integer literals.
fn gen_many_integers(count: usize) -> Vec<u8> {
    let mut src = String::with_capacity(count * 10);
    for i in 0..count {
        if i > 0 {
            src.push_str(" + ");
        }
        src.push_str(&format!("{}", (i % 1000) * 7));
    }
    src.into_bytes()
}

/// Generate a source file with many float literals.
fn gen_many_floats(count: usize) -> Vec<u8> {
    let mut src = String::with_capacity(count * 14);
    for i in 0..count {
        if i > 0 {
            src.push_str(" + ");
        }
        src.push_str(&format!("{}.{}", i % 100, (i * 7) % 1000));
    }
    src.into_bytes()
}

/// Generate a source file with many string literals.
fn gen_many_strings(count: usize) -> Vec<u8> {
    let mut src = Vec::with_capacity(count * 30);
    for i in 0..count {
        if i > 0 {
            src.push(b' ');
        }
        src.push(b'"');
        src.extend_from_slice(format!("str{}", i % 50).as_bytes());
        src.push(b'"');
    }
    src
}

/// Generate a source file with many keywords and punctuation (small program).
fn gen_program(kb: usize) -> Vec<u8> {
    let mut src = String::new();
    src.push_str("fn main() {\n");
    for i in 0..kb * 20 {
        src.push_str("    let x");
        src.push_str(&format!("{}", i));
        src.push_str(": i32 = ");
        src.push_str(&format!("{}", i * 7));
        src.push_str(";\n");
    }
    src.push_str("    ret 0;\n");
    src.push_str("}\n");
    src.into_bytes()
}

/// Generate a source file with many UTF-8 identifiers.
fn gen_utf8_program(kb: usize) -> Vec<u8> {
    let mut src = String::new();
    src.push_str("fn main() {\n");
    let vars = ["x", "y", "z", "λ", "π", "ω", "α", "β", "γ", "θ"];
    for i in 0..kb * 15 {
        let var = vars[i % vars.len()];
        src.push_str("    let ");
        src.push_str(var);
        src.push_str(&format!("{}", i));
        src.push_str(" = ");
        src.push_str(&format!("{}", i * 3));
        src.push_str(";\n");
    }
    src.push_str("    ret 0;\n");
    src.push_str("}\n");
    src.into_bytes()
}

/// Generate source with all comment lines.
fn gen_comments(kb: usize) -> Vec<u8> {
    let line = "# This is a comment line used for benchmarking the lexer throughput\n";
    let repeats = (kb * 1024) / line.len();
    let mut src = Vec::with_capacity(repeats * line.len());
    for _ in 0..repeats {
        src.extend_from_slice(line.as_bytes());
    }
    src
}

/// Generate source with a mix of everything.
fn gen_mixed(kb: usize) -> Vec<u8> {
    let mut src = Vec::new();
    src.extend_from_slice(b"# Mixed benchmark\n");
    src.extend_from_slice(b"fn entry() -> i32 {\n");
    for i in 0..kb * 8 {
        match i % 5 {
            0 => {
                src.extend_from_slice(b"    let x");
                src.extend_from_slice(format!("{}", i).as_bytes());
                src.extend_from_slice(b" = ");
                src.extend_from_slice(format!("{}", i * 7).as_bytes());
                src.extend_from_slice(b";\n");
            }
            1 => {
                src.extend_from_slice(b"    let s");
                src.extend_from_slice(format!("{}", i).as_bytes());
                src.extend_from_slice(b" = \"hello ");
                src.extend_from_slice(format!("{}", i).as_bytes());
                src.extend_from_slice(b"\";\n");
            }
            2 => {
                src.extend_from_slice(b"    let f");
                src.extend_from_slice(format!("{}", i).as_bytes());
                src.extend_from_slice(b" = ");
                src.extend_from_slice(format!("{}.{}", i, i * 3).as_bytes());
                src.extend_from_slice(b";\n");
            }
            3 => {
                src.extend_from_slice("    let _λ".as_bytes());
                src.extend_from_slice(format!("{}", i).as_bytes());
                src.extend_from_slice(b" = ");
                src.extend_from_slice(format!("0x{:x}", i).as_bytes());
                src.extend_from_slice(b";\n");
            }
            4 => {
                src.extend_from_slice(b"    ret ");
                src.extend_from_slice(format!("{}", i).as_bytes());
                src.extend_from_slice(b" + ");
                src.extend_from_slice(format!("{}", i * 2).as_bytes());
                src.extend_from_slice(b";\n");
            }
            _ => unreachable!(),
        }
    }
    src.extend_from_slice(b"    ret 0;\n}\n");
    src
}

// -----------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------

fn main() {
    println!("Nitrate Lexer Throughput Benchmark");
    println!("{}", "=".repeat(80));
    println!(
        "{:36} {:>12} {:>12} {:>10}",
        "Input", "Latency/iter", "Throughput", "Tokens"
    );
    println!("{}", "-".repeat(80));

    // ---- Small inputs (micro-benchmarks) ----
    bench(b"let x = 42;", "single let decl");
    bench(b"fn f(a: i32) -> i32 { ret a; }", "small function");
    bench(b"\"hello \\n world\"", "string with escapes");
    bench(&gen_repeating_identifiers(100), "100 identifiers + ops");
    bench(&gen_many_integers(100), "100 integer exprs");
    bench(&gen_many_floats(100), "100 float exprs");
    bench(&gen_many_strings(100), "100 string literals");
    bench(&gen_utf8_program(1), "1KB UTF-8 identifiers");
    bench(&gen_comments(1), "1KB comments");

    // ---- Larger inputs (4KB+) ----
    bench(&gen_program(4), "4KB program");
    bench(&gen_program(16), "16KB program");
    bench(&gen_mixed(4), "4KB mixed content");
    bench(&gen_mixed(16), "16KB mixed content");
    bench(&gen_utf8_program(4), "4KB UTF-8 program");
    bench(&gen_utf8_program(16), "16KB UTF-8 program");

    // Near max test size
    bench(&gen_program(3), "3KB max-test-block");

    // ---- Peak throughput with trivial input ----
    let id_string = "x + ".repeat(2000);
    bench(id_string.as_bytes(), "~12KB ident chain");
    let big_int = "100000 + ".repeat(2000);
    bench(big_int.as_bytes(), "~16KB int chain");

    println!("{}", "=".repeat(80));
    println!("Note: all benchmarks run with trivia DISABLED.");
    println!("Each result is the median of 100 iterations after 10 warmup iterations.");
}
