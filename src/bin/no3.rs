use nitrate_driver::Interpreter;

fn main() {
    env_logger::Builder::from_default_env()
        .format_timestamp(None)
        .format_level(true)
        .format_target(false)
        .init();

    let args: Vec<String> = std::env::args().collect();

    if Interpreter::default().execute(&args).is_err() {
        std::process::exit(1);
    }
}
