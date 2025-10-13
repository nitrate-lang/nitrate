use crate::TryIntoHir;
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::hir::{HirCtx, Type, Value};
use nitrate_parse::Parser;
use nitrate_tokenize::{Lexer, LexerError};

pub fn from_nitrate_expression(
    nitrate_expr: &str,
    ctx: &mut HirCtx,
    log: &CompilerLog,
) -> Result<Value, ()> {
    let lexer = match Lexer::new(nitrate_expr.as_bytes(), None) {
        Ok(lexer) => lexer,
        Err(LexerError::SourceTooBig) => return Err(()),
    };

    let tmp_log = CompilerLog::default();
    let mut parser = Parser::new(lexer, &tmp_log);

    let expression = parser.parse_expression();
    if tmp_log.error_bit() {
        return Err(());
    }

    let hir_value = expression.try_into_hir(ctx, log)?;
    if log.error_bit() {
        return Err(());
    }

    Ok(hir_value)
}

pub fn from_nitrate_type(
    nitrate_type: &str,
    ctx: &mut HirCtx,
    log: &CompilerLog,
) -> Result<Type, ()> {
    let lexer = match Lexer::new(nitrate_type.as_bytes(), None) {
        Ok(lexer) => lexer,
        Err(LexerError::SourceTooBig) => return Err(()),
    };

    let tmp_log = CompilerLog::default();
    let mut parser = Parser::new(lexer, &tmp_log);

    let expression = parser.parse_type();
    if tmp_log.error_bit() {
        return Err(());
    }

    let hir_value = expression.try_into_hir(ctx, log)?;
    if log.error_bit() {
        return Err(());
    }

    Ok(hir_value)
}
