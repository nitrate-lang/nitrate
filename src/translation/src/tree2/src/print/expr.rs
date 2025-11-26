use crate::{prelude::*, print::item::print_trivia};

impl std::fmt::Display for Expr {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Expr::Trivia { trivia } => print_trivia(trivia, f),

            Expr::Integer {
                source_offset: _,
                trivia,
                value,
            } => {
                print_trivia(trivia, f)?;
                write!(f, "{}", value)
            }

            Expr::Boolean {
                source_offset: _,
                trivia,
                value,
            } => {
                print_trivia(trivia, f)?;
                write!(f, "{}", value)
            }
        }
    }
}
