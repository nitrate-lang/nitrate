use crate::diagnosis::Diagnostic;
use crate::error::EvalError;
use crate::evaluator::BuiltinFn;
use nitrate_hir::Value;
use nitrate_nstring::NString;
use nitrate_tree::ByteSpan;
use std::collections::HashMap;
use std::sync::LazyLock;

/// Default built-in functions available during evaluation.
///
/// These are lazily initialized and merged with user-registered builtins
/// at evaluation time.
pub static DEFAULT_BUILTIN_FUNCTIONS: LazyLock<HashMap<NString, BuiltinFn>> = LazyLock::new(|| {
    let mut m: HashMap<NString, BuiltinFn> = HashMap::new();

    m.insert(
        "std::meta::abort".into(),
        (|_eval, args| {
            if args.len() != 0 {
                return Err(EvalError::TypeError);
            }

            Err(EvalError::Abort)
        }) as BuiltinFn,
    );

    m.insert(
        "std::meta::report".into(),
        (|eval, args| {
            if args.len() != 2 {
                return Err(EvalError::TypeError);
            }

            let msg_string = &args[0];
            let srcloc = &args[1];

            let Value::StringLit { value, .. } = msg_string else {
                return Err(EvalError::TypeError);
            };

            let Value::StructObject {
                span,
                struct_def,
                fields,
            } = srcloc
            else {
                return Err(EvalError::TypeError);
            };

            if &*struct_def.borrow().name != "std::meta::SourceLocation" {
                return Err(EvalError::TypeError);
            }

            let line = match fields
                .iter()
                .find(|(name, _)| &**name == "line")
                .map(|(_, value)| value.borrow())
                .as_deref()
            {
                Some(Value::U32 { value, .. }) => *value,
                _ => return Err(EvalError::TypeError),
            };

            let column = match fields
                .iter()
                .find(|(name, _)| &**name == "column")
                .map(|(_, value)| value.borrow())
                .as_deref()
            {
                Some(Value::U32 { value, .. }) => *value,
                _ => return Err(EvalError::TypeError),
            };

            let filename = match fields
                .iter()
                .find(|(name, _)| &**name == "filename")
                .map(|(_, value)| value.borrow())
                .as_deref()
            {
                Some(Value::StringLit { value, .. }) => value.clone(),
                _ => return Err(EvalError::TypeError),
            };

            let message = Diagnostic::UserMessage {
                message: format!("{}:{}:{}: {}", filename, line, column, value.as_str()),
            };
            eval.log.report(&message);
            Ok(Value::Unit {
                span: ByteSpan::default(),
            }
            .into())
        }) as BuiltinFn,
    );

    m
});
