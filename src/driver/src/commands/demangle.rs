use crate::Interpreter;
use clap::Parser;
use nitrate_translation::hir::Store;
use nitrate_translation::hir_dump::Dump;
use nitrate_translation::hir_mangle::{demangle_module, demangle_name};
use slog::error;

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub(crate) struct DemangleArgs {
    /// Mangled Nitrate symbol name to demangle
    #[arg(value_name = "SYMBOL")]
    pub(crate) symbol: String,
}

impl Interpreter<'_> {
    pub(crate) fn sc_demangle(&mut self, args: DemangleArgs) -> anyhow::Result<()> {
        let symbol = args.symbol;
        let log = self.log;

        // Demangling reconstructs `Type` values whose `TypeId` handles are
        // interned through the HIR store, so the decode runs inside HIR TLS
        // storage.
        let hir_store = Store::new();
        nitrate_translation::hir::using_storage(&hir_store, || {
            // A full symbol demangles to (package, name, type).
            if let Ok((package, name, ty)) = demangle_name(&symbol) {
                println!("package: {package}");
                println!("name: {name}");
                println!("type: {}", ty.to_string());
                return Ok(());
            }

            // A module name is a single mangled string: `_N<module>`.
            if let Ok(module) = demangle_module(&symbol) {
                println!("module: {module}");
                return Ok(());
            }

            error!(
                log,
                "'{symbol}' is not a valid Nitrate-mangled symbol (expected a `_N` prefix)"
            );
            Err(anyhow::anyhow!("invalid mangled symbol"))
        })
    }
}
