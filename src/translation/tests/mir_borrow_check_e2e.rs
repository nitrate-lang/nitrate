//! End-to-end regression tests for MIR borrow checking.
//!
//! These tests compile real `.nit` source through the full pipeline (lex →
//! parse → HIR → validate → mangle → MIR) and assert on the diagnostics the
//! MIR borrow checker reports. They exist because the MIR lowering decides
//! *where* `Move` operands are emitted — the borrow checker can only reject a
//! double move if the lowering actually moves the value.

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

/// Run the full pipeline (through MIR borrow checking) on `source` and return
/// every diagnostic message that was reported.
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

const STRUCT_SOURCE: &str = r#"
struct S {
    pub a: i32,
    pub b: i32,
}

fn take(_s: S) -> i32 {
    ret 0;
}
"#;

#[test]
fn double_move_is_rejected_end_to_end() {
    let source = format!(
        r#"{STRUCT_SOURCE}
fn main() {{
    let s = S {{ a: 1, b: 2 }};
    let r1 = take(s);
    let r2 = take(s);
    ret;
}}
"#
    );
    let diags = compile_source(&source, "double_move_e2e");
    assert!(
        diags.iter().any(|d| d.contains("use of moved value")),
        "expected a use-after-move diagnostic from the borrow checker, got:\n{diags:#?}"
    );
}

#[test]
fn single_move_is_accepted_end_to_end() {
    let source = format!(
        r#"{STRUCT_SOURCE}
fn main() {{
    let s = S {{ a: 1, b: 2 }};
    let r1 = take(s);
    ret;
}}
"#
    );
    let diags = compile_source(&source, "single_move_e2e");
    assert!(
        diags.iter().all(|d| !d.contains("use of moved value")),
        "single move must not be flagged, got:\n{diags:#?}"
    );
}

#[test]
fn copy_type_reuse_is_accepted_end_to_end() {
    // Copy types (primitives, tuples of primitives) may be used any number of
    // times without being deinitialized.
    let source = format!(
        r#"{STRUCT_SOURCE}
fn main() {{
    let c: i32 = 5;
    let d: i32 = c;
    let e: i32 = c;
    let t: (i32, i32) = (1, 2);
    let u: (i32, i32) = t;
    let v: (i32, i32) = t;
    ret;
}}
"#
    );
    let diags = compile_source(&source, "copy_type_e2e");
    assert!(
        diags.iter().all(|d| !d.contains("use of moved value")),
        "copy types must not be flagged as moved, got:\n{diags:#?}"
    );
}
