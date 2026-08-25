//! End-to-end regression tests for generic monomorphization.
//!
//! These tests compile real `.nit` source that uses generics — generic
//! structs, generic functions with turbofish calls (`foo::<i32>()`) and
//! inference-driven calls (`identity(42)`) — through the full pipeline (lex →
//! parse → HIR → solve → validate → mangle → MIR). Before the fixes in the
//! solver, generic functions failed validation, monomorphized copies were
//! never solved, and unresolved `Type::GenericParam`s reached MIR lowering.

use nitrate_diagnosis::CompilerLog;
use nitrate_translation::pipeline::{Pipeline, PipelineConfig};
use std::sync::{Arc, Mutex};

/// A slog drain that collects every formatted diagnostic message.
struct VecDrain(Arc<Mutex<Vec<String>>>);

impl slog::Drain for VecDrain {
    type Ok = ();
    type Err = slog::Never;

    fn log(&self, record: &slog::Record<'_>, _values: &slog::OwnedKVList) -> Result<Self::Ok, Self::Err> {
        self.0.lock().unwrap().push(record.msg().to_string());
        Ok(())
    }
}

/// Run the full pipeline (through MIR lowering) on `source` and return every
/// diagnostic message that was reported. Panics if the pipeline fails.
fn compile_source(source: &str, package_name: &str) -> Vec<String> {
    let path = std::env::temp_dir().join(format!(
        "no3_{package_name}_{}.nit",
        std::process::id()
    ));
    std::fs::write(&path, source.trim_start()).expect("write test source");

    let messages: Arc<Mutex<Vec<String>>> = Arc::new(Mutex::new(Vec::new()));
    let log = CompilerLog::new(slog::Logger::root(VecDrain(messages.clone()), slog::o!()));

    let mut config = PipelineConfig::default();
    config.package_name = package_name.to_string();
    config.log = log.clone();

    let hir_store = nitrate_translation::hir::Store::new();
    let mir_store = nitrate_translation::mir::MirStore::new();

    let pipeline = Pipeline::new(config);
    let parsed = pipeline
        .load_source(&path)
        .and_then(|s| s.lex())
        .and_then(|t| t.parse())
        .unwrap_or_else(|e| {
            panic!(
                "lex/parse failed: {e}\ncaptured diagnostics:\n{:#?}",
                messages.lock().unwrap().clone()
            )
        });
    let ptr_size = std::mem::size_of::<*const u8>() as u32;

    nitrate_translation::hir::using_storage(&hir_store, || {
        let hir_lowered = parsed.lower_hir(ptr_size).expect("HIR lowering failed");
        let hir_validated = hir_lowered.validate().expect("HIR validation failed");
        let hir_mangled = hir_validated.mangle();
        nitrate_translation::mir::using_storage(&mir_store, || {
            hir_mangled.lower_mir();
        });
    });

    let _ = std::fs::remove_file(&path);
    messages.lock().unwrap().clone()
}

/// The canonical generic test: a generic function returning a generic struct
/// literal, instantiated via explicit turbofish type arguments at the call
/// site. This is the case that previously failed validation with a return
/// type mismatch and left unresolved generic params in MIR lowering.
const GENERIC_TURBOFISH_SOURCE: &str = r#"
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
"#;

#[test]
fn generic_turbofish_compiles_end_to_end() {
    let diags = compile_source(GENERIC_TURBOFISH_SOURCE, "generic_turbofish_e2e");
    assert!(
        diags.is_empty(),
        "generic turbofish program should compile cleanly, got:\n{diags:#?}"
    );
}

/// A generic function whose type parameter is inferred from a literal
/// argument (`identity(42)` drives `T := i32`).
const GENERIC_INFERENCE_SOURCE: &str = r#"
extern "C" {
    fn [no_mangle] printf(format: *const u8, ...) -> i32;
}

fn identity<T>(x: T) -> T {
    x
}

extern "C" fn [no_mangle] main() {
    let v = identity(42);
    printf("identity: %d\n\0", v);
}
"#;

#[test]
fn generic_inference_compiles_end_to_end() {
    let diags = compile_source(GENERIC_INFERENCE_SOURCE, "generic_inference_e2e");
    assert!(
        diags.is_empty(),
        "inference-driven generic call should compile cleanly, got:\n{diags:#?}"
    );
}

/// An uncalled generic function is a template: it must not be lowered to MIR
/// and must not poison compilation.
const UNCALLED_GENERIC_SOURCE: &str = r#"
extern "C" {
    fn [no_mangle] printf(format: *const u8, ...) -> i32;
}

fn unused<T>(x: T) -> T {
    x
}

extern "C" fn [no_mangle] main() {
    printf("hello\n\0");
}
"#;

#[test]
fn uncalled_generic_compiles_end_to_end() {
    let diags = compile_source(UNCALLED_GENERIC_SOURCE, "uncalled_generic_e2e");
    assert!(
        diags.is_empty(),
        "uncalled generic function should not break compilation, got:\n{diags:#?}"
    );
}

/// A generic struct with multiple type parameters, instantiated with two
/// concrete arguments.
const MULTI_PARAM_GENERIC_SOURCE: &str = r#"
extern "C" {
    fn [no_mangle] printf(format: *const u8, ...) -> i32;
}

struct Pair<A, B> {
    pub first: A,
    pub second: B,
}

fn both<A, B>(a: A, b: B) -> Pair<A, B> {
    Pair { first: a, second: b }
}

extern "C" fn [no_mangle] main() {
    let p = both::<i32, i64>(1, 2);
    printf("pair: %d %lld\n\0", p.first, p.second);
}
"#;

#[test]
fn multi_param_generic_compiles_end_to_end() {
    let diags = compile_source(MULTI_PARAM_GENERIC_SOURCE, "multi_param_generic_e2e");
    assert!(
        diags.is_empty(),
        "multi-parameter generic struct should compile cleanly, got:\n{diags:#?}"
    );
}
