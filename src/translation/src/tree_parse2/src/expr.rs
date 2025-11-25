use super::parse::Parser;
use nitrate_tree2::prelude::*;

impl Parser<'_, '_> {
    pub fn parse_expression(&mut self) -> Option<Expr> {
        match self.lexer.peek()?.token {
            _ => {
                // TODO: Implement expression parsing
                None
            }
        }
    }
}
