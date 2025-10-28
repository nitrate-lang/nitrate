mod options;
mod options_builder;
mod translate;

pub use nitrate_hir as hir;
pub use nitrate_hir_evaluate as hir_evaluate;
pub use nitrate_hir_from_tree as hir_from_tree;
pub use nitrate_hir_get_type as hir_get_type;
pub use nitrate_hir_resolve_type as hir_resolve_type;
pub use nitrate_hir_validate as hir_validate;
pub use nitrate_token as token;
pub use nitrate_token_lexer as token_lexer;
pub use nitrate_tree as parsetree;
pub use nitrate_tree_parse as parse;

pub use options::TranslationOptions;
pub use options_builder::TranslationOptionsBuilder;

pub use translate::{TranslationError, compile_code, compile_debugable_code, compile_fast_code};
