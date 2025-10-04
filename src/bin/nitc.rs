use nitrate_translation::{TranslationError, TranslationOptionsBuilder, compile_code};

#[derive(Debug)]
enum Error {
    NotEnoughArguments,
    OpenInputFileFailed(std::io::Error),
    CreateOutputFileFailed(std::io::Error),
    CompilationFailed(TranslationError),
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

    let mut machine_code = match std::fs::File::create(target_filename) {
        Ok(f) => f,
        Err(e) => {
            return Err(Error::CreateOutputFileFailed(e));
        }
    };

    if let Err(error) = compile_code(
        &mut file,
        &mut machine_code,
        &TranslationOptionsBuilder::default_debug_build_options()
            .build()
            .unwrap(),
    ) {
        return Err(Error::CompilationFailed(error));
    }

    Ok(())
}

fn main() -> () {
    match program() {
        Ok(()) => {}

        Err(Error::NotEnoughArguments) => {
            eprintln!("Not enough arguments. Usage: nitc <input-file> <output-file>");
            std::process::exit(1);
        }
        Err(Error::OpenInputFileFailed(io)) => {
            eprintln!("Failed to open input file: {}", io);
            std::process::exit(1);
        }
        Err(Error::CreateOutputFileFailed(io)) => {
            eprintln!("Failed to create output file: {}", io);
            std::process::exit(1);
        }
        Err(Error::CompilationFailed(error)) => {
            eprintln!("Compilation failed: {:?}", error);
            std::process::exit(1);
        }
    }
}
