use crate::{Gen, Symbol, SymbolKind};
use nitrate_translation::{
    nstring::NString,
    parsetree::ast::{self, *},
    token::IntegerKind,
};
use std::unreachable;

impl Gen {
    /// Select a random rvalue kind that is **compatible** with the given target
    /// type **and** for which the necessary symbols exist in scope.
    fn select_rvalue_kind(&mut self, ty: &ast::Type) -> ast::RValueKind {
        let mut compatible = compatible_kinds(ty, self);
        // If compatible is somehow empty (shouldn't happen), fall back to Integer
        if compatible.is_empty() {
            return ast::RValueKind::Integer;
        }
        let idx = self.gen_index(compatible.len());
        compatible.swap_remove(idx)
    }

    /// Generate an expression whose type **must** match the given `ty`.
    pub(crate) fn gen_rvalue(&mut self, ty: &ast::Type) -> ast::Expr {
        if self.budget_left() == 0 {
            return fallback_expr(ty);
        }

        self.spend_budget();
        self.rvalue_depth += 1;

        let kind = self.select_rvalue_kind(ty);
        let expr = match kind {
            ast::RValueKind::SyntaxError => {
                unreachable!("select_rvalue_kind should not return SyntaxError")
            }
            ast::RValueKind::Parentheses => self.gen_rvalue_parentheses(ty),
            ast::RValueKind::Boolean => self.gen_rvalue_boolean(ty),
            ast::RValueKind::Integer => self.gen_rvalue_integer(ty),
            ast::RValueKind::Float => self.gen_rvalue_float(ty),
            ast::RValueKind::String => self.gen_rvalue_string(ty),
            ast::RValueKind::BString => self.gen_rvalue_bstring(ty),
            ast::RValueKind::TypeInfo => self.gen_rvalue_type_info(ty),
            ast::RValueKind::List => self.gen_rvalue_list(ty),
            ast::RValueKind::Tuple => self.gen_rvalue_tuple(ty),
            ast::RValueKind::StructInit => self.gen_rvalue_struct_init(ty),
            ast::RValueKind::UnaryExpr => self.gen_rvalue_unary_expr(ty),
            ast::RValueKind::BinExpr => self.gen_rvalue_bin_expr(ty),
            ast::RValueKind::Range => self.gen_rvalue_range(ty),
            ast::RValueKind::Cast => self.gen_rvalue_cast(ty),
            ast::RValueKind::Block => self.gen_rvalue_block(ty),
            ast::RValueKind::Closure => self.gen_rvalue_closure(ty),
            ast::RValueKind::Path => self.gen_rvalue_path(ty),
            ast::RValueKind::IndexAccess => self.gen_rvalue_index_access(ty),
            ast::RValueKind::FieldAccess => self.gen_rvalue_field_access(ty),
            ast::RValueKind::If => self.gen_rvalue_if(ty),
            ast::RValueKind::While => self.gen_rvalue_while(ty),
            ast::RValueKind::Match => self.gen_rvalue_match(ty),
            ast::RValueKind::Break => self.gen_rvalue_break(ty),
            ast::RValueKind::Continue => self.gen_rvalue_continue(ty),
            ast::RValueKind::Return => self.gen_rvalue_return(ty),
            ast::RValueKind::ForEach => self.gen_rvalue_foreach(ty),
            ast::RValueKind::Await => self.gen_rvalue_await(ty),
            ast::RValueKind::FunctionCall => self.gen_rvalue_function_call(ty),
            ast::RValueKind::MethodCall => self.gen_rvalue_method_call(ty),
        };

        self.rvalue_depth = self.rvalue_depth.saturating_sub(1);
        expr
    }

    // ─────────────────────────────────────────────────────────────────
    // Individual rvalue generators
    // ─────────────────────────────────────────────────────────────────

    fn gen_rvalue_parentheses(&mut self, ty: &ast::Type) -> ast::Expr {
        let inner = self.gen_rvalue(ty);
        ast::Expr::Parentheses(Box::new(ast::ExprParentheses {
            span: SrcSpan::default(),
            inner,
        }))
    }

    fn gen_rvalue_boolean(&mut self, _ty: &ast::Type) -> ast::Expr {
        let value = self.next_bool();
        ast::Expr::Boolean(ast::BooleanLit {
            span: SrcSpan::default(),
            value,
        })
    }

    fn gen_rvalue_integer(&mut self, _ty: &ast::Type) -> ast::Expr {
        let value = self.next_u64() as u128;
        ast::Expr::Integer(Box::new(ast::IntegerLit {
            span: SrcSpan::default(),
            value,
            kind: IntegerKind::Dec,
        }))
    }

    fn gen_rvalue_float(&mut self, _ty: &ast::Type) -> ast::Expr {
        let bits = self.next_u64();
        let raw = f64::from_bits((bits >> 8) | 0x3FF0000000000000);
        let value = ordered_float::NotNan::new(raw).unwrap_or(ordered_float::NotNan::new(1.0).unwrap());
        ast::Expr::Float(ast::FloatLit {
            span: SrcSpan::default(),
            value,
        })
    }

    fn gen_rvalue_string(&mut self, _ty: &ast::Type) -> ast::Expr {
        let len = 1 + (self.next_u64() as usize % 16);
        let mut s = String::with_capacity(len);
        for _ in 0..len {
            let c = (self.next_u64() as u8 % 95).wrapping_add(32);
            s.push(c as char);
        }
        s = s.replace('\\', "\\\\").replace('"', "\\\"");
        ast::Expr::String(ast::StringLit {
            span: SrcSpan::default(),
            value: s,
        })
    }

    fn gen_rvalue_bstring(&mut self, _ty: &ast::Type) -> ast::Expr {
        let len = 1 + (self.next_u64() as usize % 16);
        let mut bytes = Vec::with_capacity(len);
        for _ in 0..len {
            bytes.push(self.next_u64() as u8);
        }
        ast::Expr::BString(Box::new(ast::BStringLit {
            span: SrcSpan::default(),
            value: bytes,
        }))
    }

    fn gen_rvalue_type_info(&mut self, ty: &ast::Type) -> ast::Expr {
        ast::Expr::TypeInfo(Box::new(ast::TypeInfo {
            span: SrcSpan::default(),
            the: ty.clone(),
        }))
    }

    fn gen_rvalue_list(&mut self, ty: &ast::Type) -> ast::Expr {
        let elem_ty = element_type_of(ty);
        let len = self.gen_index(6);
        let elements: Vec<ast::Expr> = (0..len).map(|_| self.gen_rvalue(&elem_ty)).collect();
        ast::Expr::List(Box::new(ast::List {
            span: SrcSpan::default(),
            elements,
        }))
    }

    fn gen_rvalue_tuple(&mut self, ty: &ast::Type) -> ast::Expr {
        let field_types = tuple_field_types(ty);
        let elements: Vec<ast::Expr> = field_types.iter().map(|ft| self.gen_rvalue(ft)).collect();
        ast::Expr::Tuple(Box::new(ast::Tuple {
            span: SrcSpan::default(),
            elements,
        }))
    }

    fn gen_rvalue_struct_init(&mut self, ty: &ast::Type) -> ast::Expr {
        // Extract the struct name from ty if it's a TypePath, otherwise
        // pick a known struct name.
        let path = if let ast::Type::TypePath(tp) = ty {
            if let Some(first_seg) = tp.segments.first() {
                self.make_single_segment_path(first_seg.name.clone())
            } else {
                self.pick_struct_path()
            }
        } else {
            self.pick_struct_path()
        };
        // All seed structs and generated structs have exactly 4 fields:
        // field_0 through field_3 with i32 types. Generate matching fields.
        let fields: Vec<(NString, ast::Expr)> = (0..4)
            .map(|i| {
                let field_name: NString = format!("field_{i}").into();
                (field_name, self.gen_rvalue(&int32_type()))
            })
            .collect();
        ast::Expr::StructInit(Box::new(ast::StructInit {
            span: SrcSpan::default(),
            path,
            fields,
        }))
    }

    fn gen_rvalue_unary_expr(&mut self, ty: &ast::Type) -> ast::Expr {
        let ops = compatible_unary_ops(ty);
        let operator = ops[self.gen_index(ops.len())];
        let operand_ty = unary_operand_type(operator, ty);
        let operand = self.gen_rvalue(&operand_ty);
        ast::Expr::UnaryExpr(Box::new(ast::UnaryExpr {
            span: SrcSpan::default(),
            operator,
            operand,
        }))
    }

    fn gen_rvalue_bin_expr(&mut self, ty: &ast::Type) -> ast::Expr {
        let ops = compatible_binary_ops(ty);
        let op = ops[self.gen_index(ops.len())];
        let (left_ty, right_ty) = binary_operand_types(op, ty);
        let left = self.gen_rvalue(&left_ty);
        let right = self.gen_rvalue(&right_ty);
        ast::Expr::BinExpr(Box::new(ast::BinExpr {
            span: SrcSpan::default(),
            operator: op,
            left,
            right,
        }))
    }

    fn gen_rvalue_range(&mut self, ty: &ast::Type) -> ast::Expr {
        let kinds: &[ast::RangeKind] = &[
            ast::RangeKind::Range,
            ast::RangeKind::RangeInclusive,
            ast::RangeKind::RangeFrom,
            ast::RangeKind::RangeTo,
            ast::RangeKind::RangeToInclusive,
            ast::RangeKind::RangeFull,
        ];
        let kind = kinds[self.gen_index(kinds.len())];
        let bound_ty = range_element_type(ty);
        let (start, end) = match kind {
            ast::RangeKind::Range | ast::RangeKind::RangeInclusive => (
                Some(Box::new(self.gen_rvalue(&bound_ty))),
                Some(Box::new(self.gen_rvalue(&bound_ty))),
            ),
            ast::RangeKind::RangeFrom => (Some(Box::new(self.gen_rvalue(&bound_ty))), None),
            ast::RangeKind::RangeTo | ast::RangeKind::RangeToInclusive => {
                (None, Some(Box::new(self.gen_rvalue(&bound_ty))))
            }
            ast::RangeKind::RangeFull => (None, None),
        };
        ast::Expr::Range(Box::new(ast::Range {
            span: SrcSpan::default(),
            kind,
            start,
            end,
        }))
    }

    fn gen_rvalue_cast(&mut self, ty: &ast::Type) -> ast::Expr {
        let source_ty = random_cast_source_type(self);
        let value = self.gen_rvalue(&source_ty);
        ast::Expr::Cast(Box::new(ast::Cast {
            span: SrcSpan::default(),
            value,
            to: ty.clone(),
        }))
    }

    fn gen_rvalue_block(&mut self, ty: &ast::Type) -> ast::Expr {
        let prev_in_loop = self.in_loop;
        self.push_frame();

        let stmt_count = 1 + self.gen_index(5);
        let mut elements = Vec::with_capacity(stmt_count + 1);
        for _ in 0..stmt_count {
            if self.next_bool() && !self.force_leaf() {
                // Generate init first to prevent self-reference.
                let init_ty = if self.next_bool() { ty.clone() } else { arbitrary_type() };
                let init = self.gen_rvalue(&init_ty);
                let var_name_prefix = format!("v_{}", self.next_u64() & 0xFFF);
                self.add_local(var_name_prefix.clone(), init_ty.clone());
                // Retrieve the actual (potentially deduped) name from scope
                let actual_name = self
                    .frames
                    .last()
                    .and_then(|f| f.locals.last())
                    .map(|s| s.name.clone())
                    .unwrap_or(var_name_prefix);
                elements.push(ast::BlockItem::Variable(ast::LocalVariable {
                    span: SrcSpan::default(),
                    kind: ast::LocalVariableKind::Var,
                    attributes: None,
                    mutability: None,
                    name: actual_name.into(),
                    ty: Some(init_ty),
                    initializer: Some(init),
                }));
            } else {
                let stmt_ty = arbitrary_type();
                let expr = self.gen_rvalue(&stmt_ty);
                elements.push(ast::BlockItem::Stmt(expr));
            }
        }
        let final_expr = self.gen_rvalue(ty);
        elements.push(ast::BlockItem::Expr(final_expr));

        self.pop_frame();
        self.in_loop = prev_in_loop;

        ast::Expr::Block(Box::new(ast::Block {
            span: SrcSpan::default(),
            safety: None,
            elements,
        }))
    }

    fn gen_rvalue_closure(&mut self, ty: &ast::Type) -> ast::Expr {
        let (param_types, return_ty) = function_type_parts(ty);
        let parameters: Option<Vec<ast::FuncParam>> = if param_types.is_empty() {
            None
        } else {
            Some(
                param_types
                    .iter()
                    .enumerate()
                    .map(|(i, pt)| ast::FuncParam {
                        span: SrcSpan::default(),
                        attributes: None,
                        mutability: None,
                        name: format!("a_{i}").into(),
                        ty: pt.clone(),
                        default_value: None,
                    })
                    .collect(),
            )
        };

        // Closures get their own scope; break/continue from outer loop are
        // invalid inside a closure body.
        let prev_in_loop = self.in_loop;
        self.in_loop = false;

        self.push_frame();
        if let Some(ref params) = parameters {
            for p in params.iter() {
                let n: String = p.name.to_string();
                self.add_local(n, p.ty.clone());
            }
        }
        let body = self.gen_block_with_type(&return_ty);
        self.pop_frame();

        self.in_loop = prev_in_loop;

        ast::Expr::Closure(Box::new(ast::Closure {
            span: SrcSpan::default(),
            attributes: None,
            parameters,
            return_type: Some(return_ty),
            definition: body,
        }))
    }

    /// Generate a path expression. The name MUST refer to a symbol that
    /// exists in the current scope (local, function, or struct), and the
    /// type must be compatible with `ty`.
    fn gen_rvalue_path(&mut self, ty: &ast::Type) -> ast::Expr {
        let path = self.pick_compatible_path(ty);
        ast::Expr::Path(Box::new(path))
    }

    /// Pick a path expression that is compatible with the given type.
    /// Priority: 1) local variables of matching type, 2) function names
    /// (as callable values for function types), 3) struct names (for
    /// TypePath expressions — only when target type is a TypePath).
    fn pick_compatible_path(&mut self, ty: &ast::Type) -> ast::ExprPath {
        // Priority 1: Local variables of matching type
        let len = self.find_locals_by_type(ty).len();
        if len > 0 {
            let idx = self.gen_index(len);
            let compatible_locals = self.find_locals_by_type(ty);
            let name = compatible_locals[idx].name.clone();
            return self.make_single_segment_path(name);
        }

        // Priority 2: Known struct names (only valid as type paths, not
        // as runtime values). Return a TypePath expression only when the
        // target type expects a type-level expression.
        if self.has_any_struct() && is_type_path_or_typeof_target(ty) {
            let idx = self.gen_index(self.known_structs.len());
            let name = self.known_structs[idx].clone();
            return self.make_single_segment_path(name);
        }

        // Priority 3: Function name (only for function-typed targets)
        if self.has_any_function() {
            let idx = self.gen_index(self.known_functions.len());
            let name = self.known_functions[idx].clone();
            return self.make_single_segment_path(name);
        }

        // Ultimate fallback: use a struct name (will produce a valid
        // AST node even if the type doesn't match)
        if self.has_any_struct() {
            let idx = self.gen_index(self.known_structs.len());
            let name = self.known_structs[idx].clone();
            return self.make_single_segment_path(name);
        }

        // Last resort fallback: a known function name
        if self.has_any_function() {
            let name = "add".to_string();
            return self.make_single_segment_path(name);
        }

        unreachable!("gen_rvalue_path called with no known symbols");
    }

    fn gen_rvalue_index_access(&mut self, ty: &ast::Type) -> ast::Expr {
        // Index access on an array/slice produces the element type.
        // `ty` is the expected result type (element type).
        // Build collection_ty = SliceType(ty) so the index expr matches.
        let collection_ty = ast::Type::SliceType(Box::new(ast::SliceType {
            span: SrcSpan::default(),
            element_type: ty.clone(),
        }));
        let collection = self.gen_rvalue(&collection_ty);
        let index = self.gen_rvalue(&usize_type());
        ast::Expr::IndexAccess(Box::new(ast::IndexAccess {
            span: SrcSpan::default(),
            collection,
            index,
        }))
    }

    fn gen_rvalue_field_access(&mut self, ty: &ast::Type) -> ast::Expr {
        // Field access requires an object that is a struct instance.
        // Generate a struct init as the object, matching the target type.
        let object = if is_type_path_like(ty) {
            self.gen_rvalue_struct_init(ty)
        } else {
            // If ty isn't a type path, generate a struct init for a
            // known struct, then access one of its fields. The result
            // type won't match ty, but the AST is valid.
            let struct_path = self.pick_struct_path();
            let fields: Vec<(NString, ast::Expr)> = (0..4)
                .map(|i| {
                    let field_name: NString = format!("field_{i}").into();
                    (field_name, self.gen_rvalue(&int32_type()))
                })
                .collect();
            ast::Expr::StructInit(Box::new(ast::StructInit {
                span: SrcSpan::default(),
                path: struct_path,
                fields,
            }))
        };
        // field_0 always exists (all structs have at least 4 fields).
        let field = "field_0".to_string();
        ast::Expr::FieldAccess(Box::new(ast::FieldAccess {
            span: SrcSpan::default(),
            object,
            field,
        }))
    }

    fn gen_rvalue_if(&mut self, ty: &ast::Type) -> ast::Expr {
        let condition = self.gen_rvalue(&bool_type());
        let true_branch = self.gen_block_with_type(ty);
        let false_branch = if self.next_bool() {
            if self.next_bool() {
                Some(ast::ElseIf::If(Box::new(ast::If {
                    span: SrcSpan::default(),
                    condition: self.gen_rvalue(&bool_type()),
                    true_branch: self.gen_block_with_type(ty),
                    false_branch: None,
                })))
            } else {
                Some(ast::ElseIf::Block(self.gen_block_with_type(ty)))
            }
        } else {
            None
        };
        ast::Expr::If(Box::new(ast::If {
            span: SrcSpan::default(),
            condition,
            true_branch,
            false_branch,
        }))
    }

    fn gen_rvalue_while(&mut self, ty: &ast::Type) -> ast::Expr {
        let prev_in_loop = self.in_loop;
        self.in_loop = true; // break/continue now valid

        let condition = if self.next_bool() {
            Some(self.gen_rvalue(&bool_type()))
        } else {
            None
        };
        let body = self.gen_block_with_type(ty);

        self.in_loop = prev_in_loop;
        ast::Expr::While(Box::new(ast::WhileLoop {
            span: SrcSpan::default(),
            condition,
            body,
        }))
    }

    fn gen_rvalue_match(&mut self, ty: &ast::Type) -> ast::Expr {
        // Vary the match condition type and generate type-compatible cases.
        let (condition, case_gen): (ast::Expr, Box<dyn Fn(u64) -> ast::Expr>) = match self.next_u64() % 3 {
            0 => (
                self.gen_rvalue(&int32_type()),
                Box::new(|v| {
                    ast::Expr::Integer(Box::new(ast::IntegerLit {
                        span: SrcSpan::default(),
                        value: v as u128,
                        kind: IntegerKind::Dec,
                    }))
                }),
            ),
            1 => (
                self.gen_rvalue(&bool_type()),
                Box::new(|v| {
                    ast::Expr::Boolean(ast::BooleanLit {
                        span: SrcSpan::default(),
                        value: (v & 1) != 0,
                    })
                }),
            ),
            _ => (
                self.gen_rvalue(&integer_type()),
                Box::new(|v| {
                    ast::Expr::Integer(Box::new(ast::IntegerLit {
                        span: SrcSpan::default(),
                        value: v as u128,
                        kind: IntegerKind::Dec,
                    }))
                }),
            ),
        };
        let case_count = 1 + self.gen_index(4);
        let mut cases = Vec::with_capacity(case_count);
        for i in 0..case_count {
            let case_condition = case_gen(self.next_u64() ^ (i as u64));
            let body = self.gen_block_with_type(ty);
            cases.push(ast::MatchCase {
                span: SrcSpan::default(),
                condition: case_condition,
                body,
            });
        }
        let default_case = Some(self.gen_block_with_type(ty));
        ast::Expr::Match(Box::new(ast::Match {
            span: SrcSpan::default(),
            condition,
            cases,
            default_case,
        }))
    }

    fn gen_rvalue_break(&mut self, _ty: &ast::Type) -> ast::Expr {
        ast::Expr::Break(Box::new(ast::Break {
            span: SrcSpan::default(),
            label: None,
        }))
    }

    fn gen_rvalue_continue(&mut self, _ty: &ast::Type) -> ast::Expr {
        ast::Expr::Continue(Box::new(ast::Continue {
            span: SrcSpan::default(),
            label: None,
        }))
    }

    fn gen_rvalue_return(&mut self, ty: &ast::Type) -> ast::Expr {
        let value = if self.next_bool() {
            Some(self.gen_rvalue(ty))
        } else {
            None
        };
        ast::Expr::Return(Box::new(ast::Return {
            span: SrcSpan::default(),
            value,
        }))
    }

    fn gen_rvalue_foreach(&mut self, ty: &ast::Type) -> ast::Expr {
        // The result type of a foreach is the body type, typically unit.
        // The element type is derived from what's being iterated: if ty
        // is a collection, use its element type; otherwise use int32.
        let elem_ty = if is_collection_type(ty) {
            element_type_of(ty)
        } else {
            int32_type()
        };
        let bindings: Vec<NString> = vec!["it_0".into()];
        let iterable_ty = ast::Type::SliceType(Box::new(ast::SliceType {
            span: SrcSpan::default(),
            element_type: elem_ty.clone(),
        }));
        let iterable = self.gen_rvalue(&iterable_ty);

        let prev_in_loop = self.in_loop;
        self.in_loop = true;

        // Bindings added to scope before body
        self.push_frame();
        for binding in &bindings {
            let bname: String = binding.to_string();
            self.add_local(bname, elem_ty.clone());
        }
        let body = self.gen_block_with_type(ty);
        self.pop_frame();

        self.in_loop = prev_in_loop;

        ast::Expr::For(Box::new(ast::ForEach {
            span: SrcSpan::default(),
            attributes: None,
            bindings,
            iterable,
            body,
        }))
    }

    fn gen_rvalue_await(&mut self, ty: &ast::Type) -> ast::Expr {
        // Await is a best-effort generator: we produce an await expression
        // wrapping a function call. The called function may not return an
        // awaitable type, but the AST is valid.
        let future = self.gen_rvalue_function_call(ty);
        ast::Expr::Await(Box::new(ast::Await {
            span: SrcSpan::default(),
            future,
        }))
    }

    fn gen_rvalue_function_call(&mut self, ty: &ast::Type) -> ast::Expr {
        // Callee must be a known function name
        let callee = if self.has_any_function() {
            let idx = self.gen_index(self.known_functions.len());
            let name = self.known_functions[idx].clone();
            self.make_single_segment_path(name)
        } else {
            self.make_single_segment_path("add".to_string())
        };
        // Generate args with type-aware approach: if ty is a function type,
        // generate matching args; otherwise use arbitrary types.
        let (arg_types, _ret_ty) = function_type_parts(ty);
        let arg_count = self.gen_index(6).min(arg_types.len().max(1));
        let positional: Vec<ast::Expr> = if !arg_types.is_empty() && arg_count <= arg_types.len() {
            arg_types[..arg_count].iter().map(|at| self.gen_rvalue(at)).collect()
        } else {
            (0..arg_count).map(|_| self.gen_rvalue(&arbitrary_type())).collect()
        };
        let named = Vec::new();
        ast::Expr::FunctionCall(Box::new(ast::FunctionCall {
            span: SrcSpan::default(),
            callee: ast::Expr::Path(Box::new(callee)),
            positional,
            named,
        }))
    }

    fn gen_rvalue_method_call(&mut self, _ty: &ast::Type) -> ast::Expr {
        // Generate a struct init as the object so the method call has a
        // valid receiver (not a type name or literal).
        let struct_path = self.pick_struct_path();
        let fields: Vec<(NString, ast::Expr)> = (0..4)
            .map(|i| {
                let field_name: NString = format!("field_{i}").into();
                (field_name, self.gen_rvalue(&int32_type()))
            })
            .collect();
        let object = ast::Expr::StructInit(Box::new(ast::StructInit {
            span: SrcSpan::default(),
            path: struct_path,
            fields,
        }));
        let method_names = ["foo", "bar", "baz", "method", "call", "run", "apply", "transform"];
        let method_name = method_names[self.gen_index(method_names.len())].to_string();
        let arg_count = self.gen_index(4);
        let positional: Vec<ast::Expr> = (0..arg_count).map(|_| self.gen_rvalue(&arbitrary_type())).collect();
        let named = Vec::new();
        ast::Expr::MethodCall(Box::new(ast::MethodCall {
            span: SrcSpan::default(),
            object,
            method_name,
            positional,
            named,
        }))
    }

    // ─────────────────────────────────────────────────────────────────
    // Symbol-aware helpers
    // ─────────────────────────────────────────────────────────────────

    /// Find all locals across all frames whose type is structurally compatible
    /// with `target_ty`. Returns references to the Symbol entries.
    fn find_locals_by_type(&self, target_ty: &ast::Type) -> Vec<&Symbol> {
        let mut result = Vec::new();
        // Search from inner to outer scope
        for frame in self.frames.iter().rev() {
            for sym in &frame.locals {
                let SymbolKind::Local(local_ty) = &sym.kind;
                if types_compatible(local_ty, target_ty) {
                    result.push(sym);
                }
            }
        }
        result
    }

    /// Build a single-segment ExprPath from a name.
    fn make_single_segment_path(&self, name: String) -> ast::ExprPath {
        ast::ExprPath {
            span: SrcSpan::default(),
            segments: vec![ast::ExprPathSegment {
                span: SrcSpan::default(),
                name,
                type_arguments: None,
            }],
            resolved_path: None,
        }
    }

    /// Pick a path that references a known struct name.
    fn pick_struct_path(&mut self) -> ast::ExprPath {
        if self.has_any_struct() {
            let idx = self.gen_index(self.known_structs.len());
            let name = self.known_structs[idx].clone();
            self.make_single_segment_path(name)
        } else {
            self.make_single_segment_path("Vec".to_string())
        }
    }

    /// Generate a block with the given result type. Manages its own scope.
    fn gen_block_with_type(&mut self, ty: &ast::Type) -> ast::Block {
        let prev_in_loop = self.in_loop;
        self.push_frame();

        let stmt_count = self.gen_index(4);
        let mut elements = Vec::with_capacity(stmt_count + 1);
        for _ in 0..stmt_count {
            if self.next_bool() {
                // Generate init first so it can't reference the variable
                let init_ty = arbitrary_type();
                let init = self.gen_rvalue(&init_ty);
                let var_name_prefix = format!("v_{}", self.next_u64() & 0xFFF);
                self.add_local(var_name_prefix.clone(), init_ty.clone());
                // Retrieve the actual (potentially deduped) name
                let actual_name = self
                    .frames
                    .last()
                    .and_then(|f| f.locals.last())
                    .map(|s| s.name.clone())
                    .unwrap_or(var_name_prefix);
                elements.push(ast::BlockItem::Variable(ast::LocalVariable {
                    span: SrcSpan::default(),
                    kind: ast::LocalVariableKind::Var,
                    attributes: None,
                    mutability: None,
                    name: actual_name.into(),
                    ty: Some(init_ty),
                    initializer: Some(init),
                }));
            } else {
                let expr = self.gen_rvalue(&arbitrary_type());
                elements.push(ast::BlockItem::Stmt(expr));
            }
        }
        let final_expr = self.gen_rvalue(ty);
        elements.push(ast::BlockItem::Expr(final_expr));

        self.pop_frame();
        self.in_loop = prev_in_loop;

        ast::Block {
            span: SrcSpan::default(),
            safety: None,
            elements,
        }
    }
}

// ─────────────────────────────────────────────────────────────────
// Type compatibility & inspection helpers
// ─────────────────────────────────────────────────────────────────

/// Return the list of rvalue kinds that can produce a value of type `ty`.
/// Gated by symbol availability — kinds that reference named symbols are
/// only included when the corresponding names are known.
fn compatible_kinds(ty: &ast::Type, generator: &Gen) -> Vec<ast::RValueKind> {
    use ast::RValueKind::*;

    if generator.force_leaf() {
        return leaf_kinds(ty, generator);
    }

    // Base kinds valid for nearly all types
    let mut kinds = vec![Parentheses, Block, If, Match, Cast];

    // ForEach only if we have anything to iterate over
    if generator.has_any_local() || generator.has_any_function() || generator.has_any_struct() {
        kinds.push(ForEach);
    }

    // Path: only if we have locals, functions, or structs to reference
    if generator.has_any_local() || generator.has_any_function() || generator.has_any_struct() {
        kinds.push(Path);
    }

    // FunctionCall: only if functions are known
    if generator.has_any_function() {
        kinds.push(FunctionCall);
    }

    // MethodCall: only if structs or locals exist (methods are on instances)
    if generator.has_any_struct() || generator.has_any_local() {
        kinds.push(MethodCall);
    }

    // StructInit and FieldAccess: only when target type is a TypePath
    // (structs are created by name, fields are accessed on struct instances)
    if generator.has_any_struct() && is_type_path_like(ty) {
        kinds.push(StructInit);
        kinds.push(FieldAccess);
    }

    match ty {
        ast::Type::Bool(_) => {
            kinds.push(Boolean);
            kinds.push(UnaryExpr);
            kinds.push(BinExpr);
        }
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
        | ast::Type::USize(_) => {
            kinds.push(Integer);
            kinds.push(UnaryExpr);
            kinds.push(BinExpr);
            kinds.push(Range);
        }
        ast::Type::Float32(_) | ast::Type::Float64(_) => {
            kinds.push(Float);
            kinds.push(Integer);
            kinds.push(UnaryExpr);
            kinds.push(BinExpr);
        }
        ast::Type::ArrayType(_) | ast::Type::SliceType(_) => {
            kinds.push(List);
            kinds.push(IndexAccess);
        }
        ast::Type::TupleType(_) => {
            kinds.push(Tuple);
        }
        ast::Type::FunctionType(_) => {
            kinds.push(Closure);
        }
        ast::Type::ReferenceType(_) | ast::Type::PointerType(_) => {
            kinds.push(UnaryExpr);
        }
        ast::Type::InferType(_) => {
            kinds.push(Boolean);
            kinds.push(Integer);
            kinds.push(Float);
            kinds.push(BString);
            kinds.push(TypeInfo);
        }
        // TypePath targets can use StructInit and FieldAccess
        ast::Type::TypePath(_) => {
            kinds.push(Boolean);
            kinds.push(Integer);
            kinds.push(Float);
            kinds.push(BString);
            kinds.push(TypeInfo);
            kinds.push(UnaryExpr);
            kinds.push(BinExpr);
            // StructInit and FieldAccess already added above if has_any_struct
        }
        _ => {
            kinds.push(Boolean);
            kinds.push(Integer);
            kinds.push(Float);
            kinds.push(String);
            kinds.push(BString);
            kinds.push(TypeInfo);
            kinds.push(UnaryExpr);
            kinds.push(BinExpr);
        }
    }

    // Control-flow / statement kinds (result type irrelevant).
    // Only emit Break/Continue when inside a loop.
    if generator.in_loop {
        kinds.push(Break);
        kinds.push(Continue);
    }
    kinds.push(Return);
    kinds.push(While);
    kinds.push(Await);

    kinds
}

/// Leaf-only (non-recursive) kinds appropriate for type `ty`.
fn leaf_kinds(ty: &ast::Type, generator: &Gen) -> Vec<ast::RValueKind> {
    use ast::RValueKind::*;
    let mut kinds = vec![];

    if generator.has_any_local() || generator.has_any_function() || generator.has_any_struct() {
        kinds.push(Path);
    }

    match ty {
        ast::Type::Bool(_) => {
            kinds.push(Boolean);
        }
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
        | ast::Type::USize(_) => {
            kinds.push(Integer);
        }
        ast::Type::Float32(_) | ast::Type::Float64(_) => {
            kinds.push(Integer);
            kinds.push(Float);
        }
        ast::Type::InferType(_) => {
            kinds.push(Boolean);
            kinds.push(Integer);
            kinds.push(Float);
        }
        _ => {
            kinds.push(Boolean);
            kinds.push(Integer);
            kinds.push(Float);
            kinds.push(String);
        }
    }

    // Even in leaf mode, control flow is fine (they don't recurse deeply)
    if generator.in_loop {
        kinds.push(Break);
        kinds.push(Continue);
    }
    kinds.push(Return);

    kinds
}

/// Unary operators compatible with producing `ty` as a result.
fn compatible_unary_ops(ty: &ast::Type) -> &'static [ast::UnaryExprOp] {
    use ast::UnaryExprOp::*;
    match ty {
        ast::Type::Bool(_) => &[Not],
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
        | ast::Type::Float32(_)
        | ast::Type::Float64(_) => &[Add, Sub, Typeof],
        ast::Type::ReferenceType(_) | ast::Type::PointerType(_) => &[Deref],
        _ => &[Add, Sub, Not, Deref, Borrow, Typeof],
    }
}

fn unary_operand_type(op: ast::UnaryExprOp, result_ty: &ast::Type) -> ast::Type {
    match op {
        ast::UnaryExprOp::Not => bool_type(),
        ast::UnaryExprOp::Deref => ast::Type::ReferenceType(Box::new(ast::ReferenceType {
            span: SrcSpan::default(),
            lifetime: None,
            exclusivity: None,
            mutability: None,
            to: result_ty.clone(),
        })),
        ast::UnaryExprOp::Borrow => match result_ty {
            ast::Type::ReferenceType(r) => r.to.clone(),
            ast::Type::PointerType(p) => p.to.clone(),
            _ => int32_type(),
        },
        ast::UnaryExprOp::Typeof => arbitrary_type(),
        _ => result_ty.clone(),
    }
}

/// Logical operators (LogicAnd, LogicOr) are only valid on bool types.
/// They've been removed from numeric operator sets and only appear here.
fn compatible_binary_ops(ty: &ast::Type) -> &'static [ast::BinExprOp] {
    use ast::BinExprOp::*;
    match ty {
        ast::Type::Bool(_) => &[LogicAnd, LogicOr, LogicEq, LogicNe, LogicLt, LogicGt, LogicLe, LogicGe],
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
        | ast::Type::USize(_) => &[
            Add, Sub, Mul, Div, Mod, BitAnd, BitOr, BitXor, BitShl, BitShr, LogicLt, LogicGt, LogicLe, LogicGe,
            LogicEq, LogicNe,
        ],
        ast::Type::Float32(_) | ast::Type::Float64(_) => {
            &[Add, Sub, Mul, Div, LogicLt, LogicGt, LogicLe, LogicGe, LogicEq, LogicNe]
        }
        _ => &[Add, Sub, Mul, Div, LogicEq, LogicNe, LogicLt, LogicGt, LogicLe, LogicGe],
    }
}

fn binary_operand_types(op: ast::BinExprOp, result_ty: &ast::Type) -> (ast::Type, ast::Type) {
    use ast::BinExprOp::*;
    match op {
        // Comparison operators produce bool but compare operand types
        LogicEq | LogicNe | LogicLt | LogicGt | LogicLe | LogicGe if is_bool_type(result_ty) => {
            (int32_type(), int32_type())
        }
        LogicEq | LogicNe | LogicLt | LogicGt | LogicLe | LogicGe => (result_ty.clone(), result_ty.clone()),
        LogicAnd | LogicOr => (bool_type(), bool_type()),
        _ => (result_ty.clone(), result_ty.clone()),
    }
}

/// Emergency fallback expression for budget exhaustion.
fn fallback_expr(ty: &ast::Type) -> ast::Expr {
    match ty {
        ast::Type::Bool(_) => ast::Expr::Boolean(ast::BooleanLit {
            span: SrcSpan::default(),
            value: false,
        }),
        ast::Type::Float32(_) | ast::Type::Float64(_) => ast::Expr::Float(ast::FloatLit {
            span: SrcSpan::default(),
            value: ordered_float::NotNan::new(0.0).unwrap(),
        }),
        _ => ast::Expr::Integer(Box::new(ast::IntegerLit {
            span: SrcSpan::default(),
            value: 0,
            kind: IntegerKind::Dec,
        })),
    }
}

fn is_bool_type(ty: &ast::Type) -> bool {
    matches!(ty, ast::Type::Bool(_))
}

fn is_collection_type(ty: &ast::Type) -> bool {
    matches!(ty, ast::Type::ArrayType(_) | ast::Type::SliceType(_))
}

/// Check if a type is a TypePath (for struct init/field access gating).
fn is_type_path_like(ty: &ast::Type) -> bool {
    matches!(ty, ast::Type::TypePath(_))
}

/// Check if a type is used in a context where a type-level expression
/// (type name) makes sense.
fn is_type_path_or_typeof_target(ty: &ast::Type) -> bool {
    matches!(
        ty,
        ast::Type::TypePath(_) | ast::Type::InferType(_) | ast::Type::TypePotential(_)
    )
}

/// Two types are compatible if they have the same discriminant.
/// This is a structural compatibility check for expression generation.
fn types_compatible(a: &ast::Type, b: &ast::Type) -> bool {
    std::mem::discriminant(a) == std::mem::discriminant(b)
}

fn element_type_of(ty: &ast::Type) -> ast::Type {
    match ty {
        ast::Type::ArrayType(arr) => arr.element_type.clone(),
        ast::Type::SliceType(slice) => slice.element_type.clone(),
        _ => int32_type(),
    }
}

fn tuple_field_types(ty: &ast::Type) -> Vec<ast::Type> {
    match ty {
        ast::Type::TupleType(tup) => tup.element_types.clone(),
        _ => (0..2).map(|_| int32_type()).collect(),
    }
}

fn function_type_parts(ty: &ast::Type) -> (Vec<ast::Type>, ast::Type) {
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

fn range_element_type(ty: &ast::Type) -> ast::Type {
    if is_integral_type(ty) { ty.clone() } else { int32_type() }
}

// ─────────────────────────────────────────────────────────────────
// Convenience type constructors
// ─────────────────────────────────────────────────────────────────

fn bool_type() -> ast::Type {
    ast::Type::Bool(ast::Bool {
        span: SrcSpan::default(),
    })
}

fn int32_type() -> ast::Type {
    ast::Type::Int32(ast::Int32 {
        span: SrcSpan::default(),
    })
}

fn integer_type() -> ast::Type {
    ast::Type::Int32(ast::Int32 {
        span: SrcSpan::default(),
    })
}

fn usize_type() -> ast::Type {
    ast::Type::USize(ast::USize {
        span: SrcSpan::default(),
    })
}

fn is_integral_type(ty: &ast::Type) -> bool {
    match ty {
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
        | ast::Type::USize(_) => true,
        _ => false,
    }
}

fn arbitrary_type() -> ast::Type {
    int32_type()
}

/// Pick a random non-void type for cast source.
fn random_cast_source_type(generator: &mut Gen) -> ast::Type {
    match generator.next_u64() % 8 {
        0 => int32_type(),
        1 => bool_type(),
        2 => usize_type(),
        3 => ast::Type::Float64(ast::Float64 {
            span: SrcSpan::default(),
        }),
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
