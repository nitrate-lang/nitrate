use crate::Gen;
use nitrate_translation::parsetree::ast::{self, *};
use nitrate_translation::token::IntegerKind;
use std::unreachable;

impl Gen {
    /// Select a random rvalue kind that is **compatible** with the given target type.
    /// When `force_leaf()` is true, only simple literal/variable expressions
    /// are considered to ensure generation terminates.
    fn select_rvalue_kind(&mut self, ty: &ast::Type) -> ast::RValueKind {
        let mut compatible = compatible_kinds(ty, self.force_leaf());
        // Weighted selection from compatible kinds
        let idx = self.gen_index(compatible.len());
        // RValueKind is not Copy, so we use swap_remove for efficiency
        compatible.swap_remove(idx)
    }

    /// Generate an expression whose type **must** match the given `ty`.
    pub(crate) fn gen_rvalue(&mut self, ty: &ast::Type) -> ast::Expr {
        if self.budget == 0 {
            // Emergency fallback: produce a zero literal of the appropriate kind
            return fallback_expr(ty);
        }

        self.budget = self.budget.saturating_sub(1);
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
    // Individual rvalue generators — each receives the target type `ty`
    // and MUST produce an expression compatible with that type.
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

    fn gen_rvalue_integer(&mut self, ty: &ast::Type) -> ast::Expr {
        // Clamp value based on integer width
        let kind = infer_integer_kind(ty);
        let value = match ty {
            ast::Type::Int8(_) | ast::Type::UInt8(_) => (self.next_u64() & 0xFF) as u128,
            ast::Type::Int16(_) | ast::Type::UInt16(_) => (self.next_u64() & 0xFFFF) as u128,
            ast::Type::Int32(_) | ast::Type::UInt32(_) => (self.next_u64() & 0xFFFF_FFFF) as u128,
            ast::Type::Int64(_) | ast::Type::UInt64(_) | ast::Type::USize(_) => self.next_u64() as u128,
            _ => self.next_u64() as u128,
        };
        ast::Expr::Integer(Box::new(ast::IntegerLit {
            span: SrcSpan::default(),
            value,
            kind,
        }))
    }

    fn gen_rvalue_float(&mut self, ty: &ast::Type) -> ast::Expr {
        // Generate a float compatible with f32 or f64
        let bits = self.next_u64();
        let raw = f64::from_bits((bits >> 8) | 0x3FF0000000000000); // clamp to ~[1.0, 4.0)
        let val = ordered_float::NotNan::new(raw).unwrap_or(ordered_float::NotNan::new(1.0).unwrap());
        // If target is f32, truncate to f32 precision then back to f64
        let value = match ty {
            ast::Type::Float32(_) => ordered_float::NotNan::new(val.into_inner() as f32 as f64).unwrap(),
            _ => val,
        };
        ast::Expr::Float(ast::FloatLit {
            span: SrcSpan::default(),
            value,
        })
    }

    fn gen_rvalue_string(&mut self, ty: &ast::Type) -> ast::Expr {
        let _ = ty;
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

    fn gen_rvalue_bstring(&mut self, ty: &ast::Type) -> ast::Expr {
        let _ = ty;
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
        // typeinfo always evaluates to Type at compile time,
        // so the target type should be Type or a generic
        let the = ty.clone();
        ast::Expr::TypeInfo(Box::new(ast::TypeInfo {
            span: SrcSpan::default(),
            the,
        }))
    }

    fn gen_rvalue_list(&mut self, ty: &ast::Type) -> ast::Expr {
        // Extract element type from the target type
        let elem_ty = element_type_of(ty);
        let len = self.gen_index(8);
        let elements: Vec<ast::Expr> = (0..len).map(|_| self.gen_rvalue(&elem_ty)).collect();
        ast::Expr::List(Box::new(ast::List {
            span: SrcSpan::default(),
            elements,
        }))
    }

    fn gen_rvalue_tuple(&mut self, ty: &ast::Type) -> ast::Expr {
        // Extract field types from tuple type, or generate random
        let field_types = tuple_field_types(ty);
        let elements: Vec<ast::Expr> = field_types.iter().map(|ft| self.gen_rvalue(ft)).collect();
        ast::Expr::Tuple(Box::new(ast::Tuple {
            span: SrcSpan::default(),
            elements,
        }))
    }

    fn gen_rvalue_struct_init(&mut self, _ty: &ast::Type) -> ast::Expr {
        let path = self.gen_expr_path();
        let field_count = 1 + self.gen_index(5);
        let fields: Vec<(_, ast::Expr)> = (0..field_count)
            .map(|i| {
                let field_name = format!("field_{i}").into();
                let int_ty = int32_type();
                (field_name, self.gen_rvalue(&int_ty))
            })
            .collect();
        ast::Expr::StructInit(Box::new(ast::StructInit {
            span: SrcSpan::default(),
            path,
            fields,
        }))
    }

    fn gen_rvalue_unary_expr(&mut self, ty: &ast::Type) -> ast::Expr {
        // Select a unary operator whose result type matches `ty`
        let ops = compatible_unary_ops(ty);
        let operator = ops[self.gen_index(ops.len())];
        // Determine the operand type required by this operator
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
        // Determine operand types: for comparison/logical ops the operands
        // can differ from the result type; for arithmetic they match.
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
        // Range bounds match the element type (if iterable) or Int32
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
        // Cast TO `ty` FROM some other type
        let source_ty = arbitrary_non_void_type();
        let value = self.gen_rvalue(&source_ty);
        ast::Expr::Cast(Box::new(ast::Cast {
            span: SrcSpan::default(),
            value,
            to: ty.clone(),
        }))
    }

    fn gen_rvalue_block(&mut self, ty: &ast::Type) -> ast::Expr {
        let stmt_count = 1 + self.gen_index(5);
        let mut elements = Vec::with_capacity(stmt_count + 1);
        for _ in 0..stmt_count {
            if self.next_bool() && !self.force_leaf() {
                let var_name = format!("v_{}", self.next_u64() & 0xFFF);
                let init_ty = if self.next_bool() { ty.clone() } else { arbitrary_type() };
                let init = self.gen_rvalue(&init_ty);
                elements.push(ast::BlockItem::Variable(ast::LocalVariable {
                    span: SrcSpan::default(),
                    kind: ast::LocalVariableKind::Var,
                    attributes: None,
                    mutability: None,
                    name: var_name.into(),
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
        ast::Expr::Block(Box::new(ast::Block {
            span: SrcSpan::default(),
            safety: None,
            elements,
        }))
    }

    fn gen_rvalue_closure(&mut self, ty: &ast::Type) -> ast::Expr {
        // Closure must match a function type. Extract param/return types.
        let (param_types, return_ty) = function_type_parts(ty);
        let parameters = if param_types.is_empty() {
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
        let body = self.gen_block_with_type(&return_ty);
        ast::Expr::Closure(Box::new(ast::Closure {
            span: SrcSpan::default(),
            attributes: None,
            parameters,
            return_type: Some(return_ty),
            definition: body,
        }))
    }

    fn gen_rvalue_path(&mut self, _ty: &ast::Type) -> ast::Expr {
        // If we have locals in scope, sometimes reference one
        if !self.frames.is_empty() && self.next_bool() {
            // Pre-compute index before borrowing self.frames immutably
            let frame_len = self.frames.last().unwrap().locals.len();
            let maybe_name = if frame_len > 0 {
                let idx = self.gen_index(frame_len);
                // Now do the immutable borrow to get the name
                let frame = self.frames.last().unwrap();
                let locals: Vec<&String> = frame.locals.iter().collect();
                Some(locals[idx].clone())
            } else {
                None
            };
            if let Some(name) = maybe_name {
                return ast::Expr::Path(Box::new(ast::ExprPath {
                    span: SrcSpan::default(),
                    segments: vec![ast::ExprPathSegment {
                        span: SrcSpan::default(),
                        name,
                        type_arguments: None,
                    }],
                    resolved_path: None,
                }));
            }
        }
        ast::Expr::Path(Box::new(self.gen_expr_path()))
    }

    fn gen_rvalue_index_access(&mut self, ty: &ast::Type) -> ast::Expr {
        // Index access on some collection; result type = element type
        let collection_ty = if is_collection_type(ty) {
            ty.clone()
        } else {
            // Wrap in a slice type so the index result is `ty`
            ast::Type::SliceType(Box::new(ast::SliceType {
                span: SrcSpan::default(),
                element_type: ty.clone(),
            }))
        };
        let collection = self.gen_rvalue(&collection_ty);
        let index = self.gen_rvalue(&usize_type());
        ast::Expr::IndexAccess(Box::new(ast::IndexAccess {
            span: SrcSpan::default(),
            collection,
            index,
        }))
    }

    fn gen_rvalue_field_access(&mut self, ty: &ast::Type) -> ast::Expr {
        // Field access on a struct; the object should be a struct type
        let object = self.gen_rvalue_path(ty);
        let field = format!("field_{}", self.gen_index(10));
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
        let condition = if self.next_bool() {
            Some(self.gen_rvalue(&bool_type()))
        } else {
            None
        };
        let body = self.gen_block_with_type(ty);
        ast::Expr::While(Box::new(ast::WhileLoop {
            span: SrcSpan::default(),
            condition,
            body,
        }))
    }

    fn gen_rvalue_match(&mut self, ty: &ast::Type) -> ast::Expr {
        // Match on an integer expression
        let condition = self.gen_rvalue(&int32_type());
        let case_count = 1 + self.gen_index(4);
        let mut cases = Vec::with_capacity(case_count);
        for _ in 0..case_count {
            let case_condition = ast::Expr::Integer(Box::new(ast::IntegerLit {
                span: SrcSpan::default(),
                value: self.next_u64() as u128,
                kind: IntegerKind::Dec,
            }));
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
        // Iterate over a collection whose element type is derived from ty
        let elem_ty = if is_collection_type(ty) {
            element_type_of(ty)
        } else {
            int32_type()
        };
        let binding_count = 1 + self.gen_index(2);
        let bindings: Vec<_> = (0..binding_count).map(|i| format!("it_{i}").into()).collect();
        // The iterable should be a collection of elem_ty
        let iterable_ty = ast::Type::SliceType(Box::new(ast::SliceType {
            span: SrcSpan::default(),
            element_type: elem_ty,
        }));
        let iterable = self.gen_rvalue(&iterable_ty);
        let body = self.gen_block_with_type(ty);
        ast::Expr::For(Box::new(ast::ForEach {
            span: SrcSpan::default(),
            attributes: None,
            bindings,
            iterable,
            body,
        }))
    }

    fn gen_rvalue_await(&mut self, ty: &ast::Type) -> ast::Expr {
        // Await a future; the future's output should be `ty`
        let future = self.gen_rvalue_function_call(ty);
        ast::Expr::Await(Box::new(ast::Await {
            span: SrcSpan::default(),
            future,
        }))
    }

    fn gen_rvalue_function_call(&mut self, ty: &ast::Type) -> ast::Expr {
        let callee = self.gen_rvalue_path(ty);
        let arg_count = self.gen_index(6);
        let positional: Vec<ast::Expr> = (0..arg_count).map(|_| self.gen_rvalue(&arbitrary_type())).collect();
        let named = Vec::new();
        ast::Expr::FunctionCall(Box::new(ast::FunctionCall {
            span: SrcSpan::default(),
            callee,
            positional,
            named,
        }))
    }

    fn gen_rvalue_method_call(&mut self, ty: &ast::Type) -> ast::Expr {
        let object = self.gen_rvalue_path(ty);
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
    // Helper methods
    // ─────────────────────────────────────────────────────────────────

    fn gen_block_with_type(&mut self, ty: &ast::Type) -> ast::Block {
        let stmt_count = self.gen_index(4);
        let mut elements = Vec::with_capacity(stmt_count + 1);
        for _ in 0..stmt_count {
            if self.next_bool() {
                let var_name = format!("v_{}", self.next_u64() & 0xFFF);
                let init_ty = arbitrary_type();
                let init = self.gen_rvalue(&init_ty);
                elements.push(ast::BlockItem::Variable(ast::LocalVariable {
                    span: SrcSpan::default(),
                    kind: ast::LocalVariableKind::Var,
                    attributes: None,
                    mutability: None,
                    name: var_name.into(),
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
        ast::Block {
            span: SrcSpan::default(),
            safety: None,
            elements,
        }
    }

    fn gen_expr_path(&mut self) -> ast::ExprPath {
        let seg_count = 1 + self.gen_index(3);
        let segment_names = [
            "a", "b", "c", "x", "y", "z", "foo", "bar", "baz", "MyStruct", "MyEnum", "value", "data", "result",
        ];
        let segments: Vec<ast::ExprPathSegment> = (0..seg_count)
            .map(|_| {
                let name = segment_names[self.gen_index(segment_names.len())].to_string();
                ast::ExprPathSegment {
                    span: SrcSpan::default(),
                    name,
                    type_arguments: None,
                }
            })
            .collect();
        ast::ExprPath {
            span: SrcSpan::default(),
            segments,
            resolved_path: None,
        }
    }
}

// ─────────────────────────────────────────────────────────────────
// Type compatibility & inspection helpers
// ─────────────────────────────────────────────────────────────────

/// Return the list of rvalue kinds that can produce a value of type `ty`.
/// When `leaf_only` is true, restricts to simple/non-recursive kinds.
fn compatible_kinds(ty: &ast::Type, leaf_only: bool) -> Vec<ast::RValueKind> {
    use ast::RValueKind::*;

    if leaf_only {
        return leaf_kinds(ty);
    }

    // Base kinds valid for nearly all types
    let mut kinds = vec![
        Parentheses,
        Path,
        Block,
        If,
        Match,
        FunctionCall,
        MethodCall,
        Cast,
        ForEach,
    ];

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
        ast::Type::TypePath(_) => {
            kinds.push(StructInit);
            kinds.push(FieldAccess);
        }
        ast::Type::FunctionType(_) => {
            kinds.push(Closure);
        }
        ast::Type::ReferenceType(_) | ast::Type::PointerType(_) => {
            kinds.push(UnaryExpr);
        }
        ast::Type::InferType(_) => {
            // Unknown type — allow all leaf-like expressions
            kinds.push(Boolean);
            kinds.push(Integer);
            kinds.push(Float);
            kinds.push(BString);
            kinds.push(TypeInfo);
        }
        _ => {
            // For any other type, default to broad leaf set
            kinds.push(Boolean);
            kinds.push(Integer);
            kinds.push(Float);
            kinds.push(BString);
            kinds.push(TypeInfo);
            kinds.push(UnaryExpr);
            kinds.push(BinExpr);
        }
    }

    // Always include some control-flow / statement kinds (result type irrelevant)
    kinds.push(Break);
    kinds.push(Continue);
    kinds.push(Return);
    kinds.push(While);
    kinds.push(Await);

    kinds
}

/// Leaf-only (non-recursive) kinds appropriate for type `ty`.
fn leaf_kinds(ty: &ast::Type) -> Vec<ast::RValueKind> {
    use ast::RValueKind::*;
    let mut kinds = vec![Path, Parentheses];

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
        }
    }
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
        ast::Type::ReferenceType(_) => &[Deref],
        ast::Type::PointerType(_) => &[Deref],
        _ => &[Add, Sub, Not, Deref, Borrow, Typeof],
    }
}

/// Determine the operand type required for a unary operator to produce `result_ty`.
fn unary_operand_type(op: ast::UnaryExprOp, result_ty: &ast::Type) -> ast::Type {
    match op {
        ast::UnaryExprOp::Not => bool_type(),
        ast::UnaryExprOp::Deref => {
            // Deref: operand is a reference/pointer to result_ty
            ast::Type::ReferenceType(Box::new(ast::ReferenceType {
                span: SrcSpan::default(),
                lifetime: None,
                exclusivity: None,
                mutability: None,
                to: result_ty.clone(),
            }))
        }
        ast::UnaryExprOp::Borrow => {
            // Borrow: operand is result_ty (reference to it)
            // Result is a reference, so operand is the pointee
            match result_ty {
                ast::Type::ReferenceType(r) => r.to.clone(),
                ast::Type::PointerType(p) => p.to.clone(),
                _ => int32_type(),
            }
        }
        ast::UnaryExprOp::Typeof => arbitrary_type(),
        _ => result_ty.clone(), // Add, Sub: operand type = result type
    }
}

/// Binary operators compatible with producing `ty` as a result.
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
            Add, Sub, Mul, Div, Mod, BitAnd, BitOr, BitXor, BitShl, BitShr, LogicAnd, LogicOr, LogicLt, LogicGt,
            LogicLe, LogicGe, LogicEq, LogicNe,
        ],
        ast::Type::Float32(_) | ast::Type::Float64(_) => {
            &[Add, Sub, Mul, Div, LogicLt, LogicGt, LogicLe, LogicGe, LogicEq, LogicNe]
        }
        _ => &[
            Add, Sub, Mul, Div, LogicAnd, LogicOr, LogicEq, LogicNe, LogicLt, LogicGt, LogicLe, LogicGe,
        ],
    }
}

/// Determine operand types for a binary operator given the result type.
fn binary_operand_types(op: ast::BinExprOp, result_ty: &ast::Type) -> (ast::Type, ast::Type) {
    use ast::BinExprOp::*;
    match op {
        // Comparison/logical ops produce bool from non-bool operands
        LogicEq | LogicNe | LogicLt | LogicGt | LogicLe | LogicGe if !is_bool_type(result_ty) => {
            (result_ty.clone(), result_ty.clone())
        }
        // Logical ops on bools
        LogicAnd | LogicOr if is_bool_type(result_ty) => (bool_type(), bool_type()),
        // Arithmetic/bitwise: operands match result type
        _ => (result_ty.clone(), result_ty.clone()),
    }
}

/// Convert to integer kind based on type width.
fn infer_integer_kind(ty: &ast::Type) -> IntegerKind {
    match ty {
        ast::Type::Bool(_) | ast::Type::Int8(_) | ast::Type::UInt8(_) | ast::Type::Int16(_) | ast::Type::UInt16(_) => {
            IntegerKind::Hex
        }
        _ => IntegerKind::Dec,
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

/// Extract the element type from collection types, defaulting to Int32.
fn element_type_of(ty: &ast::Type) -> ast::Type {
    match ty {
        ast::Type::ArrayType(arr) => arr.element_type.clone(),
        ast::Type::SliceType(slice) => slice.element_type.clone(),
        _ => int32_type(),
    }
}

/// Extract field types from a tuple type; falls back to 1-3 Int32 fields.
fn tuple_field_types(ty: &ast::Type) -> Vec<ast::Type> {
    match ty {
        ast::Type::TupleType(tup) => tup.element_types.clone(),
        _ => {
            let count = 1 + 3; // arbitrary fallback: 2-4 fields
            (0..count).map(|_| int32_type()).collect()
        }
    }
}

/// Extract param and return types from a function type.
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
        _ => {
            // Fallback: 1 int param, returns int32
            (vec![int32_type()], int32_type())
        }
    }
}

/// Range element type: if ty is a range type, return the bound type, else Int32.
fn range_element_type(ty: &ast::Type) -> ast::Type {
    // Ranges are typically used with integer types; for now return the
    // underlying type if it's integral, otherwise Int32.
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

/// An arbitrary non-void type for sub-expressions (used when type doesn't matter).
fn arbitrary_type() -> ast::Type {
    int32_type()
}

/// An arbitrary non-void type distinct from potentially the target (for casts).
fn arbitrary_non_void_type() -> ast::Type {
    ast::Type::Float64(ast::Float64 {
        span: SrcSpan::default(),
    })
}
