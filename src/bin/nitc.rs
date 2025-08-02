use nitrate_compiler::lexer::*;
use nitrate_compiler::parser::*;
use nitrate_compiler::parsetree::*;
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

fn read_source_file(filename: &str) -> std::io::Result<Vec<u8>> {
    let mut file = std::fs::File::open(filename)?;
    let mut contents = Vec::new();
    file.read_to_end(&mut contents)?;
    Ok(contents)
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    enable_allocation_tracking();

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

    let mut storage = Storage::default();
    let mut parser = Parser::new(&mut lexer, &mut storage);

    println!("===========================================");

    {
        let _parsetree = parser.parse().map_or_else(
            || Err(format!("Failed to parse source code in file {}", filename)),
            |tree| Ok(tree),
        )?;
    }

    println!("===========================================");

    println!("Successfully parsed file: {}", filename);

    Ok(())
}
