use std::{fs::OpenOptions, io::Read};

use nitrate_diagnosis::{CompilerLog, intern_file_id};
use nitrate_translation::{
    TranslationError,
    parse::Parser,
    parsetree::{
        kind::{Item, Module, Visibility},
        tag::{PackageNameId, intern_module_name},
    },
    resolve::{ImportContext, ResolveIssue, resolve_imports, resolve_names},
    tokenize::{Lexer, LexerError},
};
use slog::{Drain, Record, o};
use slog_term::{RecordDecorator, ThreadSafeTimestampFn};

#[derive(Debug)]
enum Error {
    NotEnoughArguments,
    OpenInputFileFailed(std::io::Error),
    CreateOutputFileFailed(std::io::Error),
    ParseFailed(TranslationError),
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

fn visibility_filter(item: &Item) -> bool {
    match item {
        Item::SyntaxError(_) => false,
        Item::Impl(_) => true,

        Item::Module(i) => i.visibility == Some(Visibility::Public),
        Item::Import(i) => i.visibility == Some(Visibility::Public),

        Item::TypeAlias(i) => i.read().unwrap().visibility == Some(Visibility::Public),
        Item::Struct(i) => i.read().unwrap().visibility == Some(Visibility::Public),
        Item::Enum(i) => i.read().unwrap().visibility == Some(Visibility::Public),
        Item::Trait(i) => i.read().unwrap().visibility == Some(Visibility::Public),
        Item::Function(i) => i.read().unwrap().visibility == Some(Visibility::Public),
        Item::Variable(i) => i.read().unwrap().visibility == Some(Visibility::Public),
    }
}

fn load_package(name: PackageNameId, log: &CompilerLog) -> Result<Module, ResolveIssue> {
    // FIXME: Correctly resolve module into filepath.

    let module_path = std::path::PathBuf::from(format!("{}.nit", name));

    let source_code = std::fs::read_to_string(&module_path)
        .map_err(|err| ResolveIssue::ImportNotFound((name.clone(), err)))?;

    let file_id = intern_file_id(&module_path.to_string_lossy());
    let lexer = Lexer::new(source_code.as_bytes(), file_id).map_err(|err| match err {
        LexerError::SourceTooBig => ResolveIssue::ImportSourceCodeSizeLimitExceeded(module_path),
    })?;

    let all_items = Parser::new(lexer, log).parse_source();
    drop(source_code);

    let visible_items = all_items
        .into_iter()
        .filter(visibility_filter)
        .collect::<Vec<_>>();

    let module = Module {
        visibility: None,
        attributes: None,
        name: intern_module_name(name.to_string()),
        items: visible_items,
    };

    Ok(module)
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

    let filename = &args[1];
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

    let fileid = intern_file_id(filename);
    let lexer = match Lexer::new(source_code.as_bytes(), fileid) {
        Ok(l) => l,
        Err(e) => {
            return Err(Error::ParseFailed(TranslationError::LexerError(e)));
        }
    };

    let file = OpenOptions::new()
        .create(true)
        .write(true)
        .truncate(true)
        .open(error_filename)
        .unwrap();

    let decorator = slog_term::PlainDecorator::new(file);
    let drain = slog_term::FullFormat::new(decorator)
        .use_custom_header_print(custom_print_msg_header)
        .build()
        .fuse();
    let drain = slog_async::Async::new(drain).build().fuse();
    let log = slog::Logger::root(drain, o!());
    let log = CompilerLog::new(log);

    let mut module = Module {
        attributes: None,
        visibility: None,
        name: intern_module_name("".into()),
        items: Parser::new(lexer, &log).parse_source(),
    };

    let import_context = ImportContext {
        load_package: &load_package,
        this_package_name: None,
    };

    resolve_imports(&import_context, &mut module, &log);
    resolve_names(&mut module, &log);

    if let Err(_) = serde_json::to_writer_pretty(&mut parse_tree_output, &module) {
        return Err(Error::ParseFailed(TranslationError::SyntaxError));
    }

    Ok(())
}

fn main() -> () {
    match program() {
        Ok(()) => return,

        Err(Error::NotEnoughArguments) => {
            eprintln!(
                "Not enough arguments. Usage: nitrate-parse <input-file> <output-ast-json> <output-error-file>"
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
    };

    std::process::exit(1);
}
