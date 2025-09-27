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
use std::{fs::OpenOptions, io::Read};

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

fn is_visible(vis: Option<Visibility>, within: bool) -> bool {
    match vis.unwrap_or(Visibility::Private) {
        Visibility::Public => true,
        Visibility::Protected => within,
        Visibility::Private => false,
    }
}

fn visibility_filter(item: Item, within: bool) -> Option<Item> {
    match item {
        Item::SyntaxError(_) => None,

        Item::Impl(i) => Some(Item::Impl(i)),

        Item::Module(mut node) => {
            if !is_visible(node.visibility, within) {
                return None;
            }

            node.items = node
                .items
                .into_iter()
                .filter_map(|item| visibility_filter(item, within))
                .collect();

            Some(Item::Module(node))
        }

        Item::Import(node) => {
            if !is_visible(node.visibility, within) {
                return None;
            }

            Some(Item::Import(node))
        }

        Item::TypeAlias(node) => {
            if !is_visible(node.read().unwrap().visibility, within) {
                return None;
            }

            Some(Item::TypeAlias(node))
        }

        Item::Struct(node) => {
            let mut lock = node.write().unwrap();

            if !is_visible(lock.visibility, within) {
                return None;
            }

            lock.methods
                .retain(|method| is_visible(method.read().unwrap().visibility, within));

            drop(lock);
            Some(Item::Struct(node))
        }

        Item::Enum(node) => {
            if !is_visible(node.read().unwrap().visibility, within) {
                return None;
            }

            Some(Item::Enum(node))
        }

        Item::Trait(node) => {
            if !is_visible(node.read().unwrap().visibility, within) {
                return None;
            }

            Some(Item::Trait(node))
        }

        Item::Function(node) => {
            if !is_visible(node.read().unwrap().visibility, within) {
                return None;
            }

            Some(Item::Function(node))
        }

        Item::Variable(node) => {
            if !is_visible(node.read().unwrap().visibility, within) {
                return None;
            }

            Some(Item::Variable(node))
        }
    }
}

fn load_package(
    name: PackageNameId,
    log: &CompilerLog,
    _ctx: &ImportContext,
    _source_file_dir: &std::path::Path,
) -> Result<Module, ResolveIssue> {
    // FIXME: Correctly resolve module into filepath.

    // TODO: Determine if the item is in the same package.
    let within = false;

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
        .filter_map(|item| visibility_filter(item, within))
        .collect();

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

    let fileid = intern_file_id(&filename.to_string_lossy());
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
        load_package: &|name, log, ctx| load_package(name, log, ctx, filename),
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
