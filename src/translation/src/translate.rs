use crate::{TranslationOptions, TranslationOptionsBuilder, options::Diagnose};
use nitrate_codegen::{Codegen, CodegenError};
use nitrate_diagnosis::{DiagnosticCollector, FileId, get_or_create_file_id};
use nitrate_parse::Parser;
use nitrate_parsetree::{
    kind::{Module, Package, Visibility},
    tag::{intern_module_name, intern_package_name},
};
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
    fileid: Option<FileId>,
) -> Result<Lexer<'a>, TranslationError> {
    let lexer = Lexer::new(source_str.as_bytes(), fileid);
    lexer.map_err(TranslationError::LexerError)
}

fn parse_language(lexer: Lexer, package_name: &str, bugs: &DiagnosticCollector) -> Package {
    let mut parser = Parser::new(lexer, bugs);
    let items = parser.parse_source();

    Package {
        name: intern_package_name(package_name.to_string()),
        root: Module {
            attributes: None,
            name: intern_module_name("".into()),
            visibility: Some(Visibility::Public),
            items,
        },
    }
}

// fn resolve_names(_program: &mut Expr) -> Result<(), TranslationError> {
//     // TODO: Implement name resolution logic
//     Ok(())
// }

// fn type_check(_program: &mut Expr) -> Result<(), TranslationError> {
//     // TODO: Implement type checking logic
//     Ok(())
// }

fn diagnose_problems(
    package: &Package,
    diagnostic_passes: &[Box<dyn Diagnose + Sync>],
    bugs: &DiagnosticCollector,
    pool: &ThreadPool,
) {
    scope_with(pool, |scope| {
        for diagnostic in diagnostic_passes {
            scope.execute(|| {
                diagnostic.diagnose(package, bugs);
            });
        }
    });
}

// fn optimize_functions(
//     symbols: &mut SymbolTable,
//     function_optimization_passes: &[Box<dyn FunctionOptimization + Sync>],
//     bugs: &DiagnosticCollector,
//     pool: &ThreadPool,
// ) {
//     scope_with(pool, |scope| {
//         for function_mut in symbols.function_iter_mut() {
//             // We can't optimize function declarations
//             if function_mut.is_declaration() {
//                 continue;
//             }

//             // The RwLock race condition checking if the function is a declaration
//             // is fine, because optimization passes will check it internally
//             // and be a no-op.

//             scope.execute(|| {
//                 for pass in function_optimization_passes {
//                     pass.optimize_function(function_mut, bugs);
//                 }
//             });
//         }
//     });
// }

fn generate_code(
    package: &Package,
    object: &mut dyn std::io::Write,
) -> Result<(), TranslationError> {
    let target_triple_string = "x86_64"; // Example target ISA
    let isa_config = HashMap::new();

    let codegen = Codegen::new(target_triple_string.to_string(), isa_config);

    codegen
        .generate(package, object)
        .map_err(TranslationError::CodegenError)
}

pub fn compile_code(
    source_code: &mut dyn std::io::Read,
    machine_code: &mut dyn std::io::Write,
    options: &TranslationOptions,
) -> Result<(), TranslationError> {
    let bugs = &options.bugs;
    let source = scan_into_memory(source_code)?;

    let fileid = get_or_create_file_id(&options.source_name_for_debug_messages);
    let lexer = create_lexer(&source, fileid)?;

    let package = parse_language(lexer, &options.package_name, bugs);

    // resolve_names(&mut program, &mut symtab)?;
    // type_check(&mut program)?;

    let pool = ThreadPool::new(options.thread_count.get());

    diagnose_problems(&package, &options.diagnostic_passes, bugs, &pool);

    if bugs.error_bit() {
        return Err(TranslationError::DiagnosticError);
    }

    // optimize_functions(&mut symtab, &options.function_optimizations, bugs, &pool);
    drop(pool);

    generate_code(&package, machine_code)
}
