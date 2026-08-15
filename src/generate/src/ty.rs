use crate::Gen;
use nitrate_translation::parsetree::ast::{self, *};
use std::matches;

/// Maximum nesting depth for recursive type generation.
const MAX_TYPE_DEPTH: u32 = 4;

impl Gen {
    /// Generate a concrete type that we can always construct a value for.
    /// This is a primitive (bool/int/uint/float/usize) or a tuple of such types.
    /// Struct/enum type-paths are deliberately excluded because the compiler
    /// currently cannot construct struct values or enum variants without error.
    pub(crate) fn gen_concrete_type(&mut self) -> ast::Type {
        if self.type_depth >= MAX_TYPE_DEPTH {
            return self.gen_primitive_type();
        }
        self.type_depth += 1;
        let result = if self.next_u64() % 8 == 0 {
            self.gen_tuple_type()
        } else {
            self.gen_primitive_type()
        };
        self.type_depth = self.type_depth.saturating_sub(1);
        result
    }

    /// Type used for struct fields, enum payloads, parameters, return types and
    /// constants — everything we must be able to produce a value for.
    pub(crate) fn gen_field_type(&mut self) -> ast::Type {
        self.gen_concrete_type()
    }

    pub(crate) fn gen_primitive_type(&mut self) -> ast::Type {
        match self.next_u64() % 12 {
            0 => bool_type(),
            1 => int32_type(),
            2 => int64_type(),
            3 => int8_type(),
            4 => int16_type(),
            5 => uint8_type(),
            6 => uint16_type(),
            7 => uint32_type(),
            8 => uint64_type(),
            9 => usize_type(),
            10 => float32_type(),
            _ => float64_type(),
        }
    }

    fn gen_tuple_type(&mut self) -> ast::Type {
        let count = 2 + (self.next_u64() % 3) as usize; // 2..=4 elements
        let element_types: Vec<ast::Type> = (0..count).map(|_| self.gen_concrete_type()).collect();
        ast::Type::TupleType(Box::new(ast::TupleType {
            span: SrcSpan::default(),
            element_types,
        }))
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Primitive type constructors
// ─────────────────────────────────────────────────────────────────────────────

pub(crate) fn bool_type() -> ast::Type {
    ast::Type::Bool(ast::Bool {
        span: SrcSpan::default(),
    })
}

pub(crate) fn int8_type() -> ast::Type {
    ast::Type::Int8(ast::Int8 {
        span: SrcSpan::default(),
    })
}

pub(crate) fn int16_type() -> ast::Type {
    ast::Type::Int16(ast::Int16 {
        span: SrcSpan::default(),
    })
}

pub(crate) fn int32_type() -> ast::Type {
    ast::Type::Int32(ast::Int32 {
        span: SrcSpan::default(),
    })
}

pub(crate) fn int64_type() -> ast::Type {
    ast::Type::Int64(ast::Int64 {
        span: SrcSpan::default(),
    })
}

pub(crate) fn uint8_type() -> ast::Type {
    ast::Type::UInt8(ast::UInt8 {
        span: SrcSpan::default(),
    })
}

pub(crate) fn uint16_type() -> ast::Type {
    ast::Type::UInt16(ast::UInt16 {
        span: SrcSpan::default(),
    })
}

pub(crate) fn uint32_type() -> ast::Type {
    ast::Type::UInt32(ast::UInt32 {
        span: SrcSpan::default(),
    })
}

pub(crate) fn uint64_type() -> ast::Type {
    ast::Type::UInt64(ast::UInt64 {
        span: SrcSpan::default(),
    })
}

pub(crate) fn usize_type() -> ast::Type {
    ast::Type::USize(ast::USize {
        span: SrcSpan::default(),
    })
}

pub(crate) fn float32_type() -> ast::Type {
    ast::Type::Float32(ast::Float32 {
        span: SrcSpan::default(),
    })
}

pub(crate) fn float64_type() -> ast::Type {
    ast::Type::Float64(ast::Float64 {
        span: SrcSpan::default(),
    })
}

pub(crate) fn unit_type() -> ast::Type {
    ast::Type::TupleType(Box::new(ast::TupleType {
        span: SrcSpan::default(),
        element_types: vec![],
    }))
}

// ─────────────────────────────────────────────────────────────────────────────
// Classification helpers
// ─────────────────────────────────────────────────────────────────────────────

pub(crate) fn is_unit_type(ty: &ast::Type) -> bool {
    matches!(ty, ast::Type::TupleType(t) if t.element_types.is_empty())
}

pub(crate) fn is_bool_type(ty: &ast::Type) -> bool {
    matches!(ty, ast::Type::Bool(_))
}

pub(crate) fn is_float_type(ty: &ast::Type) -> bool {
    matches!(ty, ast::Type::Float32(_) | ast::Type::Float64(_))
}

pub(crate) fn is_integral_type(ty: &ast::Type) -> bool {
    matches!(
        ty,
        ast::Type::Int8(_)
            | ast::Type::Int16(_)
            | ast::Type::Int32(_)
            | ast::Type::Int64(_)
            | ast::Type::Int128(_)
            | ast::Type::UInt8(_)
            | ast::Type::UInt16(_)
            | ast::Type::UInt32(_)
            | ast::Type::UInt64(_)
            | ast::Type::UInt128(_)
            | ast::Type::USize(_)
    )
}

pub(crate) fn is_signed_type(ty: &ast::Type) -> bool {
    matches!(
        ty,
        ast::Type::Int8(_) | ast::Type::Int16(_) | ast::Type::Int32(_) | ast::Type::Int64(_) | ast::Type::Int128(_)
    )
}

/// Returns the element types of a tuple, or an empty vector for non-tuple types.
pub(crate) fn tuple_field_types(ty: &ast::Type) -> Vec<ast::Type> {
    match ty {
        ast::Type::TupleType(t) => t.element_types.clone(),
        _ => vec![],
    }
}

/// Returns the maximum non-negative magnitude that can be represented by the
/// given integral type, so generated literals always fit.  `None` if not integral.
pub(crate) fn unsigned_magnitude_max(ty: &ast::Type) -> Option<u128> {
    match ty {
        ast::Type::Int8(_) => Some(i8::MAX as u128),
        ast::Type::Int16(_) => Some(i16::MAX as u128),
        ast::Type::Int32(_) => Some(i32::MAX as u128),
        ast::Type::Int64(_) => Some(i64::MAX as u128),
        ast::Type::Int128(_) => Some(i128::MAX as u128),
        ast::Type::UInt8(_) => Some(u8::MAX as u128),
        ast::Type::UInt16(_) => Some(u16::MAX as u128),
        ast::Type::UInt32(_) => Some(u32::MAX as u128),
        ast::Type::UInt64(_) => Some(u64::MAX as u128),
        ast::Type::UInt128(_) => Some(u128::MAX),
        ast::Type::USize(_) => Some(u64::MAX as u128),
        _ => None,
    }
}
