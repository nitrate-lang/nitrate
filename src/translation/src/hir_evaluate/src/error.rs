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
}
