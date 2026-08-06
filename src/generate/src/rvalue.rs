use crate::ty::{
    bool_type, element_type_of, fallback_expr, function_type_parts, int32_type, is_bool_type, is_castable_type,
    is_integral_type, is_numeric_type, is_type_path, is_unit_type, max_integer_value, random_cast_source_type,
    range_element_type, tuple_field_types, types_compatible, unit_type, usize_type,
};
use crate::{Gen, StructInfo, Symbol, SymbolKind};
use nitrate_translation::{
    nstring::NString,
    parsetree::ast::{self, *},
    token::IntegerKind,
};

impl Gen {
    /// Select a random rvalue kind that is type-compatible AND scope-compatible.
    fn select_rvalue_kind(&mut self, ty: &ast::Type) -> ast::RValueKind {
        use ast::RValueKind::*;

        let is_unit = is_unit_type(ty);

        let mut kinds: Vec<ast::RValueKind> = Vec::new();

        // Parentheses are always valid
        kinds.push(Parentheses);

        // Path references — only if matching locals or structs exist
        if self.has_any_local() {
            kinds.push(Path);
        }
        // If no locals but functions exist and we need a function-typed path
        if self.has_any_function() && is_type_path(ty) {
            kinds.push(Path);
        }

        // Cast — valid between castable types
        if is_castable_type(ty) {
            kinds.push(Cast);
        }
        // Also allow casting to reference/pointer/compound types from integers
        if !is_castable_type(ty) && !is_unit {
            kinds.push(Cast);
        }

        // If expression — valid for any type (produces a value)
        if !is_unit {
            kinds.push(If);
        }

        // Match expression — only for integral types, produces a value
        if is_integral_type(ty) {
            kinds.push(Match);
        }

        // While loop — only for unit type
        if is_unit {
            kinds.push(While);
        }

        // ForEach loop — only for unit type
        if is_unit {
            kinds.push(ForEach);
        }

        // Function call — if functions exist (caller must check return compatibility)
        if self.has_any_function() {
            kinds.push(FunctionCall);
        }

        // Block — valid for any type
        if !self.force_leaf() {
            kinds.push(Block);
        }

        // Type-specific literals
        if is_bool_type(ty) {
            kinds.push(Boolean);
            if !self.force_leaf() {
                kinds.push(UnaryExpr);
            }
        } else if is_integral_type(ty) {
            kinds.push(Integer);
            if !self.force_leaf() {
                kinds.push(UnaryExpr);
                kinds.push(BinExpr);
                kinds.push(Range);
            }
        } else if matches!(ty, ast::Type::Float32(_) | ast::Type::Float64(_)) {
            kinds.push(Float);
            if !self.force_leaf() {
                kinds.push(UnaryExpr);
                kinds.push(BinExpr);
            }
        } else {
            // Compound types
            match ty {
                ast::Type::ArrayType(_) | ast::Type::SliceType(_) => {
                    kinds.push(List);
                    if !self.force_leaf() {
                        kinds.push(IndexAccess);
                    }
                }
                ast::Type::TupleType(_) => {
                    kinds.push(Tuple);
                }
                ast::Type::FunctionType(_) => {
                    if !self.force_leaf() {
                        kinds.push(Closure);
                    }
                }
                ast::Type::TypePath(_) => {
                    if self.has_any_struct() && !self.force_leaf() {
                        kinds.push(StructInit);
                        kinds.push(FieldAccess);
                    }
                }
                ast::Type::ReferenceType(_) | ast::Type::PointerType(_) => {
                    // References/pointers can be obtained via casts
                    if !self.force_leaf() {
                        kinds.push(Cast);
                    }
                }
                _ => {
                    // fallback: allow integer (will be cast) as last resort for unknown types
                    kinds.push(Integer);
                }
            }
        }

        // If we're at leaf depth, filter to only leaf-appropriate kinds
        if self.force_leaf() {
            kinds.retain(|k| {
                matches!(
                    k,
                    Boolean | Integer | Float | String | BString | Path | Cast | Parentheses
                )
            });
        }

        if kinds.is_empty() {
            return Integer;
        }

        let idx = self.gen_index(kinds.len());
        kinds.swap_remove(idx)
    }

    /// Generate an expression whose type **must** match the given `ty`.
    pub(crate) fn gen_rvalue(&mut self, ty: &ast::Type) -> ast::Expr {
        if self.budget_left() == 0 {
            return fallback_expr(ty);
        }

        self.inc_rvalue_depth();

        let kind = self.select_rvalue_kind(ty);
        // Spend budget based on the kind's weight
        self.spend_budget_for_kind(&kind);

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
            ast::RValueKind::ForEach => self.gen_rvalue_foreach(ty),
            ast::RValueKind::FunctionCall => self.gen_rvalue_function_call(ty),
            // Control-flow expressions as values — only valid inside loops/return position
            ast::RValueKind::Break => self.gen_rvalue_break(ty),
            ast::RValueKind::Continue => self.gen_rvalue_continue(ty),
            ast::RValueKind::Return => self.gen_rvalue_return(ty),
            ast::RValueKind::Await => fallback_expr(ty),
            ast::RValueKind::MethodCall => fallback_expr(ty),
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

    fn gen_rvalue_integer(&mut self, ty: &ast::Type) -> ast::Expr {
        // Clamp integer values to be representable in the target type
        let max_val = max_integer_value(ty);
        let value = if max_val <= 1 {
            0u128
        } else if max_val == u128::MAX {
            self.next_u64() as u128
        } else {
            (self.next_u64() as u128) % (max_val + 1)
        };
        ast::Expr::Integer(Box::new(ast::IntegerLit {
            span: SrcSpan::default(),
            value,
            kind: IntegerKind::Dec,
        }))
    }

    fn gen_rvalue_float(&mut self, _ty: &ast::Type) -> ast::Expr {
        let bits = self.next_u64();
        let mantissa = (bits & 0xFFFFF) as u64;
        let raw = f64::from_bits(0x3FF0000000000000u64 | (mantissa << 32));
        let value = ordered_float::NotNan::new(raw).unwrap_or(ordered_float::NotNan::new(1.0).unwrap());
        ast::Expr::Float(ast::FloatLit {
            span: SrcSpan::default(),
            value,
        })
    }

    fn gen_rvalue_string(&mut self, _ty: &ast::Type) -> ast::Expr {
        let len = 1 + (self.next_u64() as usize % 8);
        let safe_chars: &[u8] =
            b"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 !#$%&'()*+,-./:;<=>?@[]^_`{|}~";
        let mut s = String::with_capacity(len);
        for _ in 0..len {
            let idx = (self.next_u64() as usize) % safe_chars.len();
            s.push(safe_chars[idx] as char);
        }
        ast::Expr::String(ast::StringLit {
            span: SrcSpan::default(),
            value: s,
        })
    }

    fn gen_rvalue_bstring(&mut self, _ty: &ast::Type) -> ast::Expr {
        let len = 1 + (self.next_u64() as usize % 8);
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
        // typeof on a type, e.g. typeof(i32)
        ast::Expr::TypeInfo(Box::new(ast::TypeInfo {
            span: SrcSpan::default(),
            the: ty.clone(),
        }))
    }

    fn gen_rvalue_list(&mut self, ty: &ast::Type) -> ast::Expr {
        let elem_ty = element_type_of(ty);
        let len = 1 + self.gen_index(4);
        let elements: Vec<ast::Expr> = (0..len).map(|_| self.gen_rvalue(&elem_ty)).collect();
        ast::Expr::List(Box::new(ast::List {
            span: SrcSpan::default(),
            elements,
        }))
    }

    fn gen_rvalue_tuple(&mut self, ty: &ast::Type) -> ast::Expr {
        let field_types = tuple_field_types(ty);
        if field_types.is_empty() {
            return ast::Expr::Tuple(Box::new(ast::Tuple {
                span: SrcSpan::default(),
                elements: vec![],
            }));
        }
        let elements: Vec<ast::Expr> = field_types.iter().map(|ft| self.gen_rvalue(ft)).collect();
        ast::Expr::Tuple(Box::new(ast::Tuple {
            span: SrcSpan::default(),
            elements,
        }))
    }

    fn gen_rvalue_struct_init(&mut self, ty: &ast::Type) -> ast::Expr {
        let struct_info = self.pick_struct_init_target(ty);
        if struct_info.fields.is_empty() {
            return fallback_expr(ty);
        }

        let path = self.make_single_segment_path(struct_info.name.clone());

        // Clone field info first to avoid borrow conflicts with self
        let field_data: Vec<(String, ast::Type)> = struct_info.fields.clone();

        let fields: Vec<(NString, ast::Expr)> = field_data
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

    fn pick_struct_init_target(&mut self, ty: &ast::Type) -> StructInfo {
        // Try type-directed struct lookup first
        if let ast::Type::TypePath(tp) = ty {
            if let Some(seg) = tp.segments.first() {
                if let Some(info) = self.find_struct_by_name(&seg.name) {
                    if !info.fields.is_empty() {
                        return info.clone();
                    }
                }
            }
        }
        // Try to find any struct with fields
        let indices_with_fields: Vec<usize> = self
            .known_structs
            .iter()
            .enumerate()
            .filter(|(_, s)| !s.fields.is_empty())
            .map(|(i, _)| i)
            .collect();
        if !indices_with_fields.is_empty() {
            let idx = self.gen_index(indices_with_fields.len());
            return self.known_structs[indices_with_fields[idx]].clone();
        }
        StructInfo {
            name: String::new(),
            fields: vec![],
        }
    }

    fn gen_rvalue_unary_expr(&mut self, ty: &ast::Type) -> ast::Expr {
        let ops = compatible_unary_ops(ty);
        if ops.is_empty() {
            return fallback_expr(ty);
        }
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
        if ops.is_empty() {
            return fallback_expr(ty);
        }
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
        // Generate a cast expression: source_value as target_type
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

        let stmt_count = self.gen_index(3);
        let mut elements = Vec::with_capacity(stmt_count + 1);
        for _ in 0..stmt_count {
            if self.budget_left() > 0 {
                if self.next_bool() {
                    // Variable declaration
                    let init_ty = if self.next_bool() {
                        ty.clone()
                    } else {
                        crate::ty::arbitrary_type(self)
                    };
                    let init = self.gen_rvalue(&init_ty);
                    let var_name_prefix = self.gen_unique_name("v");
                    let actual_name = self.add_local(var_name_prefix, init_ty.clone());
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
                    // Statement expression (discard value)
                    let stmt_ty = crate::ty::arbitrary_type(self);
                    let expr = self.gen_rvalue(&stmt_ty);
                    elements.push(ast::BlockItem::Stmt(expr));
                }
            }
        }
        // Final expression produces the block's value
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
                        name: format!("a_{}", i).into(),
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
        if path.segments.first().map_or(true, |s| s.name.is_empty()) {
            return fallback_expr(ty);
        }
        ast::Expr::Path(Box::new(path))
    }

    fn pick_compatible_path(&mut self, ty: &ast::Type) -> ast::ExprPath {
        // Priority 1: Local variables of matching type
        let compatible_names: Vec<String> = self.find_locals_by_type(ty).iter().map(|s| s.name.clone()).collect();
        if !compatible_names.is_empty() {
            let idx = self.gen_index(compatible_names.len());
            let name = compatible_names[idx].clone();
            return self.make_single_segment_path(name);
        }

        // Priority 2: Function names (for function types)
        if matches!(ty, ast::Type::FunctionType(_)) {
            if self.has_any_function() {
                let info = self.pick_function();
                return self.make_single_segment_path(info.name);
            }
        }

        // Priority 3: Struct/Enum names for TypePath targets
        if is_type_path(ty) {
            // Extract the name from the TypePath and return it as a path
            if let ast::Type::TypePath(tp) = ty {
                if let Some(seg) = tp.segments.first() {
                    return self.make_single_segment_path(seg.name.clone());
                }
            }
        }

        // Priority 4: Any local variable as last resort (type may not match exactly,
        // but it's better than an empty path)
        if self.has_any_local() {
            // find any local
            for frame in self.frames.iter().rev() {
                if let Some(sym) = frame.locals.first() {
                    return self.make_single_segment_path(sym.name.clone());
                }
            }
        }

        // Priority 5: Any function name as last resort
        if self.has_any_function() && is_unit_type(ty) {
            let info = self.pick_function();
            return self.make_single_segment_path(info.name);
        }

        // No compatible path found - return empty path (caller will use fallback)
        self.make_single_segment_path(String::new())
    }

    fn gen_rvalue_index_access(&mut self, ty: &ast::Type) -> ast::Expr {
        // Generate an array/slice value and index into it
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
        // Find a struct with at least one field of matching type
        let candidates: Vec<(usize, usize)> = self
            .known_structs
            .iter()
            .enumerate()
            .flat_map(|(si, s)| {
                s.fields
                    .iter()
                    .enumerate()
                    .filter(|(_, (_, ft))| types_compatible(ft, ty))
                    .map(move |(fi, _)| (si, fi))
            })
            .collect();

        if let Some(&(struct_idx, field_idx)) = candidates.first() {
            // Clone to avoid borrow conflicts with self
            let info = self.known_structs[struct_idx].clone();
            let field_name = info.fields[field_idx].0.clone();

            // Generate struct init as the object
            let path = self.make_single_segment_path(info.name.clone());

            // Pre-generate field values outside the closure that borrows self
            let field_data: Vec<(String, ast::Type)> = info.fields.clone();
            let field_exprs: Vec<(NString, ast::Expr)> = field_data
                .iter()
                .map(|(n, ft)| {
                    let fname: NString = n.clone().into();
                    let val = self.gen_rvalue(ft);
                    (fname, val)
                })
                .collect();

            let object = ast::Expr::StructInit(Box::new(ast::StructInit {
                span: SrcSpan::default(),
                path,
                fields: field_exprs,
            }));
            return ast::Expr::FieldAccess(Box::new(ast::FieldAccess {
                span: SrcSpan::default(),
                object,
                field: field_name,
            }));
        }

        fallback_expr(ty)
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

        // While loops always need a condition
        let condition = Some(self.gen_rvalue(&bool_type()));
        let body = self.gen_block_with_type(ty);

        self.set_in_loop(prev_in_loop);
        ast::Expr::While(Box::new(ast::WhileLoop {
            span: SrcSpan::default(),
            condition,
            body,
        }))
    }

    fn gen_rvalue_match(&mut self, ty: &ast::Type) -> ast::Expr {
        let condition = self.gen_rvalue(&int32_type());
        let case_count = 1 + self.gen_index(3);
        let mut cases = Vec::with_capacity(case_count);
        // Use pre-generated unique values to avoid duplicate arms
        let mut used_values: Vec<u128> = Vec::with_capacity(case_count);
        for _ in 0..case_count {
            // Generate a small value that hasn't been used yet
            let mut case_val: u128;
            let mut tries = 0;
            loop {
                case_val = self.next_u64() as u128 % 1000;
                if !used_values.contains(&case_val) || tries > 10 {
                    break;
                }
                tries += 1;
            }
            used_values.push(case_val);
            let case_expr = ast::Expr::Integer(Box::new(ast::IntegerLit {
                span: SrcSpan::default(),
                value: case_val,
                kind: IntegerKind::Dec,
            }));
            let body = self.gen_block_with_type(ty);
            cases.push(ast::MatchCase {
                span: SrcSpan::default(),
                condition: case_expr,
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
        // Only valid inside a loop
        if !self.in_loop() {
            return fallback_expr(&unit_type());
        }
        ast::Expr::Break(Box::new(ast::Break {
            span: SrcSpan::default(),
            label: None,
        }))
    }

    fn gen_rvalue_continue(&mut self, _ty: &ast::Type) -> ast::Expr {
        // Only valid inside a loop
        if !self.in_loop() {
            return fallback_expr(&unit_type());
        }
        ast::Expr::Continue(Box::new(ast::Continue {
            span: SrcSpan::default(),
            label: None,
        }))
    }

    fn gen_rvalue_return(&mut self, ty: &ast::Type) -> ast::Expr {
        if is_unit_type(ty) && self.next_bool() {
            return ast::Expr::Return(Box::new(ast::Return {
                span: SrcSpan::default(),
                value: None,
            }));
        }
        let value = self.gen_rvalue(ty);
        ast::Expr::Return(Box::new(ast::Return {
            span: SrcSpan::default(),
            value: Some(value),
        }))
    }

    fn gen_rvalue_foreach(&mut self, ty: &ast::Type) -> ast::Expr {
        // Generate an iterable type: slice of int32
        let elem_ty = int32_type();
        let binding_name: NString = format!("it_{}", self.gen_unique_suffix()).into();
        let bindings: Vec<NString> = vec![binding_name.clone()];

        let iterable_ty = ast::Type::SliceType(Box::new(ast::SliceType {
            span: SrcSpan::default(),
            element_type: elem_ty.clone(),
        }));
        let iterable = self.gen_rvalue(&iterable_ty);

        let prev_in_loop = self.in_loop();
        self.set_in_loop(true);

        self.push_frame();
        let bname: String = binding_name.to_string();
        self.add_local(bname, elem_ty);

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

    fn gen_rvalue_function_call(&mut self, ty: &ast::Type) -> ast::Expr {
        // Pick a function whose return type is compatible with the expected type
        let func = if self.has_any_function() {
            self.pick_function_matching_return(ty)
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

    /// Generate a block with the given result type. Creates its own scope.
    pub(crate) fn gen_block_with_type(&mut self, ty: &ast::Type) -> ast::Block {
        let prev_in_loop = self.in_loop();
        self.push_frame();

        let stmt_count = self.gen_index(3);
        let mut elements = Vec::with_capacity(stmt_count + 1);
        for _ in 0..stmt_count {
            if self.budget_left() > 0 {
                if self.next_bool() {
                    // Variable declaration
                    let init_ty = crate::ty::arbitrary_type(self);
                    let init = self.gen_rvalue(&init_ty);
                    let var_name_prefix = self.gen_unique_name("v");
                    let actual_name = self.add_local(var_name_prefix, init_ty.clone());
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
                    // Statement expression
                    let t = crate::ty::arbitrary_type(self);
                    let expr = self.gen_rvalue(&t);
                    elements.push(ast::BlockItem::Stmt(expr));
                }
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

// ── Unary / Binary operator helpers ──

fn compatible_unary_ops(ty: &ast::Type) -> &'static [ast::UnaryExprOp] {
    use ast::UnaryExprOp::*;
    if is_bool_type(ty) {
        &[Not]
    } else if is_numeric_type(ty) {
        &[Add, Sub]
    } else if matches!(ty, ast::Type::ReferenceType(_) | ast::Type::PointerType(_)) {
        &[Deref]
    } else {
        &[]
    }
}

fn unary_operand_type(op: ast::UnaryExprOp, result_ty: &ast::Type) -> ast::Type {
    match op {
        ast::UnaryExprOp::Not => bool_type(),
        ast::UnaryExprOp::Add | ast::UnaryExprOp::Sub => result_ty.clone(),
        ast::UnaryExprOp::Deref => ast::Type::ReferenceType(Box::new(ast::ReferenceType {
            span: SrcSpan::default(),
            lifetime: None,
            exclusivity: None,
            mutability: None,
            to: result_ty.clone(),
        })),
        _ => result_ty.clone(),
    }
}

/// Return the binary operators whose result type is compatible with `ty`.
/// Comparison operators (==, !=, <, >, <=, >=) return bool regardless of operand type,
/// so they are only offered when `ty` is bool. Arithmetic operators return the same type.
fn compatible_binary_ops(ty: &ast::Type) -> &'static [ast::BinExprOp] {
    use ast::BinExprOp::*;
    if is_bool_type(ty) {
        // When expecting bool: logic ops on bool + comparisons on numeric types
        &[LogicAnd, LogicOr, LogicEq, LogicNe, LogicLt, LogicGt, LogicLe, LogicGe]
    } else if is_integral_type(ty) {
        &[Add, Sub, Mul, Div, Mod, BitAnd, BitOr, BitXor, BitShl, BitShr]
    } else if matches!(ty, ast::Type::Float32(_) | ast::Type::Float64(_)) {
        &[Add, Sub, Mul, Div]
    } else {
        &[]
    }
}

/// Return the operand types needed for a binary op given the expected result type.
fn binary_operand_types(op: ast::BinExprOp, result_ty: &ast::Type) -> (ast::Type, ast::Type) {
    use ast::BinExprOp::*;
    match op {
        // Comparison/equality: operands are int or float, result is bool.
        // When result_ty is bool, we need integer/float operands.
        LogicEq | LogicNe | LogicLt | LogicGt | LogicLe | LogicGe => {
            // Pick a random numeric type for the operands
            if is_bool_type(result_ty) {
                (int32_type(), int32_type())
            } else {
                (result_ty.clone(), result_ty.clone())
            }
        }
        LogicAnd | LogicOr => (bool_type(), bool_type()),
        BitShl | BitShr => (result_ty.clone(), usize_type()),
        _ => (result_ty.clone(), result_ty.clone()),
    }
}
