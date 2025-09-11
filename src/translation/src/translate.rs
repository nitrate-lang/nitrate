use crate::{TranslationOptions, TranslationOptionsBuilder};
use nitrate_codegen::{Codegen, CodegenError};
use nitrate_diagnosis::{Diagnose, DiagnosticDrain};
use nitrate_optimization::FunctionOptimization;
use nitrate_parse::{Parser, SymbolTable};
use nitrate_structure::SourceModel;
use nitrate_tokenize::Lexer;
use std::collections::HashMap;
use threadpool::ThreadPool;
use threadpool_scope::scope_with;

#[derive(Debug)]
pub enum TranslationError {
    ScannerError(std::io::Error),
    LexerError(nitrate_tokenize::LexerError),
    SyntaxError,
    NameResolutionError,
    TypeCheckingError,
    DiagnosticError,
    CodegenError(CodegenError),
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

fn resolve_names(
    _model: &mut SourceModel,
    _symbol_table: &mut SymbolTable,
) -> Result<(), TranslationError> {
    // TODO: Implement name resolution logic
    Ok(())
}

fn type_check(_model: &mut SourceModel) -> Result<(), TranslationError> {
    // TODO: Implement type checking logic
    Ok(())
}

fn diagnose_problems(
    model: &SourceModel,
    diagnostic_passes: &[Box<dyn Diagnose + Sync>],
    drain: &DiagnosticDrain,
    pool: &ThreadPool,
) {
    scope_with(pool, |scope| {
        for diagnostic in diagnostic_passes {
            scope.execute(|| {
                diagnostic.diagnose(model, drain);
            });
        }
    });
}

fn optimize_functions(
    symbols: &mut SymbolTable,
    function_optimization_passes: &[Box<dyn FunctionOptimization + Sync>],
    drain: &DiagnosticDrain,
    pool: &ThreadPool,
) {
    scope_with(pool, |scope| {
        for function in symbols.function_iter_mut() {
            // We can't optimize function declarations
            if function.read().unwrap().is_declaration() {
                continue;
            }

            // The RwLock race condition checking if the function is a declaration
            // is fine, because optimization passes will check it internally
            // and be a no-op.

            scope.execute(|| {
                let mut function_mut = function.write().unwrap();

                for pass in function_optimization_passes {
                    pass.optimize_function(&mut function_mut, drain);
                }
            });
        }
    });
}

fn generate_code(
    model: &SourceModel,
    object: &mut dyn std::io::Write,
) -> Result<(), TranslationError> {
    let target_triple_string = "x86_64"; // Example target ISA
    let isa_config = HashMap::new();

    let codegen = Codegen::new(target_triple_string.to_string(), isa_config);

    codegen
        .generate(model, object)
        .map_err(TranslationError::CodegenError)
}

pub fn compile_code(
    source_code: &mut dyn std::io::Read,
    machine_code: &mut dyn std::io::Write,
    options: &TranslationOptions,
) -> Result<(), TranslationError> {
    let source = scan_into_memory(source_code)?;
    let lexer = create_lexer(&source, &options.source_name_for_debug_messages)?;

    let (mut model, mut symtab) = parse_language(lexer)?;
    if model.any_errors() {
        return Err(TranslationError::SyntaxError);
    }

    resolve_names(&mut model, &mut symtab)?;
    type_check(&mut model)?;

    let pool = ThreadPool::new(options.thread_count.get());
    let drain = &options.drain;

    diagnose_problems(&model, &options.diagnostic_passes, drain, &pool);

    if drain.any_errors() {
        return Err(TranslationError::DiagnosticError);
    }

    optimize_functions(&mut symtab, &options.function_optimizations, drain, &pool);
    drop(pool);

    generate_code(&model, machine_code)
}
