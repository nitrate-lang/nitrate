use nitrate_diagnosis::{CompilerLog, intern_file_id};
use nitrate_translation::{
    TranslationError,
    hir::{
        Dump, DumpContext,
        hir::{HirCtx, PtrSize},
    },
    hir_into::ast_mod2hir,
    parse::Parser,
    parsetree::ast,
    resolve::{ImportContext, resolve_imports, resolve_names},
    token_lexer::Lexer,
};

use slog::{Drain, Record, o};
use slog_term::{RecordDecorator, ThreadSafeTimestampFn};
use std::{
    fs::OpenOptions,
    io::{Read, Write},
};

#[derive(Debug)]
enum Error {
    NotEnoughArguments,
    OpenInputFileFailed(std::io::Error),
    CreateOutputFileFailed(std::io::Error),
    ParseFailed(TranslationError),
    HirError,
}

fn custom_print_msg_header(
    _fn_timestamp: &dyn ThreadSafeTimestampFn<Output = std::io::Result<()>>,
    rd: &mut dyn RecordDecorator,
    record: &Record,
    _use_file_location: bool,
) -> std::io::Result<bool> {
    rd.start_msg()?;
    write!(rd, "{}", record.msg())?;

    Ok(true)
}

fn program() -> Result<(), Error> {
    env_logger::Builder::from_default_env()
        .format_timestamp(None)
        .format_level(true)
        .format_target(false)
        .init();

    let args: Vec<String> = std::env::args().collect();
    if args.len() < 4 {
        return Err(Error::NotEnoughArguments);
    }

    let filename = std::path::Path::new(&args[1]);
    let target_filename = &args[2];
    let error_filename = &args[3];

    let mut file = match std::fs::File::open(filename) {
        Ok(f) => f,
        Err(e) => {
            return Err(Error::OpenInputFileFailed(e));
        }
    };

    let mut source_code = String::new();
    if let Err(e) = file.read_to_string(&mut source_code) {
        return Err(Error::OpenInputFileFailed(e));
    }

    let mut parse_tree_output = match std::fs::File::create(target_filename) {
        Ok(f) => f,
        Err(e) => {
            return Err(Error::CreateOutputFileFailed(e));
        }
    };

    let lexer = match Lexer::new(
        source_code.as_bytes(),
        intern_file_id(&filename.to_string_lossy()),
    ) {
        Ok(l) => l,
        Err(e) => {
            return Err(Error::ParseFailed(TranslationError::LexerError(e)));
        }
    };

    let diagnostic_output_file = OpenOptions::new()
        .create(true)
        .write(true)
        .truncate(true)
        .open(error_filename)
        .unwrap();

    let decorator = slog_term::PlainDecorator::new(diagnostic_output_file);
    let drain = slog_term::FullFormat::new(decorator)
        .use_custom_header_print(custom_print_msg_header)
        .build()
        .fuse();
    let drain = slog_async::Async::new(drain).build().fuse();
    let log = slog::Logger::root(drain, o!());
    let log = CompilerLog::new(log);

    let mut module = ast::Module {
        attributes: None,
        visibility: None,
        name: None,
        items: Parser::new(lexer, &log).parse_source(),
    };

    let import_context = ImportContext::new(filename.to_path_buf());
    resolve_imports(&import_context, &mut module, &log);
    resolve_names(&mut module, &log);

    let mut hir_ctx = HirCtx::new(PtrSize::U64);
    let Ok(hir_module) = ast_mod2hir(module, &mut hir_ctx, &log) else {
        return Err(Error::HirError);
    };

    let pretty_printed = hir_module.dump_to_string(&mut DumpContext::new(&hir_ctx.store()));

    parse_tree_output
        .write_all(&pretty_printed.into_bytes())
        .unwrap();

    drop(parse_tree_output);

    Ok(())
}

fn main() -> () {
    match program() {
        Ok(()) => return,

        Err(Error::NotEnoughArguments) => {
            eprintln!(
                "Not enough arguments. Usage: nitrate-parse <input-file> <output-hir> <output-error-file>"
            );
        }
        Err(Error::OpenInputFileFailed(io)) => {
            eprintln!("Failed to open input file: {}", io);
        }
        Err(Error::CreateOutputFileFailed(io)) => {
            eprintln!("Failed to create output file: {}", io);
        }
        Err(Error::ParseFailed(error)) => {
            eprintln!("Parsing failed: {:?}", error);
        }
        Err(Error::HirError) => {
            eprintln!("Failed to convert parse tree into HIR");
        }
    };

    std::process::exit(1);
}
