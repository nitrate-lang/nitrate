use std::io::Read;

use nitrate_diagnosis::DiagnosticCollector;
use nitrate_translation::{TranslationError, parse::Parser, tokenize::Lexer};
use slog::{Drain, o};

#[derive(Debug)]
enum Error {
    NotEnoughArguments,
    OpenInputFileFailed(std::io::Error),
    CreateOutputFileFailed(std::io::Error),
    ParseFailed(TranslationError),
}

fn program() -> Result<(), Error> {
    env_logger::Builder::from_default_env()
        .format_timestamp(None)
        .format_level(true)
        .format_target(false)
        .init();

    let args: Vec<String> = std::env::args().collect();
    if args.len() < 3 {
        return Err(Error::NotEnoughArguments);
    }

    let filename = &args[1];
    let target_filename = &args[2];

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

    let lexer = match Lexer::new(source_code.as_bytes(), filename.as_str()) {
        Ok(l) => l,
        Err(e) => {
            return Err(Error::ParseFailed(TranslationError::LexerError(e)));
        }
    };

    let decorator = slog_term::TermDecorator::new().build();
    let drain = slog_term::FullFormat::new(decorator).build().fuse();
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
            eprintln!("Not enough arguments. Usage: nitrate-parse <input-file> <output-file>");
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
