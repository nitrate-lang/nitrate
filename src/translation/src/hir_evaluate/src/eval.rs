use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_nstring::NString;
use ordered_float::OrderedFloat;
use std::{collections::HashMap, sync::LazyLock};

type BuiltinFunction = dyn Fn(&mut HirEvalCtx, &[Value]) -> Result<Value, Unwind> + Send + Sync;

static DEFAULT_BUILTIN_FUNCTIONS: LazyLock<HashMap<NString, Box<BuiltinFunction>>> = LazyLock::new(|| {
    let mut m: HashMap<NString, Box<BuiltinFunction>> = HashMap::new();

    m.insert(
        NString::from("std::math::abs"),
        Box::new(|_, args| {
            if args.len() != 1 {
                return Err(Unwind::TypeError);
            }
            match &args[0] {
                Value::I8 { value: i, .. } => Ok(Value::I8 {
                    span: Default::default(),
                    value: i.abs(),
                }),
                Value::I16 { value: i, .. } => Ok(Value::I16 {
                    span: Default::default(),
                    value: i.abs(),
                }),
                Value::I32 { value: i, .. } => Ok(Value::I32 {
                    span: Default::default(),
                    value: i.abs(),
                }),
                Value::I64 { value: i, .. } => Ok(Value::I64 {
                    span: Default::default(),
                    value: i.abs(),
                }),
                Value::I128 { value: i, .. } => Ok(Value::I128 {
                    span: Default::default(),
                    value: Box::new(i.abs()),
                }),
                Value::F32 { value: i, .. } => Ok(Value::F32 {
                    span: Default::default(),
                    value: OrderedFloat(i.abs()),
                }),
                Value::F64 { value: i, .. } => Ok(Value::F64 {
                    span: Default::default(),
                    value: OrderedFloat(i.abs()),
                }),
                _ => Err(Unwind::TypeError),
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
    pub(crate) current_safety: BlockSafety,
    pub(crate) unsafe_operations_performed: usize,
    added_builtin_functions: HashMap<NString, Box<BuiltinFunction>>,
    pub(crate) ptr_size: PtrSize,
}

impl<'log> HirEvalCtx<'log> {
    pub fn new(log: &'log CompilerLog, ptr_size: PtrSize) -> HirEvalCtx<'log> {
        HirEvalCtx {
            log,
            loop_iter_limit: 1000,
            loop_iter_count: 0,
            function_call_limit: 100,
            function_call_count: 0,
            current_safety: BlockSafety::Safe,
            unsafe_operations_performed: 0,
            added_builtin_functions: HashMap::new(),
            ptr_size,
        }
    }

    pub fn add_builtin_function(&mut self, name: NString, function: Box<BuiltinFunction>) {
        self.added_builtin_functions.insert(name, function);
    }

    pub fn evaluate_to_literal(&mut self, value: &Value) -> Result<Lit, Unwind> {
        match value.evaluate(self)? {
            Value::Unit { .. } => Ok(Lit::Unit),
            Value::Bool { value: b, .. } => Ok(Lit::Bool(b)),
            Value::I8 { value: i, .. } => Ok(Lit::I8(i)),
            Value::I16 { value: i, .. } => Ok(Lit::I16(i)),
            Value::I32 { value: i, .. } => Ok(Lit::I32(i)),
            Value::I64 { value: i, .. } => Ok(Lit::I64(i)),
            Value::I128 { value: i, .. } => Ok(Lit::I128(*i)),
            Value::U8 { value: u, .. } => Ok(Lit::U8(u)),
            Value::U16 { value: u, .. } => Ok(Lit::U16(u)),
            Value::U32 { value: u, .. } => Ok(Lit::U32(u)),
            Value::U64 { value: u, .. } => Ok(Lit::U64(u)),
            Value::U128 { value: u, .. } => Ok(Lit::U128(*u)),
            Value::F32 { value: f, .. } => Ok(Lit::F32(f)),
            Value::F64 { value: f, .. } => Ok(Lit::F64(f)),
            Value::USize { bits, value: u, .. } => Ok(Lit::USize(bits, u)),
            _ => Err(Unwind::TypeError),
        }
    }

    pub fn get_builtin_function(&self, name: &NString) -> Option<&Box<BuiltinFunction>> {
        self.added_builtin_functions
            .get(name)
            .or_else(|| DEFAULT_BUILTIN_FUNCTIONS.get(name))
    }
}

#[derive(Debug, Clone)]
pub enum Unwind {
    Break { label: Option<NString> },
    Continue { label: Option<NString> },
    Return(Value),
    DivisionByZero,
    ModuloByZero,
    ShiftAmountError,
    TypeError,
    LoopLimitExceeded,
    FunctionCallLimitExceeded,
}

pub trait HirEvaluate {
    type Output;
    fn evaluate(&self, ctx: &mut HirEvalCtx) -> Result<Self::Output, Unwind>;
}
