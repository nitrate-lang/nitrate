use crate::ty::{
    arbitrary_type, bool_type, element_type_of, float64_type, function_type_parts, int32_type, is_bool_type,
    is_integral_type, is_numeric_type, is_type_path_like, is_type_path_or_typeof_target, random_cast_source_type,
    range_element_type, tuple_field_types, types_compatible, usize_type,
};
use crate::{FuncInfo, Gen, StructInfo, Symbol, SymbolKind};
use nitrate_translation::{
    nstring::NString,
    parsetree::ast::{self, *},
    token::IntegerKind,
};

impl Gen {
    /// Select a random rvalue kind **compatible** with the given target type
    /// **and** for which necessary symbols exist in scope.
    fn select_rvalue_kind(&mut self, ty: &ast::Type) -> ast::RValueKind {
        let mut compatible = compatible_kinds(ty, self);
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

        self.spend_budget(1);
        self.inc_rvalue_depth();

        let kind = self.select_rvalue_kind(ty);
        let expr = match kind {
            ast::RValueKind::SyntaxError => unreachable!(),
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

        self.dec_rvalue_depth();
        expr
    }

    // ── Leaf expression generators ──

    fn gen_rvalue_parentheses(&mut self, ty: &ast::Type) -> ast::Expr {
        let inner = self.gen_rvalue(ty);
        ast::Expr::Parentheses(Box::new(ast::ExprParentheses {
            span: SrcSpan::default(),
            inner,
        }))
    }

    fn gen_rvalue_boolean(&mut self, _ty: &ast::Type) -> ast::Expr {
        ast::Expr::Boolean(ast::BooleanLit {
            span: SrcSpan::default(),
            value: self.next_bool(),
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
        // Pick a struct whose name matches if possible, otherwise random struct.
        let struct_info = if let ast::Type::TypePath(tp) = ty {
            if let Some(seg) = tp.segments.first() {
                self.find_struct_by_name(&seg.name).cloned().unwrap_or_else(|| {
                    if self.has_any_struct() {
                        self.pick_struct()
                    } else {
                        // fallback — won't be reached if no structs
                        StructInfo {
                            name: "".to_string(),
                            fields: vec![],
                        }
                    }
                })
            } else if self.has_any_struct() {
                self.pick_struct()
            } else {
                return ast::Expr::Integer(Box::new(ast::IntegerLit {
                    span: SrcSpan::default(),
                    value: 0,
                    kind: IntegerKind::Dec,
                }));
            }
        } else if self.has_any_struct() {
            self.pick_struct()
        } else {
            return ast::Expr::Integer(Box::new(ast::IntegerLit {
                span: SrcSpan::default(),
                value: 0,
                kind: IntegerKind::Dec,
            }));
        };

        let path = self.make_single_segment_path(struct_info.name.clone());

        // Use struct's actual fields for correctness.
        let fields: Vec<(NString, ast::Expr)> = struct_info
            .fields
            .iter()
            .map(|(name, field_ty)| {
                let field_name: NString = name.clone().into();
                let val = self.gen_rvalue(field_ty);
                (field_name, val)
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
        let prev_in_loop = self.in_loop();
        self.push_frame();

        let stmt_count = 1 + self.gen_index(5);
        let mut elements = Vec::with_capacity(stmt_count + 1);
        for _ in 0..stmt_count {
            if self.next_bool() && !self.force_leaf() {
                let init_ty = if self.next_bool() { ty.clone() } else { arbitrary_type() };
                let init = self.gen_rvalue(&init_ty);
                let var_name_prefix = format!("v_{}", self.next_u64() & 0xFFF);
                self.add_local(var_name_prefix.clone(), init_ty.clone());
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
        self.set_in_loop(prev_in_loop);

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

        let prev_in_loop = self.in_loop();
        self.set_in_loop(false);

        self.push_frame();
        if let Some(ref params) = parameters {
            for p in params.iter() {
                let n: String = p.name.to_string();
                self.add_local(n, p.ty.clone());
            }
        }
        let body = self.gen_block_with_type(&return_ty);
        self.pop_frame();

        self.set_in_loop(prev_in_loop);

        ast::Expr::Closure(Box::new(ast::Closure {
            span: SrcSpan::default(),
            attributes: None,
            parameters,
            return_type: Some(return_ty),
            definition: body,
        }))
    }

    fn gen_rvalue_path(&mut self, ty: &ast::Type) -> ast::Expr {
        let path = self.pick_compatible_path(ty);
        ast::Expr::Path(Box::new(path))
    }

    fn pick_compatible_path(&mut self, ty: &ast::Type) -> ast::ExprPath {
        // Priority 1: Local variables of matching type
        if self.has_any_local() {
            let local_count = self.find_locals_by_type(ty).len();
            if local_count > 0 {
                let idx = self.gen_index(local_count);
                let compatible_locals = self.find_locals_by_type(ty);
                let name = compatible_locals[idx].name.clone();
                return self.make_single_segment_path(name);
            }
        }

        // Priority 2: Known struct names (type-level path)
        if self.has_any_struct() && is_type_path_or_typeof_target(ty) {
            let info = self.pick_struct();
            return self.make_single_segment_path(info.name);
        }

        // Priority 3: Function name
        if self.has_any_function() {
            let info = self.pick_function();
            return self.make_single_segment_path(info.name);
        }

        // Ultimate fallback
        self.make_single_segment_path("main".to_string())
    }

    fn gen_rvalue_index_access(&mut self, ty: &ast::Type) -> ast::Expr {
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
        // Extract field info first to avoid borrow conflicts.
        let field_info: Option<(String, Vec<(String, ast::Type)>)> =
            if let Some(info) = self.find_struct_by_type_path(ty) {
                if !info.fields.is_empty() {
                    Some((info.name.clone(), info.fields.clone()))
                } else {
                    None
                }
            } else {
                None
            };

        if let Some((struct_name, fields)) = field_info {
            let field_name = fields[0].0.clone();
            // Generate field values now that immutable borrow is over
            let field_exprs: Vec<(NString, ast::Expr)> = fields
                .iter()
                .map(|(n, ft)| {
                    let fname: NString = n.clone().into();
                    let val = self.gen_rvalue(ft);
                    (fname, val)
                })
                .collect();
            let object = ast::Expr::StructInit(Box::new(ast::StructInit {
                span: SrcSpan::default(),
                path: self.make_single_segment_path(struct_name),
                fields: field_exprs,
            }));
            return ast::Expr::FieldAccess(Box::new(ast::FieldAccess {
                span: SrcSpan::default(),
                object,
                field: field_name,
            }));
        }

        // Fallback: generate an integer
        ast::Expr::Integer(Box::new(ast::IntegerLit {
            span: SrcSpan::default(),
            value: 0,
            kind: IntegerKind::Dec,
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
        let prev_in_loop = self.in_loop();
        self.set_in_loop(true);

        let condition = if self.next_bool() {
            Some(self.gen_rvalue(&bool_type()))
        } else {
            // while with no condition = infinite loop
            None
        };
        let body = self.gen_block_with_type(ty);

        self.set_in_loop(prev_in_loop);
        ast::Expr::While(Box::new(ast::WhileLoop {
            span: SrcSpan::default(),
            condition,
            body,
        }))
    }

    fn gen_rvalue_match(&mut self, ty: &ast::Type) -> ast::Expr {
        // Generate match condition and type-compatible case patterns.
        // Use int32 conditions for simplicity (most compatible with case literals).
        let condition = self.gen_rvalue(&int32_type());
        let case_count = 1 + self.gen_index(4);
        let mut cases = Vec::with_capacity(case_count);
        for _ in 0..case_count {
            let case_val = ast::Expr::Integer(Box::new(ast::IntegerLit {
                span: SrcSpan::default(),
                value: self.next_u64() as u128,
                kind: IntegerKind::Dec,
            }));
            let body = self.gen_block_with_type(ty);
            cases.push(ast::MatchCase {
                span: SrcSpan::default(),
                condition: case_val,
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
        // Only emit `ret;` (without value) when in a void-returning context.
        let is_unit = matches!(ty, ast::Type::TupleType(t) if t.element_types.is_empty());
        let value = if is_unit && self.next_bool() {
            None
        } else {
            Some(self.gen_rvalue(ty))
        };
        ast::Expr::Return(Box::new(ast::Return {
            span: SrcSpan::default(),
            value,
        }))
    }

    fn gen_rvalue_foreach(&mut self, ty: &ast::Type) -> ast::Expr {
        let elem_ty = int32_type();
        let bindings: Vec<NString> = vec!["it_0".into()];
        let iterable_ty = ast::Type::SliceType(Box::new(ast::SliceType {
            span: SrcSpan::default(),
            element_type: elem_ty.clone(),
        }));
        let iterable = self.gen_rvalue(&iterable_ty);

        let prev_in_loop = self.in_loop();
        self.set_in_loop(true);

        self.push_frame();
        for binding in &bindings {
            let bname: String = binding.to_string();
            self.add_local(bname, elem_ty.clone());
        }
        let body = self.gen_block_with_type(ty);
        self.pop_frame();

        self.set_in_loop(prev_in_loop);

        ast::Expr::For(Box::new(ast::ForEach {
            span: SrcSpan::default(),
            attributes: None,
            bindings,
            iterable,
            body,
        }))
    }

    fn gen_rvalue_await(&mut self, ty: &ast::Type) -> ast::Expr {
        let future = self.gen_rvalue_function_call(ty);
        ast::Expr::Await(Box::new(ast::Await {
            span: SrcSpan::default(),
            future,
        }))
    }

    fn gen_rvalue_function_call(&mut self, ty: &ast::Type) -> ast::Expr {
        // Use a known function and generate type-compatible arguments.
        let func = if self.has_any_function() {
            self.pick_function()
        } else {
            return fallback_expr(ty);
        };
        let callee = self.make_single_segment_path(func.name.clone());
        let positional: Vec<ast::Expr> = func.params.iter().map(|param_ty| self.gen_rvalue(param_ty)).collect();
        ast::Expr::FunctionCall(Box::new(ast::FunctionCall {
            span: SrcSpan::default(),
            callee: ast::Expr::Path(Box::new(callee)),
            positional,
            named: Vec::new(),
        }))
    }

    fn gen_rvalue_method_call(&mut self, _ty: &ast::Type) -> ast::Expr {
        // Generate a method call on a struct instance.
        let struct_info = if self.has_any_struct() {
            self.pick_struct()
        } else {
            return ast::Expr::Integer(Box::new(ast::IntegerLit {
                span: SrcSpan::default(),
                value: 0,
                kind: IntegerKind::Dec,
            }));
        };
        let path = self.make_single_segment_path(struct_info.name.clone());
        let fields: Vec<(NString, ast::Expr)> = struct_info
            .fields
            .iter()
            .map(|(name, field_ty)| {
                let fname: NString = name.clone().into();
                let val = self.gen_rvalue(field_ty);
                (fname, val)
            })
            .collect();
        let object = ast::Expr::StructInit(Box::new(ast::StructInit {
            span: SrcSpan::default(),
            path,
            fields,
        }));
        let method_names = ["foo", "bar", "baz", "method", "call", "run", "apply", "transform"];
        let method_name = method_names[self.gen_index(method_names.len())].to_string();
        let arg_count = self.gen_index(3);
        let positional: Vec<ast::Expr> = (0..arg_count).map(|_| self.gen_rvalue(&arbitrary_type())).collect();
        ast::Expr::MethodCall(Box::new(ast::MethodCall {
            span: SrcSpan::default(),
            object,
            method_name,
            positional,
            named: Vec::new(),
        }))
    }

    // ── Symbol-aware helpers ──

    fn find_locals_by_type(&self, target_ty: &ast::Type) -> Vec<&Symbol> {
        let mut result = Vec::new();
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

    /// Find a struct whose type path matches the given TypePath type.
    fn find_struct_by_type_path(&self, ty: &ast::Type) -> Option<&StructInfo> {
        if let ast::Type::TypePath(tp) = ty {
            if let Some(seg) = tp.segments.first() {
                return self.find_struct_by_name(&seg.name);
            }
        }
        // If we have any struct, return one as a best-effort match.
        if self.has_any_struct() {
            Some(&self.known_structs[0])
        } else {
            None
        }
    }

    /// Generate a block with the given result type. Creates its own scope.
    fn gen_block_with_type(&mut self, ty: &ast::Type) -> ast::Block {
        let prev_in_loop = self.in_loop();
        self.push_frame();

        let stmt_count = self.gen_index(4);
        let mut elements = Vec::with_capacity(stmt_count + 1);
        for _ in 0..stmt_count {
            if self.next_bool() {
                let init_ty = arbitrary_type();
                let init = self.gen_rvalue(&init_ty);
                let var_name_prefix = format!("v_{}", self.next_u64() & 0xFFF);
                self.add_local(var_name_prefix.clone(), init_ty.clone());
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
        self.set_in_loop(prev_in_loop);

        ast::Block {
            span: SrcSpan::default(),
            safety: None,
            elements,
        }
    }
}

// ── RValue kind selection ──

fn compatible_kinds(ty: &ast::Type, generator: &Gen) -> Vec<ast::RValueKind> {
    use ast::RValueKind::*;

    if generator.force_leaf() {
        return leaf_kinds(ty, generator);
    }

    let mut kinds = vec![Parentheses, Block, If, Match, Cast];

    kinds.push(ForEach);

    // Path: only if we have named things to reference
    if generator.has_any_local() || generator.has_any_function() || generator.has_any_struct() {
        kinds.push(Path);
    }

    // FunctionCall: only if functions are known
    if generator.has_any_function() {
        kinds.push(FunctionCall);
    }

    // MethodCall: only if structs or locals exist
    if generator.has_any_struct() || generator.has_any_local() {
        kinds.push(MethodCall);
    }

    // StructInit / FieldAccess: only if structs exist AND target is a type path
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
        ast::Type::InferType(_) | ast::Type::TypePath(_) => {
            kinds.push(Boolean);
            kinds.push(Integer);
            kinds.push(Float);
            kinds.push(String);
            kinds.push(BString);
            kinds.push(TypeInfo);
            kinds.push(UnaryExpr);
            kinds.push(BinExpr);
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

    // Control-flow kinds
    if generator.in_loop() {
        kinds.push(Break);
        kinds.push(Continue);
    }
    kinds.push(Return);
    kinds.push(While);
    kinds.push(Await);

    kinds
}

fn leaf_kinds(ty: &ast::Type, generator: &Gen) -> Vec<ast::RValueKind> {
    use ast::RValueKind::*;
    let mut kinds = vec![];

    if generator.has_any_local() || generator.has_any_function() || generator.has_any_struct() {
        kinds.push(Path);
    }

    if is_bool_type(ty) {
        kinds.push(Boolean);
    } else if is_integral_type(ty) {
        kinds.push(Integer);
    } else if matches!(ty, ast::Type::Float32(_) | ast::Type::Float64(_)) {
        kinds.push(Integer);
        kinds.push(Float);
    } else {
        kinds.push(Boolean);
        kinds.push(Integer);
        kinds.push(Float);
        kinds.push(String);
    }

    if generator.in_loop() {
        kinds.push(Break);
        kinds.push(Continue);
    }
    kinds.push(Return);

    kinds
}

// ── Unary / Binary operator helpers ──

fn compatible_unary_ops(ty: &ast::Type) -> &'static [ast::UnaryExprOp] {
    use ast::UnaryExprOp::*;
    if is_bool_type(ty) {
        &[Not]
    } else if is_numeric_type(ty) {
        &[Add, Sub, Typeof]
    } else if matches!(ty, ast::Type::ReferenceType(_) | ast::Type::PointerType(_)) {
        &[Deref]
    } else {
        &[Add, Sub, Not, Deref, Borrow, Typeof]
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

fn compatible_binary_ops(ty: &ast::Type) -> &'static [ast::BinExprOp] {
    use ast::BinExprOp::*;
    if is_bool_type(ty) {
        &[LogicAnd, LogicOr, LogicEq, LogicNe, LogicLt, LogicGt, LogicLe, LogicGe]
    } else if is_integral_type(ty) {
        &[
            Add, Sub, Mul, Div, Mod, BitAnd, BitOr, BitXor, BitShl, BitShr, LogicLt, LogicGt, LogicLe, LogicGe,
            LogicEq, LogicNe,
        ]
    } else if matches!(ty, ast::Type::Float32(_) | ast::Type::Float64(_)) {
        &[Add, Sub, Mul, Div, LogicLt, LogicGt, LogicLe, LogicGe, LogicEq, LogicNe]
    } else {
        &[Add, Sub, LogicEq, LogicNe]
    }
}

fn binary_operand_types(op: ast::BinExprOp, result_ty: &ast::Type) -> (ast::Type, ast::Type) {
    use ast::BinExprOp::*;
    match op {
        LogicEq | LogicNe | LogicLt | LogicGt | LogicLe | LogicGe if is_bool_type(result_ty) => {
            (int32_type(), int32_type())
        }
        LogicEq | LogicNe | LogicLt | LogicGt | LogicLe | LogicGe => (result_ty.clone(), result_ty.clone()),
        LogicAnd | LogicOr => (bool_type(), bool_type()),
        _ => (result_ty.clone(), result_ty.clone()),
    }
}

// ── Fallback ──

fn fallback_expr(ty: &ast::Type) -> ast::Expr {
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
            kind: IntegerKind::Dec,
        }))
    }
}
