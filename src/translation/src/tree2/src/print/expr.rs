use crate::{prelude::*, print::item::print_trivia};

impl std::fmt::Display for Expr {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Expr::Trivia { trivia } => print_trivia(trivia, f),

            Expr::Unit {
                source_offset: _,
                trivia,
            } => {
                print_trivia(&trivia[0], f)?;
                write!(f, "(")?;
                print_trivia(&trivia[1], f)?;
                write!(f, ")")
            }

            Expr::Boolean {
                source_offset: _,
                trivia,
                value,
            } => {
                print_trivia(trivia, f)?;
                write!(f, "{}", value)
            }

            Expr::Integer {
                source_offset: _,
                trivia,
                value: _,
                raw_value_str: value_str,
                suffix: _,
            } => {
                print_trivia(trivia, f)?;
                write!(f, "{}", value_str)
            }

            Expr::Float {
                source_offset: _,
                trivia,
                value: _,
                raw_value_str: value_str,
                suffix: _,
            } => {
                print_trivia(trivia, f)?;
                write!(f, "{}", value_str)
            }

            Expr::String {
                source_offset: _,
                trivia,
                value: _,
                raw_value_str: value_str,
            } => {
                print_trivia(trivia, f)?;
                write!(f, "{}", value_str)
            }

            Expr::BString {
                source_offset: _,
                trivia,
                value: _,
                raw_value_str: value_str,
            } => {
                print_trivia(trivia, f)?;
                write!(f, "{}", value_str)
            }

            Expr::StructInit {
                source_offset: _,
                trivia,
                struct_name,
                fields,
            } => {
                print_trivia(&trivia[0], f)?;
                write!(f, "{}", struct_name)?;
                print_trivia(&trivia[1], f)?;
                write!(f, "{{")?;

                for field in fields {
                    print_trivia(&field.trivia[0], f)?;
                    write!(f, "{}", field.name)?;
                    print_trivia(&field.trivia[1], f)?;
                    write!(f, ":")?;
                    write!(f, "{}", field.value.borrow())?;
                    print_trivia(&field.trivia[2], f)?;

                    if field.ends_with_comma {
                        write!(f, ",")?;
                    }
                }

                write!(f, "}}")
            }
        }
    }
}
