use crate::Gen;
use nitrate_translation::parsetree::ast::{self, *};

/// Maximum nesting depth for recursively-generated types to prevent stack
/// overflow when types are generated during expression generation.
const MAX_TYPE_DEPTH: u32 = 4;

/// Depth at which compound types are forced to be simplified to keep
/// generated programs readable and type-checkable.
pub(crate) const MAX_TYPE_COMPLEXITY: u32 = 3;

impl Gen {
    /// Generate a random AST type.  This may produce non-constructible
    /// types (references, pointers, function types).  Use `gen_local_type`
    /// when the type must be constructible (e.g. for locals/params).
    pub(crate) fn gen_simple_type(&mut self) -> ast::Type {
        if self.type_depth >= MAX_TYPE_DEPTH {
            return self.gen_leaf_type();
        }
        self.type_depth += 1;
        let result = match self.next_u64() % 14 {
            0..=1 => self.gen_int_type(),
            2 => self.gen_uint_type(),
            3..=4 => self.gen_bool_type(),
            5 => self.gen_float_type(),
            6 => self.gen_usize_type(),
            7 => self.gen_slice_type(),
            8 => self.gen_array_type(),
            9 => self.gen_tuple_type(),
            10 => self.gen_type_path(),
            11 => self.gen_reference_type(),
            12 => self.gen_pointer_type(),
            _ => self.gen_function_type(),
        };
        self.type_depth = self.type_depth.saturating_sub(1);
        result
    }

    /// Backward-compatible alias.
    pub(crate) fn gen_type(&mut self) -> ast::Type {
        self.gen_simple_type()
    }

    fn gen_leaf_type(&mut self) -> ast::Type {
        let uses_enums = self.has_any_enum();
        let uses_structs = self.has_any_struct();

        // Build a weighted list of available leaf types
        let mut choices: Vec<(u32, Box<dyn FnOnce(&mut Gen) -> ast::Type>)> = Vec::new();
        // Always-available primitives get higher weight
        for _ in 0..3 {
            choices.push((3, Box::new(|g: &mut Gen| g.gen_bool_type())));
            choices.push((3, Box::new(|g: &mut Gen| g.gen_int_type())));
            choices.push((2, Box::new(|g: &mut Gen| g.gen_uint_type())));
        }
        choices.push((2, Box::new(|g: &mut Gen| g.gen_float_type())));
        choices.push((2, Box::new(|g: &mut Gen| g.gen_usize_type())));

        if uses_structs {
            choices.push((2, Box::new(|g: &mut Gen| g.gen_type_path())));
        }
        if uses_enums {
            choices.push((2, Box::new(|g: &mut Gen| g.gen_enum_path())));
        }

        let total_weight: u32 = choices.iter().map(|(w, _)| w).sum();
        let mut roll = self.next_u64() as u32 % total_weight;
        for (weight, func) in choices {
            if roll < weight {
                return func(self);
            }
            roll -= weight;
        }
        // Fallback
        self.gen_int_type()
    }

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

    fn gen_array_type(&mut self) -> ast::Type {
        // Nested types in arrays should be constructible so array literals work
        let element_type = if self.type_depth + 1 >= MAX_TYPE_DEPTH {
            self.gen_leaf_type()
        } else {
            self.gen_local_type()
        };
        let len_val = (self.next_u64() % 16 + 1) as u128;
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
        let element_type = if self.type_depth + 1 >= MAX_TYPE_DEPTH {
            self.gen_leaf_type()
        } else {
            self.gen_local_type()
        };
        ast::Type::SliceType(Box::new(ast::SliceType {
            span: SrcSpan::default(),
            element_type,
        }))
    }

    fn gen_tuple_type(&mut self) -> ast::Type {
        let count = 1 + (self.next_u64() as usize % 3);
        let element_types: Vec<ast::Type> = (0..count)
            .map(|_| {
                if self.type_depth + 1 >= MAX_TYPE_DEPTH {
                    self.gen_leaf_type()
                } else {
                    self.gen_local_type()
                }
            })
            .collect();
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
                ty: self.gen_local_type(),
            })
            .collect();
        let return_type = if self.next_bool() {
            Some(self.gen_local_type())
        } else {
            None
        };
        ast::Type::FunctionType(Box::new(ast::FunctionType {
            span: SrcSpan::default(),
            attributes: None,
            parameters: parameters.into(),
            return_type,
        }))
    }

    fn gen_reference_type(&mut self) -> ast::Type {
        let to = self.gen_local_type();
        // Use anonymous lifetime (valid Nitrate) — named lifetimes like 'a are reserved
        // for function signatures and can't be used in arbitrary reference types.
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
            lifetime: None,
            exclusivity: None,
            mutability,
            to,
        }))
    }

    fn gen_pointer_type(&mut self) -> ast::Type {
        let to = self.gen_local_type();
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

    fn gen_enum_path(&mut self) -> ast::Type {
        if self.has_any_enum() {
            let idx = self.gen_index(self.known_enums.len());
            let name = self.known_enums[idx].clone();
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

    /// Generate a type that is safe to use for local variables — meaning it
    /// must be fully constructible (no ref/ptr/fn types anywhere in the type).
    pub(crate) fn gen_local_type(&mut self) -> ast::Type {
        loop {
            let ty = self.gen_simple_type();
            if is_constructible_type(&ty) && type_complexity_depth(&ty) <= MAX_TYPE_COMPLEXITY {
                return ty;
            }
        }
    }
}

/// Returns the nesting depth of the most deeply nested compound type.
/// Used to limit type complexity in generated programs.
fn type_complexity_depth(ty: &ast::Type) -> u32 {
    match ty {
        ast::Type::ArrayType(arr) => 1 + type_complexity_depth(&arr.element_type),
        ast::Type::SliceType(slice) => 1 + type_complexity_depth(&slice.element_type),
        ast::Type::TupleType(tup) => {
            1 + tup
                .element_types
                .iter()
                .map(|t| type_complexity_depth(t))
                .max()
                .unwrap_or(0)
        }
        ast::Type::FunctionType(ft) => {
            let param_max = ft
                .parameters
                .iter()
                .map(|p| type_complexity_depth(&p.ty))
                .max()
                .unwrap_or(0);
            let ret_max = ft.return_type.as_ref().map(|t| type_complexity_depth(t)).unwrap_or(0);
            1 + param_max.max(ret_max)
        }
        _ => 0,
    }
}

// ── Public helpers ──

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

pub(crate) fn unit_type() -> ast::Type {
    ast::Type::TupleType(Box::new(ast::TupleType {
        span: SrcSpan::default(),
        element_types: vec![],
    }))
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

pub(crate) fn is_signed_integral_type(ty: &ast::Type) -> bool {
    matches!(
        ty,
        ast::Type::Int8(_) | ast::Type::Int16(_) | ast::Type::Int32(_) | ast::Type::Int64(_) | ast::Type::Int128(_)
    )
}

pub(crate) fn is_unsigned_integral_type(ty: &ast::Type) -> bool {
    matches!(
        ty,
        ast::Type::UInt8(_)
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

pub(crate) fn is_unit_type(ty: &ast::Type) -> bool {
    matches!(ty, ast::Type::TupleType(t) if t.element_types.is_empty())
}

pub(crate) fn is_type_path(ty: &ast::Type) -> bool {
    matches!(ty, ast::Type::TypePath(_))
}

/// Check whether a type is fully constructible — i.e., we can generate a
/// runtime value of this type without resorting to invalid casts. This check
/// is recursive: compound types (arrays, slices, tuples) must have fully
/// constructible element types.
pub(crate) fn is_constructible_type(ty: &ast::Type) -> bool {
    match ty {
        ast::Type::ReferenceType(_) | ast::Type::PointerType(_) | ast::Type::FunctionType(_) => false,
        ast::Type::ArrayType(arr) => is_constructible_type(&arr.element_type),
        ast::Type::SliceType(slice) => is_constructible_type(&slice.element_type),
        ast::Type::TupleType(tup) => tup.element_types.iter().all(|ft| is_constructible_type(ft)),
        _ => true,
    }
}

/// Check whether two types are structurally compatible.  For fuzzing purposes
/// we use structural equality for scalar types (integers must match exactly
/// rather than accepting any integral type) and for the broad categories we
/// only accept the same kind.  This prevents the generator from placing, say,
/// a `u8` in a context that expects `i32` (which Nitrate rejects).
pub(crate) fn types_compatible(a: &ast::Type, b: &ast::Type) -> bool {
    use ast::Type::*;
    match (a, b) {
        // Exact scalar matches — Nitrate does not implicitly coerce integers
        (Int8(_), Int8(_)) => true,
        (Int16(_), Int16(_)) => true,
        (Int32(_), Int32(_)) => true,
        (Int64(_), Int64(_)) => true,
        (Int128(_), Int128(_)) => true,
        (UInt8(_), UInt8(_)) => true,
        (UInt16(_), UInt16(_)) => true,
        (UInt32(_), UInt32(_)) => true,
        (UInt64(_), UInt64(_)) => true,
        (UInt128(_), UInt128(_)) => true,
        (USize(_), USize(_)) => true,
        (Bool(_), Bool(_)) => true,
        (Float32(_), Float32(_)) => true,
        (Float64(_), Float64(_)) => true,
        // Structural compatibility for compound types — array size matters
        (ArrayType(aa), ArrayType(ab)) => {
            // Check element type compatibility AND array length
            types_compatible(&aa.element_type, &ab.element_type) && expr_values_equal(&aa.len, &ab.len)
        }
        (SliceType(sa), SliceType(sb)) => types_compatible(&sa.element_type, &sb.element_type),
        (TupleType(ta), TupleType(tb)) if ta.element_types.len() == tb.element_types.len() => ta
            .element_types
            .iter()
            .zip(tb.element_types.iter())
            .all(|(fa, fb)| types_compatible(fa, fb)),
        (FunctionType(fa), FunctionType(fb)) if fa.parameters.len() == fb.parameters.len() => {
            let params_ok = fa
                .parameters
                .iter()
                .zip(fb.parameters.iter())
                .all(|(pa, pb)| types_compatible(&pa.ty, &pb.ty));
            let ret_ok = match (&fa.return_type, &fb.return_type) {
                (Some(ra), Some(rb)) => types_compatible(ra, rb),
                (None, None) => true,
                _ => false,
            };
            params_ok && ret_ok
        }
        (ReferenceType(ra), ReferenceType(rb)) => types_compatible(&ra.to, &rb.to),
        (PointerType(pa), PointerType(pb)) => types_compatible(&pa.to, &pb.to),
        // Type paths: compare by segment name (the only information we have)
        (TypePath(tpa), TypePath(tpb)) if tpa.segments.len() == tpb.segments.len() => tpa
            .segments
            .iter()
            .zip(tpb.segments.iter())
            .all(|(sa, sb)| sa.name == sb.name),
        (InferType(_), _) | (_, InferType(_)) => true,
        _ => false,
    }
}

/// Compare two AST expression values for equality (best-effort, for array length checking).
fn expr_values_equal(a: &ast::Expr, b: &ast::Expr) -> bool {
    match (a, b) {
        (ast::Expr::Integer(al), ast::Expr::Integer(bl)) => al.value == bl.value,
        _ => false,
    }
}

/// Returns `true` if a value of type `ty` can appear as the *source* of a
/// type-cast (`expr as Ty`).  In Nitrate only numeric-to-numeric and
/// integer-to-pointer casts are valid at the parse-tree level.
pub(crate) fn is_castable_source_type(ty: &ast::Type) -> bool {
    is_numeric_type(ty)
}

/// Returns `true` if a type can appear as the *target* of a type-cast.
/// Only numeric types are valid cast targets.
pub(crate) fn is_castable_target_type(ty: &ast::Type) -> bool {
    is_numeric_type(ty)
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
        _ => vec![],
    }
}

pub(crate) fn function_type_parts(ty: &ast::Type) -> (Vec<ast::Type>, ast::Type) {
    match ty {
        ast::Type::FunctionType(ft) => {
            let params: Vec<ast::Type> = ft.parameters.iter().map(|p| p.ty.clone()).collect();
            let ret = ft.return_type.clone().unwrap_or_else(unit_type);
            (params, ret)
        }
        _ => (vec![int32_type()], int32_type()),
    }
}

pub(crate) fn range_element_type(ty: &ast::Type) -> ast::Type {
    if is_integral_type(ty) { ty.clone() } else { int32_type() }
}

/// Pick a random numeric source type for a cast expression.
pub(crate) fn random_cast_source_type(generator: &mut Gen) -> ast::Type {
    // Build a list of candidate types, avoiding ones that could cause
    // value-range issues when cast.
    match generator.next_u64() % 8 {
        0 => int32_type(),
        1 => usize_type(),
        2 => float64_type(),
        3 => ast::Type::Float32(ast::Float32 {
            span: SrcSpan::default(),
        }),
        4 => ast::Type::UInt32(ast::UInt32 {
            span: SrcSpan::default(),
        }),
        5 => ast::Type::Int64(ast::Int64 {
            span: SrcSpan::default(),
        }),
        6 => ast::Type::Int16(ast::Int16 {
            span: SrcSpan::default(),
        }),
        _ => ast::Type::UInt8(ast::UInt8 {
            span: SrcSpan::default(),
        }),
    }
}

/// Returns the *signed* maximum value for integer types (half the unsigned max for signed types).
pub(crate) fn max_integer_value(ty: &ast::Type) -> u128 {
    match ty {
        ast::Type::Int8(_) => 127,
        ast::Type::UInt8(_) => 255,
        ast::Type::Int16(_) => 32767,
        ast::Type::UInt16(_) => 65535,
        ast::Type::Int32(_) => 2_147_483_647,
        ast::Type::UInt32(_) => 4_294_967_295,
        ast::Type::Int64(_) => 9_223_372_036_854_775_807,
        ast::Type::UInt64(_) | ast::Type::USize(_) => 18_446_744_073_709_551_615,
        ast::Type::Int128(_) => 170_141_183_460_469_231_731_687_303_715_884_105_727,
        ast::Type::UInt128(_) => u128::MAX,
        _ => u128::MAX,
    }
}

/// Produce a type-safe fallback expression for any type without using bogus casts.
/// Only handles primitive and simple compound types. Callers should avoid
/// reaching this for struct/ref/ptr/function types.
pub(crate) fn fallback_expr(ty: &ast::Type) -> ast::Expr {
    if is_bool_type(ty) {
        return ast::Expr::Boolean(ast::BooleanLit {
            span: SrcSpan::default(),
            value: false,
        });
    }
    if is_float_type(ty) {
        return ast::Expr::Float(ast::FloatLit {
            span: SrcSpan::default(),
            value: ordered_float::NotNan::new(0.0).unwrap(),
        });
    }
    if is_integral_type(ty) {
        return ast::Expr::Integer(Box::new(ast::IntegerLit {
            span: SrcSpan::default(),
            value: 0,
            kind: nitrate_translation::token::IntegerKind::Dec,
        }));
    }
    if is_unit_type(ty) {
        return ast::Expr::Tuple(Box::new(ast::Tuple {
            span: SrcSpan::default(),
            elements: vec![],
        }));
    }
    // For array types, generate an array literal of the correct size and element type.
    if let ast::Type::ArrayType(arr) = ty {
        let elem_fallback = fallback_expr(&arr.element_type);
        let len: usize = match &arr.len {
            ast::Expr::Integer(lit) => lit.value.try_into().unwrap_or(0),
            _ => 0,
        };
        let elements: Vec<ast::Expr> = (0..len).map(|_| elem_fallback.clone()).collect();
        return ast::Expr::List(Box::new(ast::List {
            span: SrcSpan::default(),
            elements,
        }));
    }
    // For slice types, generate an empty list.
    if matches!(ty, ast::Type::SliceType(_)) {
        return ast::Expr::List(Box::new(ast::List {
            span: SrcSpan::default(),
            elements: vec![],
        }));
    }
    // For tuple types, generate a tuple of fallback elements.
    if let ast::Type::TupleType(tup) = ty {
        let elements: Vec<ast::Expr> = tup.element_types.iter().map(|ft| fallback_expr(ft)).collect();
        return ast::Expr::Tuple(Box::new(ast::Tuple {
            span: SrcSpan::default(),
            elements,
        }));
    }
    // For struct/function/ref/ptr types, there's no valid fallback without Gen context.
    // Return unit as a last resort. The caller should detect this and adjust.
    ast::Expr::Tuple(Box::new(ast::Tuple {
        span: SrcSpan::default(),
        elements: vec![],
    }))
}
