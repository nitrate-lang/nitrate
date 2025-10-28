use crate::{TranslationOptions, TranslationOptionsBuilder, options::Diagnose};
use nitrate_diagnosis::{CompilerLog, FileId, intern_file_id};
use nitrate_token_lexer::{Lexer, LexerError};
use nitrate_tree::ast::Module;
use nitrate_tree_parse::{Parser, ResolveCtx};
use threadpool::ThreadPool;
use threadpool_scope::scope_with;

#[derive(Debug)]
pub enum TranslationError {
    ScannerError(std::io::Error),
    LexerError(LexerError),
    SyntaxError,
    NameResolutionError,
    TypeCheckingError,
    DiagnosticError,
}

pub fn compile_debugable_code(
    source_code: &mut dyn std::io::Read,
    machine_code: &mut dyn std::io::Write,
) -> Result<(), TranslationError> {
    let debug_options = TranslationOptionsBuilder::default_debug_build_options()
        .build()
        .expect("Default debug build options should always be valid");

    compile_code(source_code, machine_code, &debug_options)
}

pub fn compile_fast_code(
    source_code: &mut dyn std::io::Read,
    machine_code: &mut dyn std::io::Write,
) -> Result<(), TranslationError> {
    let release_options = TranslationOptionsBuilder::default_release_build_options()
        .build()
        .expect("Default release build options should always be valid");

    compile_code(source_code, machine_code, &release_options)
}

fn scan_into_memory(source_code: &mut dyn std::io::Read) -> Result<String, TranslationError> {
    let mut source_str = String::new();

    source_code
        .read_to_string(&mut source_str)
        .map_err(|e| TranslationError::ScannerError(e))?;

    Ok(source_str)
}

fn create_lexer<'a>(
    source_str: &'a str,
    fileid: Option<FileId>,
) -> Result<Lexer<'a>, TranslationError> {
    let lexer = Lexer::new(source_str.as_bytes(), fileid);
    lexer.map_err(TranslationError::LexerError)
}

fn parse_language(lexer: Lexer, package_name: &str, log: &CompilerLog) -> Module {
    Parser::new(lexer, log).parse_source(Some(ResolveCtx {
        package_name: package_name.to_string(),
        package_search_paths: Vec::new(),
    }))
}

fn diagnose_problems(
    module: &Module,
    diagnostic_passes: &[Box<dyn Diagnose + Sync>],
    log: &CompilerLog,
    pool: &ThreadPool,
) {
    scope_with(pool, |scope| {
        for diagnostic in diagnostic_passes {
            scope.execute(|| {
                diagnostic.diagnose(module, log);
            });
        }
    });
}

fn generate_code(
    _module: &Module,
    _object: &mut dyn std::io::Write,
) -> Result<(), TranslationError> {
    // TODO: Implement code generation here
    Ok(())
}

pub fn compile_code(
    source_code: &mut dyn std::io::Read,
    machine_code: &mut dyn std::io::Write,
    options: &TranslationOptions,
) -> Result<(), TranslationError> {
    let log = &options.log;
    let source_code = scan_into_memory(source_code)?;

    let fileid = intern_file_id(&options.source_name_for_debug_messages);
    let lexer = create_lexer(&source_code, fileid)?;
    let ast = parse_language(lexer, &options.package_name, log);
    drop(source_code);

    if log.error_bit() {
        return Err(TranslationError::SyntaxError);
    }

    if log.error_bit() {
        return Err(TranslationError::NameResolutionError);
    }

    let pool = ThreadPool::new(options.thread_count.get());
    diagnose_problems(&ast, &options.diagnostic_passes, log, &pool);
    drop(pool);

    if log.error_bit() {
        return Err(TranslationError::DiagnosticError);
    }

    generate_code(&ast, machine_code)
}
