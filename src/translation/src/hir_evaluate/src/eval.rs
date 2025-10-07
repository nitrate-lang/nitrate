use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use std::{collections::HashMap, sync::LazyLock};

type BuiltinFunction =
    dyn Fn(&mut HirEvalCtx, &Store, &[Value]) -> Result<Value, EvalFail> + Send + Sync;

static DEFAULT_BUILTIN_FUNCTIONS: LazyLock<HashMap<QualifiedName, Box<BuiltinFunction>>> =
    LazyLock::new(|| {
        let mut m: HashMap<QualifiedName, Box<BuiltinFunction>> = HashMap::new();

        // Just an example builtin function
        m.insert(
            QualifiedName::from("std::math::abs"),
            Box::new(|_, _, args| {
                if args.len() != 1 {
                    return Err(EvalFail::TypeError);
                }

                match &args[0] {
                    Value::I8(i) => Ok(Value::I8(i.abs())),
                    Value::I16(i) => Ok(Value::I16(i.abs())),
                    Value::I32(i) => Ok(Value::I32(i.abs())),
                    Value::I64(i) => Ok(Value::I64(i.abs())),
                    Value::I128(i) => Ok(Value::I128(Box::new(i.abs()))),
                    Value::F8(i) => Ok(Value::F8(i.abs())),
                    Value::F16(i) => Ok(Value::F16(i.abs())),
                    Value::F32(i) => Ok(Value::F32(i.abs())),
                    Value::F64(i) => Ok(Value::F64(i.abs())),
                    Value::F128(i) => Ok(Value::F128(i.abs())),
                    _ => Err(EvalFail::TypeError),
                }
            }),
        );

        m
    });

pub struct HirEvalCtx<'store, 'log> {
    pub(crate) store: &'store Store,
    pub(crate) log: &'log CompilerLog,
    pub(crate) loop_iter_limit: usize,
    pub(crate) loop_iter_count: usize,
    pub(crate) function_call_limit: usize,
    pub(crate) function_call_count: usize,
    pub(crate) current_safety: BlockSafety,
    pub(crate) unsafe_operations_performed: usize,
    added_builtin_functions: HashMap<QualifiedName, Box<BuiltinFunction>>,
}

impl<'store, 'log> HirEvalCtx<'store, 'log> {
    pub fn new(store: &'store Store, log: &'log CompilerLog) -> HirEvalCtx<'store, 'log> {
        HirEvalCtx {
            store,
            log,
            loop_iter_limit: 4096,
            loop_iter_count: 0,
            function_call_limit: 4096,
            function_call_count: 0,
            current_safety: BlockSafety::Safe,
            unsafe_operations_performed: 0,
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

    pub fn lookup_builtin(&self, name: &QualifiedName) -> Option<&Box<BuiltinFunction>> {
        self.added_builtin_functions
            .get(name)
            .or_else(|| DEFAULT_BUILTIN_FUNCTIONS.get(name))
    }

    pub fn unsafe_operations_performed(&self) -> usize {
        self.unsafe_operations_performed
    }

    pub fn loop_iter_count(&self) -> usize {
        self.loop_iter_count
    }

    pub fn function_call_count(&self) -> usize {
        self.function_call_count
    }

    pub fn get_logger(&self) -> &CompilerLog {
        self.log
    }
}

pub enum EvalFail {
    LoopLimitExceeded,
    FunctionCallLimitExceeded,

    DivisionByZero,
    ModuloByZero,

    TypeError,
}

pub trait HirEvaluate {
    type Output;

    fn evaluate(&self, ctx: &mut HirEvalCtx) -> Result<Self::Output, EvalFail>;
}
