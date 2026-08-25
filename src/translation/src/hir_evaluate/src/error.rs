use std::write;

use nitrate_hir::prelude::Value;
use nitrate_nstring::NString;

/// Errors that can occur during HIR evaluation.
///
/// These replace the old `Unwind` type and provide richer error information.
/// Control-flow "errors" (`Break`, `Continue`, `Return`) use Rust's `Result`
/// mechanism to unwind the evaluation stack efficiently.
#[derive(Debug, Clone)]
pub enum EvalError {
    /// `break` statement encountered, with optional label
    Break { label: Option<NString> },
    /// `continue` statement encountered, with optional label
    Continue { label: Option<NString> },
    /// `return` statement encountered, carrying the return value
    Return(Value),
    /// Division by zero (integer or float)
    DivisionByZero,
    /// Modulo by zero
    ModuloByZero,
    /// Shift amount out of range (negative or >= bit width)
    ShiftAmountError,
    /// Runtime type error (mismatched operand types, wrong variant, etc.)
    TypeError,
    /// Loop iteration limit exceeded (prevents infinite loops)
    LoopLimitExceeded,
    /// Function call depth limit exceeded (prevents infinite recursion)
    CallDepthExceeded,
    /// Total memory allocation limit exceeded
    MemoryLimitExceeded,
    /// Dereference of a pointer that is not part of the abstract heap
    InvalidPointer,
    /// Memory access outside the bounds of a known allocation
    OutOfBoundsAccess,
    /// Memory access does not satisfy alignment requirements
    MisalignedAccess,
    /// An operation that is not yet implemented in the evaluator
    Unsupported(&'static str),
    /// An unsafe operation was attempted in a safe context
    UnsafeInSafeContext,
    /// An programmatic abort was requested (e.g., via `std::meta::abort`)
    Abort,
}

impl std::fmt::Display for EvalError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            EvalError::Break { label } => {
                if let Some(lbl) = label {
                    write!(f, "break statement encountered with label '{}'", lbl)
                } else {
                    write!(f, "break statement encountered")
                }
            }
            EvalError::Continue { label } => {
                if let Some(lbl) = label {
                    write!(f, "continue statement encountered with label '{}'", lbl)
                } else {
                    write!(f, "continue statement encountered")
                }
            }
            EvalError::Return(value) => write!(f, "return statement encountered with value {:?}", value),
            EvalError::DivisionByZero => write!(f, "division by zero"),
            EvalError::ModuloByZero => write!(f, "modulo by zero"),
            EvalError::ShiftAmountError => write!(f, "shift amount out of range"),
            EvalError::TypeError => write!(f, "runtime type error"),
            EvalError::LoopLimitExceeded => write!(f, "loop iteration limit exceeded"),
            EvalError::CallDepthExceeded => write!(f, "function call depth limit exceeded"),
            EvalError::MemoryLimitExceeded => write!(f, "total memory allocation limit exceeded"),
            EvalError::InvalidPointer => write!(f, "dereference of an invalid pointer"),
            EvalError::OutOfBoundsAccess => write!(f, "memory access out of bounds"),
            EvalError::MisalignedAccess => write!(f, "misaligned memory access"),
            EvalError::Unsupported(op) => write!(f, "unsupported operation: {}", op),
            EvalError::UnsafeInSafeContext => write!(f, "unsafe operation attempted in a safe context"),
            EvalError::Abort => write!(f, "programmatic abort requested"),
        }
    }
}
