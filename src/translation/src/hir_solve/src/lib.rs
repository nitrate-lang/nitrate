#![forbid(unsafe_code)]

mod bounds;
mod constraints;
mod diagnosis;
mod monomorphize;
mod range;
mod solve;
mod substitution;

/// Errors that can occur during type inference and solving.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub enum SolveError {
    /// Type errors were detected during inference.
    TypeErrors,
}

impl std::fmt::Display for SolveError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            SolveError::TypeErrors => write!(f, "type errors detected during inference"),
        }
    }
}

pub use solve::{resolve_function, resolve_global};

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn solve_error_display() {
        assert_eq!(
            SolveError::TypeErrors.to_string(),
            "type errors detected during inference"
        );
    }

    #[test]
    fn solve_error_debug() {
        assert_eq!(format!("{:?}", SolveError::TypeErrors), "TypeErrors");
    }
}
