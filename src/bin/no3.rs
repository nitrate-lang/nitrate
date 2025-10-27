use nitrate_driver::Interpreter;
use slog::{Drain, Record, o};
use slog_term::{FullFormat, RecordDecorator, TermDecorator, ThreadSafeTimestampFn};
use std::io::Error;

fn custom_print_msg_header(
    _fn_timestamp: &dyn ThreadSafeTimestampFn<Output = std::io::Result<()>>,
    rd: &mut dyn RecordDecorator,
    record: &Record,
    _use_file_location: bool,
) -> std::io::Result<bool> {
    rd.start_level()?;
    write!(rd, "{}", record.level())?;

    rd.start_separator()?;
    write!(rd, " ")?;

    rd.start_msg()?;
    write!(rd, "{}", record.msg())?;
    Ok(true)
}

fn main() -> Result<(), Error> {
    let args: Vec<String> = std::env::args().collect();

    let decorator = TermDecorator::new().build();
    let drain = FullFormat::new(decorator)
        .use_custom_header_print(custom_print_msg_header)
        .build()
        .fuse();
    let drain = slog_async::Async::new(drain).build().fuse();
    let log = slog::Logger::root(drain, o!());

    let mut cli = Interpreter::new(&log);
    match cli.run(&args) {
        Ok(()) => Ok(()),
        Err(_) => std::process::exit(1),
    }
}
