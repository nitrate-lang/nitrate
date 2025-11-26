#[cfg(test)]
mod tests {
    use crate::*;
    use nitrate_diagnosis::CompilerLog;
    use nitrate_token_lexer::Lexer;
    use nitrate_tree2::{GlobalSource, Store, using_source, using_storage};

    pub(crate) fn parse_correct_source_code(source: &[u8]) {
        using_source(
            &GlobalSource {
                full_source: source,
                fileid: None,
            },
            || {
                using_storage(&Store::new(), || {
                    let log = CompilerLog::default_stderr();
                    let lexer = Lexer::new(source, None).expect("Failed to create lexer");

                    let mut parser = Parser::new(lexer, &log);
                    let root = parser.parse_source();
                    assert!(!log.error_bit(), "Parsing failed with errors");

                    assert_eq!(
                        root.to_string().as_bytes(),
                        source,
                        "Parsed source does not match original"
                    );
                });
            },
        )
    }

    #[test]
    fn test_parse_module() {
        parse_correct_source_code(include_bytes!("programs/item/module.nit"));
    }

    // TODO: Test item parsing
}
