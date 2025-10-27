use crate::{Interpreter, InterpreterError};
use json_comments::StripComments;
use lazy_static::lazy_static;
use serde_json::Value;
use slog::error;
use std::io::Read;
use std::{collections::HashMap, str::FromStr};

struct ErrorCode {
    explanation: String,
}

lazy_static! {
    static ref CODEBOOK: HashMap<String, ErrorCode> = {
        let codefile_content = include_str!("../../../docs/error_codes.json");

        let mut stripped_json = String::new();
        StripComments::new(codefile_content.as_bytes())
            .read_to_string(&mut stripped_json)
            .expect("the file error_codes.json was malformed at build time");

        let parsed = Value::from_str(&stripped_json)
            .expect("the file error_codes.json was malformed at build time");

        let mut map = HashMap::new();

        if let Some(codes) = parsed.get("codes").and_then(|v| v.as_array()) {
            for code_entry in codes {
                if let (Some(code), Some(explanation)) = (
                    code_entry.get("code").and_then(|v| v.as_str()),
                    code_entry.get("explanation").and_then(|v| v.as_str()),
                ) {
                    map.insert(
                        code.to_string(),
                        ErrorCode {
                            explanation: explanation.to_string(),
                        },
                    );
                }
            }
        }

        map
    };
}

impl Interpreter<'_> {
    pub(crate) fn explain_error_code(&self, code: &str) -> Result<(), InterpreterError> {
        if let Some(error_code) = CODEBOOK.get(code) {
            println!("{}", error_code.explanation);
            Ok(())
        } else {
            error!(self.log, "'{}' is not a recognized error code.", code);
            Err(InterpreterError::ExplainErrorUnrecognizedCode)
        }
    }
}
