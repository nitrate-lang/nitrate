use nitrate_diagnosis::CompilerLog;
use nitrate_hir::hir::{QualifiedName, Value};
use ordered_float::NotNan;
use std::{collections::HashMap, sync::LazyLock};

type BuiltinFunction = dyn Fn(&mut HirEvalCtx, &[Value]) -> Result<Value, EvalFail> + Send + Sync;

static DEFAULT_BUILTIN_FUNCTIONS: LazyLock<HashMap<QualifiedName, Box<BuiltinFunction>>> =
    LazyLock::new(|| {
        let mut m: HashMap<QualifiedName, Box<BuiltinFunction>> = HashMap::new();

        // Just an example builtin function
        m.insert(
            QualifiedName::from("std::math::abs"),
            Box::new(|_ctx, args| {
                if args.len() != 1 {
                    return Err(EvalFail::TypeError);
                }

                match &args[0] {
                    Value::I8(i) => Ok(Value::I8(i.abs())),
                    Value::I16(i) => Ok(Value::I16(i.abs())),
                    Value::I32(i) => Ok(Value::I32(i.abs())),
                    Value::I64(i) => Ok(Value::I64(i.abs())),
                    Value::I128(i) => Ok(Value::I128(Box::new(i.abs()))),
                    Value::F8(i) => Ok(Value::F8(NotNan::new(i.abs()).unwrap())),
                    Value::F16(i) => Ok(Value::F16(NotNan::new(i.abs()).unwrap())),
                    Value::F32(i) => Ok(Value::F32(NotNan::new(i.abs()).unwrap())),
                    Value::F64(i) => Ok(Value::F64(NotNan::new(i.abs()).unwrap())),
                    Value::F128(i) => Ok(Value::F128(NotNan::new(i.abs()).unwrap())),
                    _ => Err(EvalFail::TypeError),
                }
            }),
        );

        m
    });

pub struct HirEvalCtx<'log> {
    pub(crate) log: &'log CompilerLog,
    pub(crate) loop_iter_limit: usize,
    pub(crate) loop_iter_count: usize,
    pub(crate) function_call_limit: usize,
    pub(crate) function_call_count: usize,
    added_builtin_functions: HashMap<QualifiedName, Box<BuiltinFunction>>,
}

impl<'a> HirEvalCtx<'a> {
    pub fn new(log: &'a CompilerLog) -> HirEvalCtx<'a> {
        HirEvalCtx {
            log,
            loop_iter_limit: 4096,
            loop_iter_count: 0,
            function_call_limit: 4096,
            function_call_count: 0,
            added_builtin_functions: HashMap::new(),
        }
    }

    pub fn with_loop_iter_limit(mut self, limit: usize) -> Self {
        self.loop_iter_limit = limit;
        self
    }

    pub fn with_function_call_limit(mut self, limit: usize) -> Self {
        self.function_call_limit = limit;
        self
    }

    pub fn add_builtin(mut self, name: QualifiedName, func: Box<BuiltinFunction>) -> Self {
        self.added_builtin_functions.insert(name, func);
        self
    }

    pub(crate) fn lookup_builtin(&self, name: &QualifiedName) -> Option<&Box<BuiltinFunction>> {
        self.added_builtin_functions
            .get(name)
            .or_else(|| DEFAULT_BUILTIN_FUNCTIONS.get(name))
    }
}

pub enum EvalFail {
    LoopLimitExceeded,
    FunctionCallLimitExceeded,

    TypeError,
}

pub trait HirEvaluate {
    type Output;

    fn evaluate(&self, ctx: &mut HirEvalCtx) -> Result<Self::Output, EvalFail>;
}
