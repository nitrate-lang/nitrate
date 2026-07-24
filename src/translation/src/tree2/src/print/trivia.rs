use crate::prelude::*;

impl std::fmt::Display for Trivia {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        for token in self.tokens() {
            write!(f, "{}", token)?;
        }

        Ok(())
    }
}
