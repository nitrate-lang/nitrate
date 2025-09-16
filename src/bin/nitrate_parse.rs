use std::{fs::OpenOptions, io::Read};

use nitrate_diagnosis::{DiagnosticCollector, get_or_create_file_id};
use nitrate_translation::{TranslationError, parse::Parser, tokenize::Lexer};
use slog::{Drain, Record, o};
use slog_term::{RecordDecorator, ThreadSafeTimestampFn};

#[derive(Debug)]
enum Error {
    NotEnoughArguments,
    OpenInputFileFailed(std::io::Error),
    CreateOutputFileFailed(std::io::Error),
    ParseFailed(TranslationError),
}

pub fn custom_print_msg_header(
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

    let fileid = get_or_create_file_id(filename);

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
    let bugs = DiagnosticCollector::new(log);

    let package = Parser::new(lexer, &bugs).parse_crate(filename.to_owned().into());

    if let Err(_) = serde_json::to_writer_pretty(&mut parse_tree_output, &package) {
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
