use crate::prelude::*;
use crate::store::LiteralId;
use crate::store::ValueId;
use nitrate_nstring::NString;
use nitrate_tree::ByteSpan;
use serde::{Deserialize, Serialize};
use std::collections::BTreeSet;
use std::matches;
use std::num::NonZeroU32;
use thin_vec::ThinVec;

/// The lifetime of a reference or pointer.
///
/// Nitrate supports several lifetime kinds:
/// - `Static`: Lives for the entire program duration.
/// - `Gc`: Garbage-collected lifetime.
/// - `ThreadLocal`: Bound to the current thread.
/// - `TaskLocal`: Bound to the current async task.
/// - `Inferred`: To be determined by the borrow checker.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum Lifetime {
    /// Lifetime spanning the entire program (`'static`).
    Static,
    /// Garbage-collected heap lifetime.
    Gc,
    /// Lifetime scoped to the current thread.
    ThreadLocal,
    /// Lifetime scoped to the current async task.
    TaskLocal,
    /// Lifetime to be inferred by the borrow checker.
    Inferred,
}

/// A trait or lifetime bound (e.g. `T: Clone + 'static`).
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum TypeBound {
    /// A trait that must be implemented by the constrained type.
    Trait(TraitId),
    /// A lifetime bound (e.g. `'static`).
    Lifetime(Lifetime),
}

/// A `where` clause constraint (e.g. `where T: Clone + Debug`).
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct WhereClause {
    /// The type being constrained.
    pub type_id: TypeId,
    /// The bounds required on the type.
    pub bounds: Vec<TypeBound>,
}

/// External ABI specification for FFI functions.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct ExternAbi {
    /// Name of the external ABI (e.g. `"C"`).
    pub name: NString,
}

/// Attributes applied to function declarations.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum FunctionAttribute {
    /// The function takes a variable number of arguments (C-style variadic).
    CVariadic,
    /// Do not mangle the function's symbol name.
    NoMangle,
    /// The function uses an external ABI.
    ExternAbi(ExternAbi),
}

/// A function type signature used in function pointer types.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct FunctionType {
    /// Attributes applied to this function type.
    pub attributes: BTreeSet<FunctionAttribute>,
    /// Named parameters with their types.
    pub params: ThinVec<(NString, TypeId)>,
    /// The return type of the function.
    pub return_type: TypeId,
}

/// The central type representation in the HIR.
///
/// Types are interned and deduplicated — two identical types always produce
/// the same [`TypeId`]. The `span` field is excluded from `Hash`/`Eq`/`Ord`
/// to preserve deduplication regardless of source location.
///
/// Types fall into several categories:
/// - **Primitives**: `Never`, `Unit`, `Bool`, integer and float types.
/// - **Compounds**: `Array`, `Tuple`, `Struct`, `Enum`.
/// - **Indirections**: `Reference`, `Pointer`, `SliceRef`, `SlicePtr`.
/// - **Special**: `Function`, `Refine`, `Parameterized`, `TraitObject`.
/// - **Inference**: `Inferred`, `InferredInteger`, `InferredFloat`, `GenericParam`.
/// - **Unresolved**: `UnresolvedArray`, `UnresolvedRefine` (pre-evaluation forms).
#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum Type {
    /// The never type (`!`). Indicates a computation that never completes.
    /// It is the bottom type: a subtype of every type.
    Never {
        /// Source location.
        span: ByteSpan,
    },
    /// The unit type (`()`). A zero-sized type with exactly one value.
    Unit {
        /// Source location.
        span: ByteSpan,
    },
    /// The boolean type (`bool`). Either `true` or `false`.
    Bool {
        /// Source location.
        span: ByteSpan,
    },
    /// Unsigned 8-bit integer (`u8`). Range: 0 to 255.
    U8 {
        /// Source location.
        span: ByteSpan,
    },
    /// Unsigned 16-bit integer (`u16`). Range: 0 to 65535.
    U16 {
        /// Source location.
        span: ByteSpan,
    },
    /// Unsigned 32-bit integer (`u32`). Range: 0 to 4,294,967,295.
    U32 {
        /// Source location.
        span: ByteSpan,
    },
    /// Unsigned 64-bit integer (`u64`).
    U64 {
        /// Source location.
        span: ByteSpan,
    },
    /// Unsigned 128-bit integer (`u128`).
    U128 {
        /// Source location.
        span: ByteSpan,
    },
    /// Architecture-dependent unsigned integer (`usize`). 32 or 64 bits.
    USize {
        /// Source location.
        span: ByteSpan,
    },
    /// Signed 8-bit integer (`i8`). Range: -128 to 127.
    I8 {
        /// Source location.
        span: ByteSpan,
    },
    /// Signed 16-bit integer (`i16`). Range: -32,768 to 32,767.
    I16 {
        /// Source location.
        span: ByteSpan,
    },
    /// Signed 32-bit integer (`i32`).
    I32 {
        /// Source location.
        span: ByteSpan,
    },
    /// Signed 64-bit integer (`i64`).
    I64 {
        /// Source location.
        span: ByteSpan,
    },
    /// Signed 128-bit integer (`i128`).
    I128 {
        /// Source location.
        span: ByteSpan,
    },
    /// 32-bit IEEE 754 floating-point (`f32`).
    F32 {
        /// Source location.
        span: ByteSpan,
    },
    /// 64-bit IEEE 754 floating-point (`f64`).
    F64 {
        /// Source location.
        span: ByteSpan,
    },

    /// A fixed-size array type (`[T; N]`).
    Array {
        /// Source location.
        span: ByteSpan,
        /// The type of each element.
        element_type: TypeId,
        /// The number of elements.
        len: u32,
    },
    /// A heterogeneous fixed-size tuple (`(A, B, C)`).
    Tuple {
        /// Source location.
        span: ByteSpan,
        /// The types of each element, in order.
        element_types: ThinVec<TypeId>,
    },
    /// A named struct type.
    Struct {
        /// Source location.
        span: ByteSpan,
        /// Reference to the struct definition.
        def: StructDefId,
    },
    /// A named enum type.
    Enum {
        /// Source location.
        span: ByteSpan,
        /// Reference to the enum definition.
        def: EnumDefId,
    },
    /// A type alias.
    TypeAlias {
        /// Source location.
        span: ByteSpan,
        /// Reference to the alias definition.
        def: TypeAliasDefId,
    },
    /// A refinement type (`T<min..max>`) constraining a base type to a
    /// specific range of values. Used for integer range types.
    Refine {
        /// Source location.
        span: ByteSpan,
        /// The base type being refined (e.g., `I32`).
        base: TypeId,
        /// The inclusive lower bound (compile-time literal).
        min: LiteralId,
        /// The inclusive upper bound (compile-time literal).
        max: LiteralId,
    },
    /// Array type with an expression for length, not yet evaluated.
    /// Resolved to [`Type::Array`] during `hir_solve`.
    UnresolvedArray {
        /// Source location.
        span: ByteSpan,
        /// The type of each element.
        element_type: TypeId,
        /// An unevaluated expression for the array length.
        len: ValueId,
    },
    /// Refinement type with expressions for bounds, not yet evaluated.
    /// Resolved to [`Type::Refine`] during `hir_solve`.
    UnresolvedRefine {
        /// Source location.
        span: ByteSpan,
        /// The base type being refined.
        base: TypeId,
        /// An unevaluated expression for the minimum bound.
        min: ValueId,
        /// An unevaluated expression for the maximum bound.
        max: ValueId,
    },
    /// A function pointer type (`fn(A, B) -> C`).
    Function {
        /// Source location.
        span: ByteSpan,
        /// The full function type signature.
        function_type: Box<FunctionType>,
    },
    /// A reference type (`&T`, `&mut T`, `&unique T`).
    Reference {
        /// Source location.
        span: ByteSpan,
        /// The lifetime of the reference.
        lifetime: Lifetime,
        /// Whether the reference is unique (exclusive) or shared.
        exclusive: bool,
        /// Whether the referent can be mutated through this reference.
        mutable: bool,
        /// The type being referenced.
        to: TypeId,
    },
    /// A reference to a slice (`&[T]`, `&mut [T]`).
    SliceRef {
        /// Source location.
        span: ByteSpan,
        /// The lifetime of the slice reference.
        lifetime: Lifetime,
        /// Whether the reference is unique (exclusive).
        exclusive: bool,
        /// Whether elements can be mutated through this reference.
        mutable: bool,
        /// The type of each element in the slice.
        element_type: TypeId,
    },
    /// A raw pointer type (`*T`, `*mut T`, `*unique T`).
    Pointer {
        /// Source location.
        span: ByteSpan,
        /// The lifetime associated with this pointer.
        lifetime: Lifetime,
        /// Whether the pointer is unique (exclusive).
        exclusive: bool,
        /// Whether the pointee can be mutated through this pointer.
        mutable: bool,
        /// The type being pointed to.
        to: TypeId,
    },
    /// A raw pointer to a slice (`*[T]`, `*mut [T]`).
    SlicePtr {
        /// Source location.
        span: ByteSpan,
        /// The lifetime associated with this slice pointer.
        lifetime: Lifetime,
        /// Whether the pointer is unique (exclusive).
        exclusive: bool,
        /// Whether elements can be mutated through this pointer.
        mutable: bool,
        /// The type of each element in the slice.
        element_type: TypeId,
    },
    /// A trait object type (`dyn Trait` or `impl Trait`).
    TraitObject {
        /// Source location.
        span: ByteSpan,
        /// The trait bounds that constrain this type.
        bounds: Vec<TypeBound>,
    },
    /// A parameterized type application (`Base<A, B, C>`).
    Parameterized {
        /// Source location.
        span: ByteSpan,
        /// The base type being parameterized (e.g., a generic struct).
        base: TypeId,
        /// The type arguments applied to the base.
        args: Arguments<TypeId>,
    },
    /// A floating-point literal whose concrete type has not yet been inferred.
    /// Defaults to `F64` if no constraints determine the type.
    InferredFloat {
        /// Source location.
        span: ByteSpan,
    },
    /// An integer literal whose concrete type has not yet been inferred.
    /// Defaults to `I32` if no constraints determine the type.
    InferredInteger {
        /// Source location.
        span: ByteSpan,
    },
    /// A type variable to be inferred by Hindley-Milner unification.
    /// Carries a unique identifier for the inference variable and an
    /// optional name for diagnostics.
    Inferred {
        /// Source location.
        span: ByteSpan,
        /// Unique identifier for this inference variable.
        id: NonZeroU32,
        /// Optional name for diagnostic output (e.g., from `_` or named type vars).
        name: Option<NString>,
    },
    /// A generic type parameter declared on a function, struct, enum, or type alias
    /// (e.g., `T` in `fn foo<T>(x: T)`).
    GenericParam {
        /// Source location.
        span: ByteSpan,
        /// Index of this parameter in the generic parameter list.
        index: u32,
        /// The name of the generic parameter.
        name: NString,
    },
    /// The range type — used internally for range expression desugaring.
    Range {
        /// Source location.
        span: ByteSpan,
    },
    /// The string type (`str`). Unsized sequence of UTF-8 bytes.
    Str {
        /// Source location.
        span: ByteSpan,
    },
}

/// Variant discriminant used for Ord ordering.
#[repr(u8)]
enum TypeDisc {
    Never = 0,
    Unit = 1,
    Bool = 2,
    U8 = 3,
    U16 = 4,
    U32 = 5,
    U64 = 6,
    U128 = 7,
    USize = 8,
    I8 = 9,
    I16 = 10,
    I32 = 11,
    I64 = 12,
    I128 = 13,
    F32 = 14,
    F64 = 15,
    Array = 16,
    Tuple = 17,
    Struct = 18,
    Enum = 19,
    TypeAlias = 20,
    Refine = 21,
    Function = 22,
    Reference = 23,
    SliceRef = 24,
    Pointer = 25,
    SlicePtr = 26,
    TraitObject = 27,
    Parameterized = 28,
    InferredFloat = 29,
    InferredInteger = 30,
    Inferred = 31,
    GenericParam = 32,
    Range = 33,
    Str = 34,
}

impl Type {
    fn disc(&self) -> TypeDisc {
        match self {
            Type::Never { .. } => TypeDisc::Never,
            Type::Unit { .. } => TypeDisc::Unit,
            Type::Bool { .. } => TypeDisc::Bool,
            Type::U8 { .. } => TypeDisc::U8,
            Type::U16 { .. } => TypeDisc::U16,
            Type::U32 { .. } => TypeDisc::U32,
            Type::U64 { .. } => TypeDisc::U64,
            Type::U128 { .. } => TypeDisc::U128,
            Type::USize { .. } => TypeDisc::USize,
            Type::I8 { .. } => TypeDisc::I8,
            Type::I16 { .. } => TypeDisc::I16,
            Type::I32 { .. } => TypeDisc::I32,
            Type::I64 { .. } => TypeDisc::I64,
            Type::I128 { .. } => TypeDisc::I128,
            Type::F32 { .. } => TypeDisc::F32,
            Type::F64 { .. } => TypeDisc::F64,
            Type::Array { .. } => TypeDisc::Array,
            Type::Tuple { .. } => TypeDisc::Tuple,
            Type::Struct { .. } => TypeDisc::Struct,
            Type::Enum { .. } => TypeDisc::Enum,
            Type::TypeAlias { .. } => TypeDisc::TypeAlias,
            Type::Refine { .. } => TypeDisc::Refine,
            Type::UnresolvedArray { .. } => TypeDisc::Array,
            Type::UnresolvedRefine { .. } => TypeDisc::Refine,
            Type::Function { .. } => TypeDisc::Function,
            Type::Reference { .. } => TypeDisc::Reference,
            Type::SliceRef { .. } => TypeDisc::SliceRef,
            Type::Pointer { .. } => TypeDisc::Pointer,
            Type::SlicePtr { .. } => TypeDisc::SlicePtr,
            Type::TraitObject { .. } => TypeDisc::TraitObject,
            Type::Parameterized { .. } => TypeDisc::Parameterized,
            Type::InferredFloat { .. } => TypeDisc::InferredFloat,
            Type::InferredInteger { .. } => TypeDisc::InferredInteger,
            Type::Inferred { .. } => TypeDisc::Inferred,
            Type::GenericParam { .. } => TypeDisc::GenericParam,
            Type::Range { .. } => TypeDisc::Range,
            Type::Str { .. } => TypeDisc::Str,
        }
    }

    #[must_use]
    pub fn span(&self) -> ByteSpan {
        match self {
            Type::Never { span } => *span,
            Type::Unit { span } => *span,
            Type::Bool { span } => *span,
            Type::U8 { span } => *span,
            Type::U16 { span } => *span,
            Type::U32 { span } => *span,
            Type::U64 { span } => *span,
            Type::U128 { span } => *span,
            Type::USize { span } => *span,
            Type::I8 { span } => *span,
            Type::I16 { span } => *span,
            Type::I32 { span } => *span,
            Type::I64 { span } => *span,
            Type::I128 { span } => *span,
            Type::F32 { span } => *span,
            Type::F64 { span } => *span,
            Type::Array { span, .. } => *span,
            Type::Tuple { span, .. } => *span,
            Type::Struct { span, .. } => *span,
            Type::Enum { span, .. } => *span,
            Type::TypeAlias { span, .. } => *span,
            Type::Refine { span, .. } | Type::UnresolvedArray { span, .. } | Type::UnresolvedRefine { span, .. } => {
                *span
            }
            Type::Function { span, .. } => *span,
            Type::Reference { span, .. } => *span,
            Type::SliceRef { span, .. } => *span,
            Type::Pointer { span, .. } => *span,
            Type::SlicePtr { span, .. } => *span,
            Type::TraitObject { span, .. } => *span,
            Type::Parameterized { span, .. } => *span,
            Type::InferredFloat { span } => *span,
            Type::InferredInteger { span } => *span,
            Type::Inferred { span, .. } => *span,
            Type::GenericParam { span, .. } => *span,
            Type::Range { span, .. } => *span,
            Type::Str { span, .. } => *span,
        }
    }

    #[must_use]
    pub fn is_diverging(&self) -> bool {
        matches!(self, Type::Never { .. })
    }
    #[must_use]
    pub fn is_bool(&self) -> bool {
        matches!(self, Type::Bool { .. })
    }
    #[must_use]
    pub fn is_unsigned_primitive(&self) -> bool {
        matches!(
            self,
            Type::U8 { .. }
                | Type::U16 { .. }
                | Type::U32 { .. }
                | Type::U64 { .. }
                | Type::U128 { .. }
                | Type::USize { .. }
        )
    }
    #[must_use]
    pub fn is_signed_primitive(&self) -> bool {
        matches!(
            self,
            Type::I8 { .. } | Type::I16 { .. } | Type::I32 { .. } | Type::I64 { .. } | Type::I128 { .. }
        )
    }
    #[must_use]
    pub fn is_integer_primitive(&self) -> bool {
        self.is_unsigned_primitive() || self.is_signed_primitive()
    }
    #[must_use]
    pub fn is_float_primitive(&self) -> bool {
        matches!(self, Type::F32 { .. } | Type::F64 { .. })
    }
    #[must_use]
    pub fn is_array(&self) -> bool {
        matches!(self, Type::Array { .. } | Type::UnresolvedArray { .. })
    }
    #[must_use]
    pub fn is_tuple(&self) -> bool {
        matches!(self, Type::Tuple { .. })
    }
    #[must_use]
    pub fn is_struct(&self) -> bool {
        matches!(self, Type::Struct { .. })
    }
    #[must_use]
    pub fn is_enum(&self) -> bool {
        matches!(self, Type::Enum { .. })
    }
    #[must_use]
    pub fn is_function(&self) -> bool {
        matches!(self, Type::Function { .. })
    }
    #[must_use]
    pub fn is_reference(&self) -> bool {
        matches!(self, Type::Reference { .. })
    }
    #[must_use]
    pub fn is_pointer(&self) -> bool {
        matches!(self, Type::Pointer { .. })
    }
    #[must_use]
    pub fn is_slice_ref(&self) -> bool {
        matches!(self, Type::SliceRef { .. })
    }
    #[must_use]
    pub fn is_slice_ptr(&self) -> bool {
        matches!(self, Type::SlicePtr { .. })
    }
    #[must_use]
    pub fn is_inferred(&self) -> bool {
        matches!(
            self,
            Type::Inferred { .. } | Type::InferredFloat { .. } | Type::InferredInteger { .. }
        )
    }
    #[must_use]
    pub fn as_struct(&self) -> Option<&StructDefId> {
        if let Type::Struct { def, .. } = self {
            Some(def)
        } else {
            None
        }
    }
    #[must_use]
    pub fn as_enum(&self) -> Option<&EnumDefId> {
        if let Type::Enum { def, .. } = self {
            Some(def)
        } else {
            None
        }
    }
    #[must_use]
    pub fn as_type_alias(&self) -> Option<&TypeAliasDefId> {
        if let Type::TypeAlias { def, .. } = self {
            Some(def)
        } else {
            None
        }
    }
}

// Custom PartialEq that ignores the span field to preserve type deduplication
impl PartialEq for Type {
    fn eq(&self, other: &Self) -> bool {
        match (self, other) {
            (Type::Never { .. }, Type::Never { .. }) => true,
            (Type::Unit { .. }, Type::Unit { .. }) => true,
            (Type::Bool { .. }, Type::Bool { .. }) => true,
            (Type::U8 { .. }, Type::U8 { .. }) => true,
            (Type::U16 { .. }, Type::U16 { .. }) => true,
            (Type::U32 { .. }, Type::U32 { .. }) => true,
            (Type::U64 { .. }, Type::U64 { .. }) => true,
            (Type::U128 { .. }, Type::U128 { .. }) => true,
            (Type::USize { .. }, Type::USize { .. }) => true,
            (Type::I8 { .. }, Type::I8 { .. }) => true,
            (Type::I16 { .. }, Type::I16 { .. }) => true,
            (Type::I32 { .. }, Type::I32 { .. }) => true,
            (Type::I64 { .. }, Type::I64 { .. }) => true,
            (Type::I128 { .. }, Type::I128 { .. }) => true,
            (Type::F32 { .. }, Type::F32 { .. }) => true,
            (Type::F64 { .. }, Type::F64 { .. }) => true,
            (
                Type::Array {
                    element_type: a1,
                    len: l1,
                    ..
                },
                Type::Array {
                    element_type: a2,
                    len: l2,
                    ..
                },
            ) => a1 == a2 && l1 == l2,
            (Type::Tuple { element_types: e1, .. }, Type::Tuple { element_types: e2, .. }) => e1 == e2,
            (Type::Struct { def: d1, .. }, Type::Struct { def: d2, .. }) => d1 == d2,
            (Type::Enum { def: d1, .. }, Type::Enum { def: d2, .. }) => d1 == d2,
            (Type::TypeAlias { def: d1, .. }, Type::TypeAlias { def: d2, .. }) => d1 == d2,
            (
                Type::Refine {
                    base: b1,
                    min: mi1,
                    max: ma1,
                    ..
                },
                Type::Refine {
                    base: b2,
                    min: mi2,
                    max: ma2,
                    ..
                },
            ) => b1 == b2 && mi1 == mi2 && ma1 == ma2,
            (Type::Function { function_type: f1, .. }, Type::Function { function_type: f2, .. }) => f1 == f2,
            (
                Type::Reference {
                    lifetime: l1,
                    exclusive: e1,
                    mutable: m1,
                    to: t1,
                    ..
                },
                Type::Reference {
                    lifetime: l2,
                    exclusive: e2,
                    mutable: m2,
                    to: t2,
                    ..
                },
            ) => l1 == l2 && e1 == e2 && m1 == m2 && t1 == t2,
            (
                Type::SliceRef {
                    lifetime: l1,
                    exclusive: e1,
                    mutable: m1,
                    element_type: t1,
                    ..
                },
                Type::SliceRef {
                    lifetime: l2,
                    exclusive: e2,
                    mutable: m2,
                    element_type: t2,
                    ..
                },
            ) => l1 == l2 && e1 == e2 && m1 == m2 && t1 == t2,
            (
                Type::Pointer {
                    lifetime: l1,
                    exclusive: e1,
                    mutable: m1,
                    to: t1,
                    ..
                },
                Type::Pointer {
                    lifetime: l2,
                    exclusive: e2,
                    mutable: m2,
                    to: t2,
                    ..
                },
            ) => l1 == l2 && e1 == e2 && m1 == m2 && t1 == t2,
            (
                Type::SlicePtr {
                    lifetime: l1,
                    exclusive: e1,
                    mutable: m1,
                    element_type: t1,
                    ..
                },
                Type::SlicePtr {
                    lifetime: l2,
                    exclusive: e2,
                    mutable: m2,
                    element_type: t2,
                    ..
                },
            ) => l1 == l2 && e1 == e2 && m1 == m2 && t1 == t2,
            (Type::TraitObject { bounds: b1, .. }, Type::TraitObject { bounds: b2, .. }) => b1 == b2,
            (Type::Parameterized { base: b1, args: a1, .. }, Type::Parameterized { base: b2, args: a2, .. }) => {
                b1 == b2 && a1 == a2
            }
            (Type::InferredFloat { .. }, Type::InferredFloat { .. }) => true,
            (Type::InferredInteger { .. }, Type::InferredInteger { .. }) => true,
            (Type::Inferred { id: i1, name: n1, .. }, Type::Inferred { id: i2, name: n2, .. }) => i1 == i2 && n1 == n2,
            (
                Type::GenericParam {
                    index: i1, name: n1, ..
                },
                Type::GenericParam {
                    index: i2, name: n2, ..
                },
            ) => i1 == i2 && n1 == n2,
            _ => false,
        }
    }
}
impl Eq for Type {}

// Custom Hash that ignores the span field
impl std::hash::Hash for Type {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        match self {
            Type::Never { .. } => 0u8.hash(state),
            Type::Unit { .. } => 1u8.hash(state),
            Type::Bool { .. } => 2u8.hash(state),
            Type::U8 { .. } => 3u8.hash(state),
            Type::U16 { .. } => 4u8.hash(state),
            Type::U32 { .. } => 5u8.hash(state),
            Type::U64 { .. } => 6u8.hash(state),
            Type::U128 { .. } => 7u8.hash(state),
            Type::USize { .. } => 8u8.hash(state),
            Type::I8 { .. } => 9u8.hash(state),
            Type::I16 { .. } => 10u8.hash(state),
            Type::I32 { .. } => 11u8.hash(state),
            Type::I64 { .. } => 12u8.hash(state),
            Type::I128 { .. } => 13u8.hash(state),
            Type::F32 { .. } => 14u8.hash(state),
            Type::F64 { .. } => 15u8.hash(state),
            Type::Array { element_type, len, .. } => {
                16u8.hash(state);
                element_type.hash(state);
                len.hash(state);
            }
            Type::Tuple { element_types, .. } => {
                17u8.hash(state);
                element_types.hash(state);
            }
            Type::Struct { def, .. } => {
                18u8.hash(state);
                def.hash(state);
            }
            Type::Enum { def, .. } => {
                19u8.hash(state);
                def.hash(state);
            }
            Type::TypeAlias { def, .. } => {
                20u8.hash(state);
                def.hash(state);
            }
            Type::Refine { base, min, max, .. } => {
                21u8.hash(state);
                base.hash(state);
                min.hash(state);
                max.hash(state);
            }
            Type::UnresolvedArray { element_type, len, .. } => {
                35u8.hash(state);
                element_type.hash(state);
                len.hash(state);
            }
            Type::UnresolvedRefine { base, min, max, .. } => {
                36u8.hash(state);
                base.hash(state);
                min.hash(state);
                max.hash(state);
            }
            Type::Function { function_type, .. } => {
                22u8.hash(state);
                function_type.hash(state);
            }
            Type::Reference {
                lifetime,
                exclusive,
                mutable,
                to,
                ..
            } => {
                23u8.hash(state);
                lifetime.hash(state);
                exclusive.hash(state);
                mutable.hash(state);
                to.hash(state);
            }
            Type::SliceRef {
                lifetime,
                exclusive,
                mutable,
                element_type,
                ..
            } => {
                24u8.hash(state);
                lifetime.hash(state);
                exclusive.hash(state);
                mutable.hash(state);
                element_type.hash(state);
            }
            Type::Pointer {
                lifetime,
                exclusive,
                mutable,
                to,
                ..
            } => {
                25u8.hash(state);
                lifetime.hash(state);
                exclusive.hash(state);
                mutable.hash(state);
                to.hash(state);
            }
            Type::SlicePtr {
                lifetime,
                exclusive,
                mutable,
                element_type,
                ..
            } => {
                26u8.hash(state);
                lifetime.hash(state);
                exclusive.hash(state);
                mutable.hash(state);
                element_type.hash(state);
            }
            Type::TraitObject { bounds, .. } => {
                27u8.hash(state);
                bounds.hash(state);
            }
            Type::Parameterized { base, args, .. } => {
                28u8.hash(state);
                base.hash(state);
                args.hash(state);
            }
            Type::InferredFloat { .. } => 29u8.hash(state),
            Type::InferredInteger { .. } => 30u8.hash(state),
            Type::Inferred { id, name, .. } => {
                31u8.hash(state);
                id.hash(state);
                name.hash(state);
            }
            Type::GenericParam { index, name, .. } => {
                32u8.hash(state);
                index.hash(state);
                name.hash(state);
            }
            Type::Range { .. } => 33u8.hash(state),
            Type::Str { .. } => 34u8.hash(state),
        }
    }
}

impl PartialOrd for Type {
    fn partial_cmp(&self, other: &Self) -> Option<std::cmp::Ordering> {
        Some(self.cmp(other))
    }
}

// Custom Ord that ignores the span field — compares by discriminant then by fields (via as_usize for handles)
impl Ord for Type {
    fn cmp(&self, other: &Self) -> std::cmp::Ordering {
        let self_disc = self.disc() as u8;
        let other_disc = other.disc() as u8;
        match self_disc.cmp(&other_disc) {
            std::cmp::Ordering::Equal => match (self, other) {
                (
                    Type::Array {
                        element_type: e1,
                        len: l1,
                        ..
                    },
                    Type::Array {
                        element_type: e2,
                        len: l2,
                        ..
                    },
                ) => match e1.as_usize().cmp(&e2.as_usize()) {
                    std::cmp::Ordering::Equal => l1.cmp(l2),
                    other => other,
                },
                (Type::Tuple { element_types: e1, .. }, Type::Tuple { element_types: e2, .. }) => {
                    let v1: Vec<usize> = e1.iter().map(|t| t.as_usize()).collect();
                    let v2: Vec<usize> = e2.iter().map(|t| t.as_usize()).collect();
                    v1.cmp(&v2)
                }
                (Type::Struct { def: d1, .. }, Type::Struct { def: d2, .. }) => d1.as_usize().cmp(&d2.as_usize()),
                (Type::Enum { def: d1, .. }, Type::Enum { def: d2, .. }) => d1.as_usize().cmp(&d2.as_usize()),
                (Type::TypeAlias { def: d1, .. }, Type::TypeAlias { def: d2, .. }) => d1.as_usize().cmp(&d2.as_usize()),
                (
                    Type::Refine {
                        base: b1,
                        min: mi1,
                        max: ma1,
                        ..
                    },
                    Type::Refine {
                        base: b2,
                        min: mi2,
                        max: ma2,
                        ..
                    },
                ) => match b1.as_usize().cmp(&b2.as_usize()) {
                    std::cmp::Ordering::Equal => match mi1.as_usize().cmp(&mi2.as_usize()) {
                        std::cmp::Ordering::Equal => ma1.as_usize().cmp(&ma2.as_usize()),
                        other => other,
                    },
                    other => other,
                },
                (
                    Type::UnresolvedArray {
                        element_type: e1,
                        len: l1,
                        ..
                    },
                    Type::UnresolvedArray {
                        element_type: e2,
                        len: l2,
                        ..
                    },
                ) => match e1.as_usize().cmp(&e2.as_usize()) {
                    std::cmp::Ordering::Equal => l1.as_usize().cmp(&l2.as_usize()),
                    other => other,
                },
                (
                    Type::UnresolvedRefine {
                        base: b1,
                        min: mi1,
                        max: ma1,
                        ..
                    },
                    Type::UnresolvedRefine {
                        base: b2,
                        min: mi2,
                        max: ma2,
                        ..
                    },
                ) => match b1.as_usize().cmp(&b2.as_usize()) {
                    std::cmp::Ordering::Equal => match mi1.as_usize().cmp(&mi2.as_usize()) {
                        std::cmp::Ordering::Equal => ma1.as_usize().cmp(&ma2.as_usize()),
                        other => other,
                    },
                    other => other,
                },
                (Type::Function { function_type: f1, .. }, Type::Function { function_type: f2, .. }) => {
                    f1.attributes.cmp(&f2.attributes)
                }
                _ => std::cmp::Ordering::Equal,
            },
            other => other,
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
pub enum PtrSize {
    U32 = 4,
    U64 = 8,
}

impl From<Type> for TypeId {
    fn from(ty: Type) -> Self {
        get_storage(|store| store.store_type(ty))
    }
}
