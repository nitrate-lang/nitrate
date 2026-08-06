use crate::Gen;
use nitrate_translation::parsetree::ast::{self, *};

/// Maximum nesting depth for recursively-generated types to prevent stack
/// overflow when types are generated during expression generation.
const MAX_TYPE_DEPTH: u32 = 4;

impl Gen {
    /// Generate a random AST type. Avoids types that are not well-supported
    /// by the compiler (refinement types, void types in certain positions).
    pub(crate) fn gen_type(&mut self) -> ast::Type {
        if self.type_depth >= MAX_TYPE_DEPTH {
            return self.gen_leaf_type();
        }
        self.type_depth += 1;
        let result = match self.next_u64() % 18 {
            0..=3 => self.gen_int_type(),
            4 => self.gen_uint_type(),
            5..=6 => self.gen_bool_type(),
            7 => self.gen_float_type(),
            8 => self.gen_usize_type(),
            9 => self.gen_slice_type(),
            10 => self.gen_array_type(),
            11 => self.gen_tuple_type(),
            12 => self.gen_type_path(),
            13 => self.gen_reference_type(),
            14 => self.gen_pointer_type(),
            15 => self.gen_function_type(),
            16..=17 => self.gen_int_type(),
            _ => self.gen_int_type(),
        };
        self.type_depth = self.type_depth.saturating_sub(1);
        result
    }

    /// Generate a type that does NOT recurse into further type generation.
    fn gen_leaf_type(&mut self) -> ast::Type {
        match self.next_u64() % 6 {
            0 => self.gen_bool_type(),
            1 => self.gen_int_type(),
            2 => self.gen_uint_type(),
            3 => self.gen_float_type(),
            4 => self.gen_usize_type(),
            _ => {
                if self.has_any_struct() {
                    let idx = self.gen_index(self.known_structs.len());
                    let name = self.known_structs[idx].name.clone();
                    ast::Type::TypePath(Box::new(ast::TypePath {
                        span: SrcSpan::default(),
                        segments: vec![ast::TypePathSegment {
                            span: SrcSpan::default(),
                            name,
                            type_arguments: None,
                        }],
                        resolved_path: None,
                    }))
                } else {
                    self.gen_int_type()
                }
            }
        }
    }

    // ── Primitive type generators ──

    fn gen_bool_type(&mut self) -> ast::Type {
        ast::Type::Bool(ast::Bool {
            span: SrcSpan::default(),
        })
    }

    fn gen_int_type(&mut self) -> ast::Type {
        match self.next_u64() % 5 {
            0 => ast::Type::Int8(ast::Int8 {
                span: SrcSpan::default(),
            }),
            1 => ast::Type::Int16(ast::Int16 {
                span: SrcSpan::default(),
            }),
            2 => ast::Type::Int32(ast::Int32 {
                span: SrcSpan::default(),
            }),
            3 => ast::Type::Int64(ast::Int64 {
                span: SrcSpan::default(),
            }),
            _ => ast::Type::Int128(ast::Int128 {
                span: SrcSpan::default(),
            }),
        }
    }

    fn gen_uint_type(&mut self) -> ast::Type {
        match self.next_u64() % 5 {
            0 => ast::Type::UInt8(ast::UInt8 {
                span: SrcSpan::default(),
            }),
            1 => ast::Type::UInt16(ast::UInt16 {
                span: SrcSpan::default(),
            }),
            2 => ast::Type::UInt32(ast::UInt32 {
                span: SrcSpan::default(),
            }),
            3 => ast::Type::UInt64(ast::UInt64 {
                span: SrcSpan::default(),
            }),
            _ => ast::Type::UInt128(ast::UInt128 {
                span: SrcSpan::default(),
            }),
        }
    }

    fn gen_float_type(&mut self) -> ast::Type {
        if self.next_bool() {
            ast::Type::Float32(ast::Float32 {
                span: SrcSpan::default(),
            })
        } else {
            ast::Type::Float64(ast::Float64 {
                span: SrcSpan::default(),
            })
        }
    }

    fn gen_usize_type(&mut self) -> ast::Type {
        ast::Type::USize(ast::USize {
            span: SrcSpan::default(),
        })
    }

    // ── Compound type generators ──

    fn gen_array_type(&mut self) -> ast::Type {
        let element_type = self.gen_type();
        let len_val = (self.next_u64() % 32 + 1) as u128;
        let len = ast::Expr::Integer(Box::new(ast::IntegerLit {
            span: SrcSpan::default(),
            value: len_val,
            kind: nitrate_translation::token::IntegerKind::Dec,
        }));
        ast::Type::ArrayType(Box::new(ast::ArrayType {
            span: SrcSpan::default(),
            element_type,
            len,
        }))
    }

    fn gen_slice_type(&mut self) -> ast::Type {
        let element_type = self.gen_type();
        ast::Type::SliceType(Box::new(ast::SliceType {
            span: SrcSpan::default(),
            element_type,
        }))
    }

    fn gen_tuple_type(&mut self) -> ast::Type {
        let count = 1 + (self.next_u64() as usize % 3);
        let element_types: Vec<ast::Type> = (0..count).map(|_| self.gen_type()).collect();
        ast::Type::TupleType(Box::new(ast::TupleType {
            span: SrcSpan::default(),
            element_types,
        }))
    }

    fn gen_function_type(&mut self) -> ast::Type {
        let param_count = self.next_u64() as usize % 3;
        let parameters: Vec<ast::FuncTypeParam> = (0..param_count)
            .map(|i| ast::FuncTypeParam {
                span: SrcSpan::default(),
                attributes: None,
                name: format!("p_{}", i).into(),
                ty: self.gen_type(),
            })
            .collect();
        let return_type = if self.next_bool() { Some(self.gen_type()) } else { None };
        ast::Type::FunctionType(Box::new(ast::FunctionType {
            span: SrcSpan::default(),
            attributes: None,
            parameters: parameters.into(),
            return_type,
        }))
    }

    fn gen_reference_type(&mut self) -> ast::Type {
        let to = self.gen_type();
        // Always include a lifetime with references to avoid unbound lifetime errors.
        let lifetime = Some(ast::Lifetime {
            span: SrcSpan::default(),
            name: "a".into(),
        });
        let mutability = if self.next_bool() {
            Some(if self.next_bool() {
                ast::Mutability::Mut
            } else {
                ast::Mutability::Const
            })
        } else {
            None
        };
        ast::Type::ReferenceType(Box::new(ast::ReferenceType {
            span: SrcSpan::default(),
            lifetime,
            exclusivity: None,
            mutability,
            to,
        }))
    }

    fn gen_pointer_type(&mut self) -> ast::Type {
        let to = self.gen_type();
        let mutability = if self.next_bool() {
            Some(if self.next_bool() {
                ast::Mutability::Mut
            } else {
                ast::Mutability::Const
            })
        } else {
            None
        };
        ast::Type::PointerType(Box::new(ast::PointerType {
            span: SrcSpan::default(),
            lifetime: None,
            exclusivity: None,
            mutability,
            to,
        }))
    }

    fn gen_type_path(&mut self) -> ast::Type {
        if self.has_any_struct() {
            let idx = self.gen_index(self.known_structs.len());
            let name = self.known_structs[idx].name.clone();
            ast::Type::TypePath(Box::new(ast::TypePath {
                span: SrcSpan::default(),
                segments: vec![ast::TypePathSegment {
                    span: SrcSpan::default(),
                    name,
                    type_arguments: None,
                }],
                resolved_path: None,
            }))
        } else {
            self.gen_int_type()
        }
    }
}

// ── Public convenience helpers (used by rvalue.rs and item.rs) ──

pub(crate) fn bool_type() -> ast::Type {
    ast::Type::Bool(ast::Bool {
        span: SrcSpan::default(),
    })
}

pub(crate) fn int32_type() -> ast::Type {
    ast::Type::Int32(ast::Int32 {
        span: SrcSpan::default(),
    })
}

pub(crate) fn usize_type() -> ast::Type {
    ast::Type::USize(ast::USize {
        span: SrcSpan::default(),
    })
}

pub(crate) fn float64_type() -> ast::Type {
    ast::Type::Float64(ast::Float64 {
        span: SrcSpan::default(),
    })
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

pub(crate) fn is_bool_type(ty: &ast::Type) -> bool {
    matches!(ty, ast::Type::Bool(_))
}

pub(crate) fn is_numeric_type(ty: &ast::Type) -> bool {
    is_integral_type(ty) || matches!(ty, ast::Type::Float32(_) | ast::Type::Float64(_))
}

pub(crate) fn is_float_type(ty: &ast::Type) -> bool {
    matches!(ty, ast::Type::Float32(_) | ast::Type::Float64(_))
}

/// Check if two types are structurally compatible for expression assignment.
/// More permissive than exact match — allows int→int and uint→uint family matches.
pub(crate) fn types_compatible(a: &ast::Type, b: &ast::Type) -> bool {
    use ast::Type::*;
    match (a, b) {
        (Bool(_), Bool(_))
        | (Int8(_), Int8(_))
        | (Int16(_), Int16(_))
        | (Int32(_), Int32(_))
        | (Int64(_), Int64(_))
        | (Int128(_), Int128(_))
        | (UInt8(_), UInt8(_))
        | (UInt16(_), UInt16(_))
        | (UInt32(_), UInt32(_))
        | (UInt64(_), UInt64(_))
        | (UInt128(_), UInt128(_))
        | (USize(_), USize(_))
        | (Float32(_), Float32(_))
        | (Float64(_), Float64(_))
        | (ArrayType(_), ArrayType(_))
        | (SliceType(_), SliceType(_))
        | (TupleType(_), TupleType(_))
        | (FunctionType(_), FunctionType(_))
        | (ReferenceType(_), ReferenceType(_))
        | (PointerType(_), PointerType(_))
        | (TypePath(_), TypePath(_))
        | (InferType(_), InferType(_)) => true,
        // Allow any integral type to be used with any other integral type for flexibility
        (a_ty, b_ty) if is_integral_type(a_ty) && is_integral_type(b_ty) => true,
        // Allow any float type with any float type
        (a_ty, b_ty) if is_float_type(a_ty) && is_float_type(b_ty) => true,
        _ => false,
    }
}

pub(crate) fn is_type_path_like(ty: &ast::Type) -> bool {
    matches!(ty, ast::Type::TypePath(_))
}

pub(crate) fn is_type_path_or_typeof_target(ty: &ast::Type) -> bool {
    matches!(ty, ast::Type::TypePath(_) | ast::Type::InferType(_))
}

pub(crate) fn element_type_of(ty: &ast::Type) -> ast::Type {
    match ty {
        ast::Type::ArrayType(arr) => arr.element_type.clone(),
        ast::Type::SliceType(slice) => slice.element_type.clone(),
        _ => int32_type(),
    }
}

pub(crate) fn tuple_field_types(ty: &ast::Type) -> Vec<ast::Type> {
    match ty {
        ast::Type::TupleType(tup) => tup.element_types.clone(),
        _ => (0..2).map(|_| int32_type()).collect(),
    }
}

pub(crate) fn function_type_parts(ty: &ast::Type) -> (Vec<ast::Type>, ast::Type) {
    match ty {
        ast::Type::FunctionType(ft) => {
            let params: Vec<ast::Type> = ft.parameters.iter().map(|p| p.ty.clone()).collect();
            let ret = ft.return_type.clone().unwrap_or_else(|| {
                ast::Type::TupleType(Box::new(ast::TupleType {
                    span: SrcSpan::default(),
                    element_types: vec![],
                }))
            });
            (params, ret)
        }
        _ => (vec![int32_type()], int32_type()),
    }
}

pub(crate) fn range_element_type(ty: &ast::Type) -> ast::Type {
    if is_integral_type(ty) { ty.clone() } else { int32_type() }
}

pub(crate) fn arbitrary_type() -> ast::Type {
    int32_type()
}

/// Pick a random non-void type for cast source.
pub(crate) fn random_cast_source_type(generator: &mut Gen) -> ast::Type {
    match generator.next_u64() % 8 {
        0 => int32_type(),
        1 => bool_type(),
        2 => usize_type(),
        3 => float64_type(),
        4 => ast::Type::Float32(ast::Float32 {
            span: SrcSpan::default(),
        }),
        5 => ast::Type::UInt32(ast::UInt32 {
            span: SrcSpan::default(),
        }),
        6 => ast::Type::Int64(ast::Int64 {
            span: SrcSpan::default(),
        }),
        _ => ast::Type::Int128(ast::Int128 {
            span: SrcSpan::default(),
        }),
    }
}

/// Generate a fallback expression of the given type (for when budget runs out).
pub(crate) fn fallback_expr(ty: &ast::Type) -> ast::Expr {
    if is_bool_type(ty) {
        ast::Expr::Boolean(ast::BooleanLit {
            span: SrcSpan::default(),
            value: false,
        })
    } else if matches!(ty, ast::Type::Float32(_) | ast::Type::Float64(_)) {
        ast::Expr::Float(ast::FloatLit {
            span: SrcSpan::default(),
            value: ordered_float::NotNan::new(0.0).unwrap(),
        })
    } else {
        ast::Expr::Integer(Box::new(ast::IntegerLit {
            span: SrcSpan::default(),
            value: 0,
            kind: nitrate_translation::token::IntegerKind::Dec,
        }))
    }
}
