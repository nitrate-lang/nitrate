use nitrate_compiler::lexer::*;
use nitrate_compiler::parser::*;
use nitrate_compiler::parsetree::*;
use slog::{Drain, Logger, Record, o};
use slog_async::Async;
use slog_term::*;
use std::io::Read;
use std::io::Write;

fn read_source_file(filename: &str) -> std::io::Result<Vec<u8>> {
    let mut file = std::fs::File::open(filename)?;
    let mut contents = Vec::new();
    file.read_to_end(&mut contents)?;
    Ok(contents)
}

fn custom_log_header_printer(
    _: &dyn ThreadSafeTimestampFn<Output = std::io::Result<()>>,
    mut rd: &mut dyn RecordDecorator,
    record: &Record,
    use_file_location: bool,
) -> std::io::Result<bool> {
    rd.start_level()?;
    write!(rd, "{}", record.level().as_short_str())?;

    if use_file_location {
        rd.start_whitespace()?;
        write!(rd, " ")?;

        rd.start_location()?;
        write!(
            rd,
            "[{}:{}:{}]:",
            record.location().file,
            record.location().line,
            record.location().column
        )?;
    }

    rd.start_whitespace()?;
    write!(rd, " ")?;

    rd.start_msg()?;
    let mut count_rd = CountingWriter::new(&mut rd);
    write!(count_rd, "{}", record.msg())?;
    Ok(count_rd.count() != 0)
}

fn program() -> i32 {
    env_logger::Builder::from_default_env()
        .format_timestamp(None)
        .format_level(false)
        .format_target(false)
        .init();

    let args: Vec<String> = std::env::args().collect();
    if args.len() < 2 {
        eprintln!("Usage: {} <source_file>", args[0]);
        return 1;
    }

    let filename = &args[1];

    let Ok(source_code) = read_source_file(filename) else {
        eprintln!("Failed to read source file: {}", filename);
        return 1;
    };

    let Ok(lexer) = Lexer::new(&source_code, filename) else {
        eprintln!("Failed to create lexer for file: {}", filename);
        return 1;
    };

    let mut storage = Storage::new();

    let do_parse = || {
        let decorator = TermDecorator::new().build();
        let drain = slog_term::FullFormat::new(decorator)
            .use_custom_header_print(custom_log_header_printer)
            .build()
            .fuse();
        let drain = Async::new(drain.fuse()).build().fuse();
        let mut root_logger = Logger::root(drain, o!());

        let mut parser = Parser::new(lexer, &mut storage, &mut root_logger);

        parser.parse()
    };

    let Some(model) = do_parse() else {
        eprintln!("Failed to parse source code in file: {}", filename);
        return 1;
    };

    if model.any_errors() {
        eprintln!("Parsing completed with errors in file: {}", filename);
        println!("model = {:#?}", model.tree().as_printable(&storage));

        1
    } else {
        println!("Parsing succeeded for file: {}", filename);
        println!("model = {:#?}", model.tree().as_printable(&storage));

        0
    }
}

fn main() {
    let exit_code = program();
    std::process::exit(exit_code);
}
