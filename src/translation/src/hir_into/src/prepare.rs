use crate::{ExprPrep, TypePrep};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::hir::{HirCtx, Type, Value};
use nitrate_parse::Parser;
use nitrate_tokenize::{Lexer, LexerError};

pub fn from_nitrate_expression(ctx: &mut HirCtx, nitrate_expr: &str) -> Result<Value, ()> {
    let lexer = match Lexer::new(nitrate_expr.as_bytes(), None) {
        Ok(lexer) => lexer,
        Err(LexerError::SourceTooBig) => return Err(()),
    };

    let trash = CompilerLog::default();
    let mut parser = Parser::new(lexer, &trash);

    let expression = parser.parse_expression();
    if trash.error_bit() {
        return Err(());
    }

    let hir_value = ExprPrep::new(expression, &trash, ctx).try_into()?;
    if trash.error_bit() {
        return Err(());
    }

    Ok(hir_value)
}

pub fn from_nitrate_type(ctx: &mut HirCtx, nitrate_type: &str) -> Result<Type, ()> {
    let lexer = match Lexer::new(nitrate_type.as_bytes(), None) {
        Ok(lexer) => lexer,
        Err(LexerError::SourceTooBig) => return Err(()),
    };

    let trash = CompilerLog::default();
    let mut parser = Parser::new(lexer, &trash);

    let expression = parser.parse_type();
    if trash.error_bit() {
        return Err(());
    }

    let hir_type = TypePrep::new(expression, &trash, ctx).try_into()?;
    if trash.error_bit() {
        return Err(());
    }

    Ok(hir_type)
}
