use crate::builtins::DEFAULT_BUILTIN_FUNCTIONS;
use crate::error::EvalError;
use crate::memory::Memory;
use crate::value::evaluate_value;
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_nstring::NString;
use nitrate_tree::ByteSpan;
use std::collections::HashMap;

/// A single stack frame for function evaluation.
#[derive(Debug, Clone)]
pub(crate) struct Frame {
    /// Local variable bindings: name → Value
    pub locals: HashMap<NString, Value>,
    /// Parameter bindings: name → Value
    pub params: HashMap<NString, Value>,
}

impl Frame {
    pub fn new(params: &[(NString, Value)]) -> Self {
        let mut param_map = HashMap::with_capacity(params.len());
        for (name, value) in params {
            param_map.insert(name.clone(), value.clone());
        }
        Self {
            locals: HashMap::new(),
            params: param_map,
        }
    }

    pub fn get_binding(&self, name: &NString) -> Option<&Value> {
        self.locals.get(name).or_else(|| self.params.get(name))
    }

    pub fn set_binding(&mut self, name: NString, value: Value) {
        self.locals.insert(name, value);
    }
}

/// The HIR tree-walking interpreter.
///
/// Evaluates HIR `Value` expressions, function bodies, blocks, and
/// global variable initializers. Uses an abstract memory model for
/// all pointer/reference operations so that safe and unsafe code
/// can both be evaluated without touching real process memory.
pub struct Evaluator<'log> {
    /// Abstract memory heap
    pub memory: Memory,
    /// Diagnostic log (not used for errors in the evaluator itself,
    /// but passed through for built-in function compatibility)
    pub log: &'log CompilerLog,
    /// Pointer size (32 or 64 bit)
    pub ptr_size: PtrSize,
    /// Maximum loop iterations before aborting
    pub loop_limit: usize,
    /// Maximum function call depth before aborting
    pub call_depth_limit: usize,
    /// Maximum total memory (in bytes) the abstract heap can grow to
    pub memory_limit: usize,
    /// Current call stack frames
    pub(crate) frames: Vec<Frame>,
    /// Loop iteration counter for the current compilation
    pub(crate) loop_count: usize,
    /// Current call depth counter
    pub(crate) call_depth: usize,
    /// Current block safety context
    pub(crate) current_safety: BlockSafety,
    /// Count of unsafe operations performed (informational)
    pub(crate) unsafe_operations_performed: usize,
    /// User-registered built-in functions
    pub(crate) added_builtins: HashMap<NString, BuiltinFn>,
}

/// Signature for a built-in function.
pub type BuiltinFn = fn(&mut Evaluator, &[Value]) -> Result<Value, EvalError>;

impl<'log> Evaluator<'log> {
    /// Create a new evaluator.
    pub fn new(log: &'log CompilerLog, ptr_size: PtrSize) -> Self {
        Self {
            memory: Memory::new(1024 * 1024), // 1 MB default
            log,
            ptr_size,
            loop_limit: 1_000_000,
            call_depth_limit: 1000,
            memory_limit: 1024 * 1024,
            frames: Vec::new(),
            loop_count: 0,
            call_depth: 0,
            current_safety: BlockSafety::Safe,
            unsafe_operations_performed: 0,
            added_builtins: HashMap::new(),
        }
    }

    /// Register a built-in function that can be called during evaluation.
    pub fn add_builtin_function(&mut self, name: NString, function: BuiltinFn) {
        self.added_builtins.insert(name, function);
    }

    /// Look up a built-in function by name.
    pub(crate) fn get_builtin(&self, name: &NString) -> Option<BuiltinFn> {
        self.added_builtins
            .get(name)
            .copied()
            .or_else(|| DEFAULT_BUILTIN_FUNCTIONS.get(name).copied())
    }

    // ────────────────────────────────────────────────────────────────
    // Public API
    // ────────────────────────────────────────────────────────────────

    /// Evaluate a HIR value to a concrete value.
    ///
    /// This is the main entry point. It recursively evaluates the
    /// expression tree, handling all control flow, function calls,
    /// and memory operations.
    pub fn evaluate(&mut self, value: &Value) -> Result<Value, EvalError> {
        evaluate_value(self, value)
    }

    /// Evaluate a HIR value and destructure it into a `Lit`.
    ///
    /// This is the compatibility API used by `hir_from_tree` for
    /// refinement bounds, array sizes, and other compile-time
    /// constant contexts.
    pub fn evaluate_to_literal(&mut self, value: &Value) -> Result<Lit, EvalError> {
        let result = self.evaluate(value)?;
        Lit::try_from(result).map_err(|_| EvalError::TypeError)
    }

    /// Evaluate a block element (expression or local declaration).
    pub fn evaluate_block_element(&mut self, element: &BlockElement) -> Result<Value, EvalError> {
        match element {
            BlockElement::Expr(expr_id) => self.evaluate(&expr_id.borrow()),
            BlockElement::Local(local_id) => {
                let local = local_id.borrow();
                let init_value = if let Some(init_id) = &local.initializer {
                    self.evaluate(&init_id.borrow())?
                } else {
                    Value::Unit {
                        span: ByteSpan::default(),
                    }
                };
                if let Some(frame) = self.frames.last_mut() {
                    frame.set_binding(local.name.clone(), init_value);
                }
                Ok(Value::Unit {
                    span: ByteSpan::default(),
                })
            }
        }
    }

    /// Evaluate a function body with the given arguments.
    ///
    /// Pushes a new stack frame, binds parameters to arguments,
    /// evaluates the body, and returns the result.
    pub fn evaluate_function(&mut self, func_id: FunctionId, args: &[Value]) -> Result<Value, EvalError> {
        if self.call_depth >= self.call_depth_limit {
            return Err(EvalError::CallDepthExceeded);
        }
        self.call_depth += 1;

        let func = func_id.borrow();
        let func = func.clone();

        let param_names: Vec<(NString, Value)> = func
            .params
            .iter()
            .enumerate()
            .map(|(i, param_id)| {
                let param = param_id.borrow();
                let value = args.get(i).cloned().unwrap_or_else(|| Value::Unit {
                    span: ByteSpan::default(),
                });
                (param.name.clone(), value)
            })
            .collect();

        let frame = Frame::new(&param_names);
        self.frames.push(frame);

        let before_safety = self.current_safety.clone();
        if func.is_unsafe && self.current_safety == BlockSafety::Safe {
            self.frames.pop();
            self.call_depth -= 1;
            return Err(EvalError::UnsafeInSafeContext);
        }

        let mut last_value = Value::Unit {
            span: ByteSpan::default(),
        };
        if let Some(body) = &func.body {
            for element in body {
                match self.evaluate_block_element(element) {
                    Ok(val) => last_value = val,
                    Err(EvalError::Return(val)) => {
                        last_value = val;
                        break;
                    }
                    Err(e) => {
                        self.frames.pop();
                        self.current_safety = before_safety;
                        self.call_depth -= 1;
                        return Err(e);
                    }
                }
            }
        } else {
            if let Some(builtin) = self.get_builtin(&func.name) {
                let result = builtin(self, args);
                self.frames.pop();
                self.current_safety = before_safety;
                self.call_depth -= 1;
                return result;
            } else {
                self.frames.pop();
                self.current_safety = before_safety;
                self.call_depth -= 1;
                return Err(EvalError::Unsupported("extern function call"));
            }
        };

        self.frames.pop();
        self.current_safety = before_safety;
        self.call_depth -= 1;

        Ok(last_value)
    }

    /// Evaluate a block of statements/expressions.
    pub fn evaluate_block(&mut self, block: &Block) -> Result<Value, EvalError> {
        let before_safety = self.current_safety.clone();
        self.current_safety = block.safety.clone();

        let mut last_value = Value::Unit {
            span: ByteSpan::default(),
        };
        for element in &block.elements {
            match self.evaluate_block_element(element) {
                Ok(val) => last_value = val,
                Err(e) => {
                    self.current_safety = before_safety;
                    return Err(e);
                }
            }
        }

        self.current_safety = before_safety;
        Ok(last_value)
    }

    /// Evaluate a global variable's initializer to a `Lit`.
    pub fn evaluate_global_initializer(&mut self, global: &GlobalVariable) -> Result<Lit, EvalError> {
        let value = self.evaluate(&global.initializer.borrow())?;
        Lit::try_from(value).map_err(|_| EvalError::TypeError)
    }

    // ────────────────────────────────────────────────────────────────
    // Frame management
    // ────────────────────────────────────────────────────────────────

    pub(crate) fn current_frame(&self) -> &Frame {
        self.frames.last().expect("no active frame")
    }

    pub(crate) fn current_frame_mut(&mut self) -> &mut Frame {
        self.frames.last_mut().expect("no active frame")
    }

    pub(crate) fn lookup_binding(&self, name: &NString) -> Option<Value> {
        for frame in self.frames.iter().rev() {
            if let Some(value) = frame.get_binding(name) {
                return Some(value.clone());
            }
        }
        None
    }
}
