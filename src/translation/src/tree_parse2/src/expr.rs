use super::parse::Parser;
use nitrate_tree2::prelude::*;

impl Parser<'_, '_> {
    pub fn parse_expression(&mut self, _leading: Option<Trivia>) -> Option<Expr> {
        // TODO: Implement expression parsing
        todo!()
    }
}
