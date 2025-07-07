use nitrate_lexer::*;
use std::io::Read;

fn main() {
    let filename = "stdin";
    let mut source_code = String::new();

    match std::io::stdin().read_to_string(&mut source_code) {
        Err(e) => {
            eprintln!("Error reading from stdin: {}", e);
            return;
        }
        Ok(_) => {}
    }

    let mut lexer = Lexer::new(&source_code.as_bytes(), &filename);
    match &mut lexer {
        Err(e) => {
            eprintln!("Failed to create lexer: {:?}", e);
            return;
        }
        Ok(lexer) => loop {
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
        },
    }
}
