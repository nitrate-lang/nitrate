use crate::Gen;
use nitrate_translation::parsetree::ast::{self, *};

impl Gen {
    /// Generate a random AST type. Uses seed-based deterministic RNG.
    pub(crate) fn gen_type(&mut self) -> ast::Type {
        match self.next_u64() % 18 {
            0 => self.gen_bool_type(),
            1 => self.gen_int_type(),
            2 => self.gen_uint_type(),
            3 => self.gen_float_type(),
            4 => self.gen_usize_type(),
            5 => self.gen_infer_type(),
            6 => self.gen_array_type(),
            7 => self.gen_slice_type(),
            8 => self.gen_tuple_type(),
            9 => self.gen_function_type(),
            10 => self.gen_reference_type(),
            11 => self.gen_pointer_type(),
            12 => self.gen_type_path(),
            13 => self.gen_refinement_type(),
            14 => self.gen_parentheses_type(),
            15 => self.gen_type_potential(),
            16 => self.gen_parentheses_type(),
            _ => self.gen_int_type(),
        }
    }

    // ─────────────────────────────────────────────────────────────────
    // Primitive type generators
    // ─────────────────────────────────────────────────────────────────

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

    fn gen_infer_type(&mut self) -> ast::Type {
        ast::Type::InferType(ast::InferType {
            span: SrcSpan::default(),
        })
    }

    // ─────────────────────────────────────────────────────────────────
    // Compound type generators
    // ─────────────────────────────────────────────────────────────────

    fn gen_array_type(&mut self) -> ast::Type {
        let element_type = self.gen_type();
        let len = ast::Expr::Integer(Box::new(ast::IntegerLit {
            span: SrcSpan::default(),
            value: (self.next_u64() % 100 + 1) as u128,
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
        let count = 1 + (self.next_u64() as usize % 5);
        let element_types: Vec<ast::Type> = (0..count).map(|_| self.gen_type()).collect();
        ast::Type::TupleType(Box::new(ast::TupleType {
            span: SrcSpan::default(),
            element_types,
        }))
    }

    fn gen_function_type(&mut self) -> ast::Type {
        let param_count = self.next_u64() as usize % 4;
        let parameters: Vec<ast::FuncTypeParam> = (0..param_count)
            .map(|i| ast::FuncTypeParam {
                span: SrcSpan::default(),
                attributes: None,
                name: format!("p_{i}").into(),
                ty: self.gen_type(),
            })
            .collect();
        let return_type = if self.next_bool() { Some(self.gen_type()) } else { None };
        ast::Type::FunctionType(Box::new(ast::FunctionType {
            span: SrcSpan::default(),
            attributes: None,
            parameters,
            return_type,
        }))
    }

    fn gen_reference_type(&mut self) -> ast::Type {
        let lifetime = if self.next_bool() {
            Some(ast::Lifetime {
                span: SrcSpan::default(),
                name: "a".into(),
            })
        } else {
            None
        };
        let exclusivity = if self.next_bool() {
            Some(if self.next_bool() {
                ast::Exclusivity::Iso
            } else {
                ast::Exclusivity::Poly
            })
        } else {
            None
        };
        let mutability = if self.next_bool() {
            Some(if self.next_bool() {
                ast::Mutability::Mut
            } else {
                ast::Mutability::Const
            })
        } else {
            None
        };
        let to = self.gen_type();
        ast::Type::ReferenceType(Box::new(ast::ReferenceType {
            span: SrcSpan::default(),
            lifetime,
            exclusivity,
            mutability,
            to,
        }))
    }

    fn gen_pointer_type(&mut self) -> ast::Type {
        let lifetime = if self.next_bool() {
            Some(ast::Lifetime {
                span: SrcSpan::default(),
                name: "a".into(),
            })
        } else {
            None
        };
        let exclusivity = if self.next_bool() {
            Some(if self.next_bool() {
                ast::Exclusivity::Iso
            } else {
                ast::Exclusivity::Poly
            })
        } else {
            None
        };
        let mutability = if self.next_bool() {
            Some(if self.next_bool() {
                ast::Mutability::Mut
            } else {
                ast::Mutability::Const
            })
        } else {
            None
        };
        let to = self.gen_type();
        ast::Type::PointerType(Box::new(ast::PointerType {
            span: SrcSpan::default(),
            lifetime,
            exclusivity,
            mutability,
            to,
        }))
    }

    fn gen_type_path(&mut self) -> ast::Type {
        // Use only registered struct names to guarantee type paths reference
        // types that are actually declared in the program output.
        if self.has_any_struct() {
            let seg_count = 1 + (self.next_u64() as usize % 3);
            let segments: Vec<ast::TypePathSegment> = (0..seg_count)
                .map(|_| {
                    let idx = self.gen_index(self.known_structs.len());
                    ast::TypePathSegment {
                        span: SrcSpan::default(),
                        name: self.known_structs[idx].clone(),
                        type_arguments: None,
                    }
                })
                .collect();
            ast::Type::TypePath(Box::new(ast::TypePath {
                span: SrcSpan::default(),
                segments,
                resolved_path: None,
            }))
        } else {
            // No structs registered yet; fall back to an integer type.
            self.gen_int_type()
        }
    }

    fn gen_refinement_type(&mut self) -> ast::Type {
        let basis_type = self.gen_int_type();
        let width = if self.next_bool() {
            Some(ast::Expr::Integer(Box::new(ast::IntegerLit {
                span: SrcSpan::default(),
                value: (self.next_u64() % 64 + 1) as u128,
                kind: nitrate_translation::token::IntegerKind::Dec,
            })))
        } else {
            None
        };
        ast::Type::RefinementType(Box::new(ast::RefinementType {
            span: SrcSpan::default(),
            basis_type,
            width,
            minimum: None,
            maximum: None,
        }))
    }

    fn gen_parentheses_type(&mut self) -> ast::Type {
        let inner = self.gen_type();
        ast::Type::Parentheses(Box::new(ast::TypeParentheses {
            span: SrcSpan::default(),
            inner,
        }))
    }

    fn gen_type_potential(&mut self) -> ast::Type {
        let body = ast::Block {
            span: SrcSpan::default(),
            safety: None,
            elements: vec![],
        };
        ast::Type::TypePotential(Box::new(ast::TypePotential {
            span: SrcSpan::default(),
            body,
        }))
    }
}
