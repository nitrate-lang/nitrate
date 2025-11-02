use interned_string::IString;
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use ordered_float::OrderedFloat;
use std::{collections::HashMap, sync::LazyLock};

type BuiltinFunction =
    dyn Fn(&mut HirEvalCtx, &Store, &[Value]) -> Result<Value, Unwind> + Send + Sync;

static DEFAULT_BUILTIN_FUNCTIONS: LazyLock<HashMap<IString, Box<BuiltinFunction>>> =
    LazyLock::new(|| {
        let mut m: HashMap<IString, Box<BuiltinFunction>> = HashMap::new();

        // Just an example builtin function
        m.insert(
            IString::from("std::math::abs"),
            Box::new(|_, _, args| {
                if args.len() != 1 {
                    return Err(Unwind::TypeError);
                }

                match &args[0] {
                    Value::I8(i) => Ok(Value::I8(i.abs())),
                    Value::I16(i) => Ok(Value::I16(i.abs())),
                    Value::I32(i) => Ok(Value::I32(i.abs())),
                    Value::I64(i) => Ok(Value::I64(i.abs())),
                    Value::I128(i) => Ok(Value::I128(Box::new(i.abs()))),
                    Value::F32(i) => Ok(Value::F32(OrderedFloat(i.abs()))),
                    Value::F64(i) => Ok(Value::F64(OrderedFloat(i.abs()))),
                    _ => Err(Unwind::TypeError),
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
    added_builtin_functions: HashMap<IString, Box<BuiltinFunction>>,
    pub(crate) ptr_size: PtrSize,
}

impl<'store, 'log> HirEvalCtx<'store, 'log> {
    pub fn new(
        store: &'store Store,
        log: &'log CompilerLog,
        ptr_size: PtrSize,
    ) -> HirEvalCtx<'store, 'log> {
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
            ptr_size,
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

    pub fn add_builtin(mut self, name: IString, func: Box<BuiltinFunction>) -> Self {
        self.added_builtin_functions.insert(name, func);
        self
    }

    pub fn lookup_builtin(&self, name: &IString) -> Option<&Box<BuiltinFunction>> {
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

pub enum Unwind {
    LoopLimitExceeded,
    FunctionCallLimitExceeded,

    DivisionByZero,
    ModuloByZero,
    ShiftAmountError,
    IndexOutOfBounds,

    Break { label: Option<IString> },
    Continue { label: Option<IString> },
    Return(Value),

    TypeError,
}

pub trait HirEvaluate {
    type Output;

    fn evaluate(&self, ctx: &mut HirEvalCtx) -> Result<Self::Output, Unwind>;
}

impl HirEvalCtx<'_, '_> {
    pub fn evaluate_to_literal(&mut self, value: &Value) -> Result<Lit, Unwind> {
        match Lit::try_from(value.evaluate(self)?) {
            Ok(lit) => Ok(lit),
            Err(_) => Err(Unwind::TypeError),
        }
    }

    pub fn evaluate_into_type(&mut self, value: &Value) -> Result<Type, Unwind> {
        match value.evaluate(self)? {
            Value::StructObject {
                struct_path: _,
                fields: _,
            } => {
                // TODO: convert from nitrate's `std::meta::Type` into nitrate_hir::Type

                Err(Unwind::TypeError)
            }

            _ => Err(Unwind::TypeError),
        }
    }
}
