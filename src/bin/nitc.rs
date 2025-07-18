use nitrate_compiler::lexer::*;
use nitrate_compiler::parser::*;
use std::io::Read;

fn read_source_file(filename: &str) -> std::io::Result<Vec<u8>> {
    let mut file = std::fs::File::open(filename)?;
    let mut contents = Vec::new();
    file.read_to_end(&mut contents)?;
    Ok(contents)
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    env_logger::Builder::from_default_env()
        .format_timestamp(None)
        .format_level(false)
        .format_target(false)
        .init();

    let args: Vec<String> = std::env::args().collect();
    if args.len() < 2 {
        eprintln!("Usage: {} <source_file>", args[0]);
        std::process::exit(1);
    }

    let filename = &args[1];

    let source_code = read_source_file(filename)
        .map_err(|e| format!("Failed to read source file {}: {}", filename, e))?;

    let mut lexer = Lexer::new(&source_code, filename)
        .map_err(|e| format!("Failed to create lexer for file {}: {}", filename, e))?;

    // println!("=====================================================");
    // loop {
    //     let token = lexer.next_token();
    //     match token.token() {
    //         Token::Eof => {
    //             println!("=====================================================");
    //             println!("End of file");
    //             break;
    //         }
    //         Token::Illegal => {
    //             println!("=====================================================");
    //             eprintln!("Illegal token found: {:?}", token);
    //             break;
    //         }
    //         _ => {
    //             println!("{:?}", token.token());
    //         }
    //     }
    // }

    let mut parser = Parser::new(&mut lexer)
        .map_err(|e| format!("Failed to create parser for file {}: {}", filename, e))?;

    let _parsetree = parser.parse().map_or(
        Err(format!("Failed to parse source code in file {}", filename)),
        |tree| Ok(tree),
    )?;

    println!("Successfully parsed file: {}", filename);

    Ok(()) // Indicate success
}
