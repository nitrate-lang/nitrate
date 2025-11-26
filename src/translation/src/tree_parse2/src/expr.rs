use super::parse::Parser;
use nitrate_tree2::prelude::*;

impl Parser<'_, '_> {
    pub fn parse_expression(&mut self) -> Expr {
        match self.lexer.peek().map(|t| &t.token) {
            _ => {
                // TODO: Implement expression parsing
                Expr::Placeholder
            }
        }
    }
}
