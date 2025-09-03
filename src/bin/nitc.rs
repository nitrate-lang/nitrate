use log::error;
use nitrate_compiler::evaluate::AbstractMachine;
use nitrate_compiler::lexical::*;
use nitrate_compiler::parsetree::Builder;
use nitrate_compiler::syntax::*;
use std::io::Read;

fn read_source_file(filename: &str) -> std::io::Result<Vec<u8>> {
    let mut file = std::fs::File::open(filename)?;
    let mut contents = Vec::new();
    file.read_to_end(&mut contents)?;
    Ok(contents)
}

fn program() -> i32 {
    env_logger::Builder::from_default_env()
        .format_timestamp(None)
        .format_level(true)
        .format_target(false)
        .init();

    // let args: Vec<String> = std::env::args().collect();
    // if args.len() < 2 {
    //     eprintln!("Usage: {} <source_file>", args[0]);
    //     return 1;
    // }

    // let filename = &args[1];

    // let Ok(source_code) = read_source_file(filename) else {
    //     eprintln!("Failed to read source file: {}", filename);
    //     return 1;
    // };

    // let Ok(lexer) = Lexer::new(&source_code, filename) else {
    //     eprintln!("Failed to create lexer for file: {}", filename);
    //     return 1;
    // };

    // let mut symbol_table = SymbolTable::default();
    // let mut parser = Parser::new(lexer, &mut symbol_table);

    // let Some(model) = parser.parse() else {
    //     eprintln!("Failed to parse source code in file: {}", filename);
    //     return 1;
    // };

    // if model.any_errors() {
    //     error!("Compilation failed: {}", filename);
    //     return 1;
    // }

    let ast = Builder::create_block()
        .add_statement(
            Builder::create_call()
                .with_callee_name("std::intrinsic::print")
                .add_argument(
                    Some("message"),
                    Builder::create_string_from_ref("Hello, World!\n"),
                )
                .build(),
        )
        .add_element(
            Builder::create_call()
                .with_callee_name("std::intrinsic::print")
                .add_argument(
                    Some("message"),
                    Builder::create_string_from_ref("Hello, World again!\n"),
                )
                .build(),
        )
        .build();

    let result = match AbstractMachine::new().evaluate(&ast) {
        Ok(result) => result,

        Err(e) => {
            // error!("Evaluation failed for file: {}", filename);
            error!("Error: {:?}", e);
            return 1;
        }
    };

    println!("result = {:#?}", result);

    0
}

fn main() {
    let exit_code = program();
    std::process::exit(exit_code);
}
