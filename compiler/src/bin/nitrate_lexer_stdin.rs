use nitrate_compiler::lexer::*;
use std::io::Read;

use tracking_allocator::{AllocationGroupId, AllocationRegistry, AllocationTracker, Allocator};

use std::alloc::System;

#[global_allocator]
static GLOBAL: Allocator<System> = Allocator::system();

struct StdoutTracker;

impl AllocationTracker for StdoutTracker {
    fn allocated(
        &self,
        addr: usize,
        object_size: usize,
        _wrapped_size: usize,
        _group_id: AllocationGroupId,
    ) {
        println!("{:016x} A {:016x}", addr, object_size);
    }

    fn deallocated(
        &self,
        addr: usize,
        object_size: usize,
        _wrapped_size: usize,
        _source_group_id: AllocationGroupId,
        _current_group_id: AllocationGroupId,
    ) {
        println!("{:016x} D {:016x}", addr, object_size);
    }
}

fn enable_allocation_tracking() {
    let _ = AllocationRegistry::set_global_tracker(StdoutTracker)
        .expect("no other global tracker should be set yet");

    AllocationRegistry::enable_tracking();
}

fn main() {
    enable_allocation_tracking();

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

    println!("==================================================================");
    println!("Starting to lex the input...");
    println!("==================================================================");

    loop {
        let token = lexer.next_token();
        match token.token() {
            Token::Eof => {
                println!("==================================================================");
                println!("Lexing completed; reached end of file.");
                break;
            }
            Token::Illegal => {
                println!("==================================================================");
                println!("Lexing completed; encountered illegal token: {:?}", token);
                break;
            }
            _ => {
                println!("{:?}", token.token());
            }
        }
    }

    println!("==================================================================");
}
