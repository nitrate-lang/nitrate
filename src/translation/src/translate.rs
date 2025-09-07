use crate::{TranslationOptions, TranslationOptionsBuilder};
use nitrate_parse::{Parser, SymbolTable};
use nitrate_structure::SourceModel;
use nitrate_tokenize::Lexer;
use threadpool::ThreadPool;
use threadpool_scope::scope_with;

#[derive(Debug, PartialEq, Eq, PartialOrd, Ord)]
pub enum TranslationError {
    ScannerError,
    LexerError(nitrate_tokenize::LexerError),
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
        .map_err(|_| TranslationError::ScannerError)?;

    Ok(source_str)
}

fn create_lexer<'a>(
    source_str: &'a str,
    source_name_for_debug_messages: &'a str,
) -> Result<Lexer<'a>, TranslationError> {
    let lexer = Lexer::new(source_str.as_bytes(), source_name_for_debug_messages);
    lexer.map_err(TranslationError::LexerError)
}

fn parse_language(lexer: Lexer) -> Result<(SourceModel, SymbolTable), TranslationError> {
    let mut symbol_table = SymbolTable::default();

    let mut parser = Parser::new(lexer, &mut symbol_table);
    let model = parser.parse().ok_or(TranslationError::SyntaxError)?;

    Ok((model, symbol_table))
}

pub fn compile_code(
    source_code: &mut dyn std::io::Read,
    _machine_code: &mut dyn std::io::Write,
    options: &TranslationOptions,
) -> Result<(), TranslationError> {
    let source = scan_into_memory(source_code)?;
    let lexer = create_lexer(&source, &options.source_name_for_debug_messages)?;

    let (model, _symbol_table) = parse_language(lexer)?;
    if model.any_errors() {
        return Err(TranslationError::SyntaxError);
    }

    let _parse_tree = model.tree();

    // TODO: Perform name resolution
    // TODO: Perform type checking

    let pool = ThreadPool::new(32);

    scope_with(&pool, |scope| {
        for diagnostic in &options.diagnostic_passes {
            scope.execute(|| {
                diagnostic.diagnose(&model, &options.drain);
            });
        }
    });

    // TODO: Perform optimizations
    // TODO: Generate code

    Err(TranslationError::NameResolutionError)
}
