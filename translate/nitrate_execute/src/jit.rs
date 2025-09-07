use nitrate_evaluate::AbstractMachine;
use nitrate_tokenize::*;
use nitrate_parse::*;
use nitrate_structure::kind::Expr;

pub enum JitError {
    LexicalError,
    ParseError,
    CompilationError,
    ExecutionError,
}

#[derive(Debug, Default)]
pub struct JitCompiler {}

impl JitCompiler {
    pub fn new() -> Self {
        JitCompiler {}
    }

    pub fn execute_raw<'a>(
        &self,
        source_code: &'a [u8],
        debug_source_path: Option<&'a str>,
    ) -> Result<Expr<'a>, JitError> {
        let Ok(lexer) = Lexer::new(source_code, debug_source_path.unwrap_or_default()) else {
            return Err(JitError::LexicalError);
        };

        let mut symbol_table = SymbolTable::default();
        let mut parser = Parser::new(lexer, &mut symbol_table);

        let Some(model) = parser.parse() else {
            return Err(JitError::ParseError);
        };

        if model.any_errors() {
            return Err(JitError::ParseError);
        }

        // TODO: Invoke nitrate_resolve crate here
        // TODO: Run nitrate_optimize crate here

        AbstractMachine::new()
            .evaluate(&model.tree())
            .map_err(|_| JitError::ExecutionError)
    }

    pub fn execute<'a>(
        &self,
        source_code: &'a str,
        debug_source_path: Option<&'a str>,
    ) -> Result<Expr<'a>, JitError> {
        self.execute_raw(source_code.as_bytes(), debug_source_path)
    }
}
