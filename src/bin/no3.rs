use nitrate_driver::Interpreter;
use slog::{Drain, Record, o};
use slog_term::{FullFormat, RecordDecorator, TermDecorator, ThreadSafeTimestampFn};
use std::process::ExitCode;

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

fn main() -> ExitCode {
    env_logger::init();

    let args: Vec<String> = std::env::args().collect();

    let decorator = TermDecorator::new().build();
    let drain = FullFormat::new(decorator)
        .use_custom_header_print(custom_print_msg_header)
        .build()
        .fuse();
    let drain = slog_async::Async::new(drain)
        .build()
        .filter_level(match log::max_level() {
            log::LevelFilter::Error => slog::Level::Error,
            log::LevelFilter::Warn => slog::Level::Warning,
            log::LevelFilter::Info => slog::Level::Info,
            log::LevelFilter::Debug => slog::Level::Debug,
            log::LevelFilter::Trace => slog::Level::Trace,
            log::LevelFilter::Off => slog::Level::Critical,
        })
        .fuse();
    let log = slog::Logger::root(drain, o!());

    let mut cli = Interpreter::new(&log);
    match cli.run(&args) {
        Ok(()) => ExitCode::SUCCESS,
        Err(_) => ExitCode::FAILURE,
    }
}
