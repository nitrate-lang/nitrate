use crate::Interpreter;
use nitrate_diagnosis::DiagnosticGroupId;
use nitrate_translation::diagnostic_explanations;
use slog::error;
use std::collections::HashMap;

/// Normalize a user-supplied error code: trim whitespace, strip an optional
/// leading `E`, and uppercase. `E27D2`, `e27d2`, and `27D2` all map to `27D2`.
fn normalize_code(code: &str) -> String {
    let code = code.trim();
    let code = code
        .strip_prefix('E')
        .or_else(|| code.strip_prefix('e'))
        .unwrap_or(code);
    code.to_uppercase()
}

/// Build the `code -> explanation` lookup table from every diagnostic group
/// registered by the compilation pipeline.
fn codebook() -> HashMap<String, (&'static str, DiagnosticGroupId)> {
    diagnostic_explanations()
        .iter()
        .map(|entry| (normalize_code(&entry.code()), (entry.explanation, entry.group_id)))
        .collect()
}

impl Interpreter<'_> {
    pub(crate) fn explain_error_code(&self, code: &str) -> anyhow::Result<()> {
        let book = codebook();
        let normalized = normalize_code(code);

        if let Some((explanation, group)) = book.get(&normalized) {
            println!("{} ({})", normalized, group);
            println!("{}", "-".repeat(normalized.len() + group.to_string().len() + 3));
            println!();
            println!("{explanation}");
            Ok(())
        } else {
            error!(self.log, "'{code}' is not a recognized error code.");
            Err(anyhow::anyhow!("Unrecognized error code"))
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::collections::HashSet;

    #[test]
    fn normalize_code_accepts_various_forms() {
        assert_eq!(normalize_code("E27D2"), "27D2");
        assert_eq!(normalize_code("27D2"), "27D2");
        assert_eq!(normalize_code("e27d2"), "27D2");
        assert_eq!(normalize_code(" E2000 "), "2000");
    }

    #[test]
    fn codebook_contains_known_codes() {
        let book = codebook();

        // Parser group (2): variant 2002 (`ExpectedExpr`) -> E27D2.
        assert!(book.contains_key("27D2"), "missing parser `ExpectedExpr` code");

        // Type group (5): variant 0 (`IntegerLiteralOutOfRange`) -> E5000.
        assert!(
            book.contains_key("5000"),
            "missing type `IntegerLiteralOutOfRange` code"
        );

        // BorrowCheck group (7): variant 0x100 (`MutableBorrowOfImmutable`) -> E7100.
        assert!(
            book.contains_key("7100"),
            "missing borrow-check `MutableBorrowOfImmutable` code"
        );

        // HIR group (4): variant 50 (`UnresolvedSymbol`) -> E4032.
        assert!(book.contains_key("4032"), "missing HIR `UnresolvedSymbol` code");

        // Semantic group (6): variant 0x003 (`AssignmentTargetNotMutable`) -> E6003.
        assert!(
            book.contains_key("6003"),
            "missing semantic `AssignmentTargetNotMutable` code"
        );
    }

    #[test]
    fn explanations_map_to_distinct_codes() {
        let mut seen = HashSet::new();
        for entry in diagnostic_explanations() {
            assert!(
                seen.insert(entry.code()),
                "duplicate diagnostic code `{}`",
                entry.code()
            );
        }
    }
}
