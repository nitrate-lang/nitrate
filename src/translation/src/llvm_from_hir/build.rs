use std::env;
use std::path::PathBuf;

fn main() {
    let manifest_dir = env::var("CARGO_MANIFEST_DIR").unwrap();
    let base_path = PathBuf::from(manifest_dir);

    let cpp_source_path = base_path.join("src").join("nitrate_extra_llvm_ffi.cpp");

    cc::Build::new()
        .cpp(true)
        .file(cpp_source_path)
        .compile("nitrate_extra_llvm_ffi");

    println!("cargo:rerun-if-changed=src/nitrate_extra_llvm_ffi.cpp");
}
