use nitrate_lexer::*;
use std::io::Read;

fn main() {
    env_logger::Builder::from_default_env()
        .format_timestamp(None)
        .format_level(false)
        .format_target(false)
        .init();

    let filename = "stdin";
    let mut source_code = Vec::new();

    if let Err(e) = std::io::stdin().read_to_end(&mut source_code) {
        eprintln!("Error reading from stdin: {}", e);
        return;
    }

    let mut storage = StringStorage::new();

    let lexer = Lexer::new(&source_code, filename, &mut storage);
    if lexer.is_err() {
        eprintln!("Failed to create lexer");
        return;
    }

    let mut lexer = lexer.unwrap();

    loop {
        let token = lexer.next_token();
        match token.token() {
            Token::Eof => {
                println!("End of file reached.");
                break;
            }
            Token::Illegal => {
                eprintln!("Illegal token encountered: {:?}", token);
                break;
            }
            _ => {
                println!("{:?}", token);
            }
        }
    }
}
