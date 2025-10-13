mod options;
mod options_builder;
mod translate;

pub use nitrate_hir as hir;
pub use nitrate_hir_evaluate as hir_evaluate;
pub use nitrate_hir_get_type as hir_get_type;
pub use nitrate_hir_into as hir_into;
pub use nitrate_hir_meta as hir_meta;
pub use nitrate_hir_resolve_type as hir_resolve_type;
pub use nitrate_parse as parse;
pub use nitrate_parsetree as parsetree;
pub use nitrate_resolve as resolve;
pub use nitrate_tokenize as tokenize;

pub use options::TranslationOptions;
pub use options_builder::TranslationOptionsBuilder;

pub use translate::{TranslationError, compile_code, compile_debugable_code, compile_fast_code};
