use crate::Gen;
use crate::ty::{bool_type, is_bool_type, is_float_type, is_integral_type, is_signed_type, is_unit_type};
use crate::ty::{int32_type, tuple_field_types, unsigned_magnitude_max};
use nitrate_translation::parsetree::ast::{self, *};
use nitrate_translation::token::IntegerKind;
use ordered_float::NotNan;

// Value kind codes used for weighted selection.
const K_UNIT: u8 = 0;
const K_BOOL: u8 = 1;
const K_INT: u8 = 2;
const K_FLOAT: u8 = 3;
const K_TUPLE: u8 = 4;
const K_UNARY: u8 = 5;
const K_ARITH: u8 = 6;
const K_CMP: u8 = 7;
const K_SHIFT: u8 = 8;

/// Budget cost (weight) of generating a node of a given kind.
/// Heavier constructs cost more, which naturally limits their frequency.
fn value_kind_cost(kind: u8) -> u32 {
    match kind {
        K_UNIT | K_BOOL | K_INT | K_FLOAT | K_TUPLE => 1,
        K_UNARY => 2,
        K_ARITH | K_CMP | K_SHIFT => 3,
        _ => 1,
    }
}

impl Gen {
    /// Generate an expression whose type must match `ty`.  Only produces
    /// *value* forms that are safe for every position (initializers, return
    /// values, operands, tuple elements).  Control-flow (`if`/`while`) and
    /// block expressions are deliberately excluded because the solver cannot
    /// yet handle them in nested value positions, so those are only emitted as
    /// *statements* via `emit_safe_statement`.
    pub(crate) fn gen_value(&mut self, ty: &ast::Type) -> ast::Expr {
        if self.budget_left() == 0 {
            return self.gen_leaf_value(ty);
        }
        self.inc_rvalue_depth();
        let result = self.gen_value_inner(ty);
        self.dec_rvalue_depth();
        result
    }

    fn gen_value_inner(&mut self, ty: &ast::Type) -> ast::Expr {
        let force = self.force_leaf();
        let kind = self.select_value_kind(ty, force);
        let cost = value_kind_cost(kind);
        if !self.spend_budget(cost) {
            return self.gen_leaf_value(ty);
        }
        match kind {
            K_UNIT => self.gen_unit_leaf(),
            K_BOOL => self.gen_bool_leaf(),
            K_INT => self.gen_int_leaf(ty),
            K_FLOAT => self.gen_float_leaf(ty),
            K_TUPLE => self.gen_tuple_value(ty, force),
            K_UNARY => self.gen_unary_value(ty),
            K_ARITH => self.gen_arith_value(ty),
            K_CMP => self.gen_cmp_value(),
            K_SHIFT => self.gen_shift_value(ty),
            _ => self.gen_unit_leaf(),
        }
    }

    fn select_value_kind(&mut self, ty: &ast::Type, force: bool) -> u8 {
        let mut opts: Vec<(u32, u8)> = Vec::new();
        if is_unit_type(ty) {
            opts.push((10, K_UNIT));
        } else if is_bool_type(ty) {
            opts.push((9, K_BOOL));
            if !force {
                opts.push((3, K_UNARY));
                opts.push((3, K_CMP));
                opts.push((2, K_ARITH));
            }
        } else if is_integral_type(ty) {
            opts.push((9, K_INT));
            if !force {
                opts.push((3, K_UNARY));
                opts.push((4, K_ARITH));
                opts.push((1, K_SHIFT));
            }
        } else if is_float_type(ty) {
            opts.push((9, K_FLOAT));
            if !force {
                opts.push((3, K_UNARY));
                opts.push((4, K_ARITH));
            }
        } else {
            // Tuple.
            opts.push((10, K_TUPLE));
        }
        self.pick_weighted(&opts)
    }

    // ── Leaf-value generators ──

    /// Generate a value for `ty` that never recurses into composite operators.
    pub(crate) fn gen_leaf_value(&mut self, ty: &ast::Type) -> ast::Expr {
        if is_unit_type(ty) {
            self.gen_unit_leaf()
        } else if is_bool_type(ty) {
            self.gen_bool_leaf()
        } else if is_integral_type(ty) {
            self.gen_int_leaf(ty)
        } else if is_float_type(ty) {
            self.gen_float_leaf(ty)
        } else {
            // Tuple: construct its fields as leaves to stay shallow.
            let fields = tuple_field_types(ty);
            let elements = fields.iter().map(|f| self.gen_leaf_value(f)).collect();
            ast::Expr::Tuple(Box::new(ast::Tuple {
                span: SrcSpan::default(),
                elements,
            }))
        }
    }

    fn gen_unit_leaf(&mut self) -> ast::Expr {
        ast::Expr::Tuple(Box::new(ast::Tuple {
            span: SrcSpan::default(),
            elements: vec![],
        }))
    }

    fn gen_bool_leaf(&mut self) -> ast::Expr {
        ast::Expr::Boolean(ast::BooleanLit {
            span: SrcSpan::default(),
            value: self.next_bool(),
        })
    }

    fn gen_int_leaf(&mut self, ty: &ast::Type) -> ast::Expr {
        let magnitude = unsigned_magnitude_max(ty).unwrap_or(1_000_000);
        let cap = magnitude.min(1_000_000);
        let value = self.next_u64() as u128 % (cap + 1);
        let literal = ast::Expr::Integer(Box::new(ast::IntegerLit {
            span: SrcSpan::default(),
            value,
            kind: IntegerKind::Dec,
        }));
        ast::Expr::Cast(Box::new(ast::Cast {
            span: SrcSpan::default(),
            value: literal,
            to: ty.clone(),
        }))
    }

    fn gen_float_leaf(&mut self, ty: &ast::Type) -> ast::Expr {
        let bits = self.next_u64();
        let mantissa = bits & 0xFFFFF;
        let raw = f64::from_bits(0x3FF0000000000000u64 | (mantissa << 32));
        let value = NotNan::new(raw).unwrap_or_else(|_| NotNan::new(1.0).unwrap());
        let literal = ast::Expr::Float(ast::FloatLit {
            span: SrcSpan::default(),
            value,
        });
        ast::Expr::Cast(Box::new(ast::Cast {
            span: SrcSpan::default(),
            value: literal,
            to: ty.clone(),
        }))
    }

    fn gen_tuple_value(&mut self, ty: &ast::Type, force: bool) -> ast::Expr {
        let fields = tuple_field_types(ty);
        let elements = fields
            .iter()
            .map(|f| {
                if force {
                    self.gen_leaf_value(f)
                } else {
                    self.gen_value(f)
                }
            })
            .collect();
        ast::Expr::Tuple(Box::new(ast::Tuple {
            span: SrcSpan::default(),
            elements,
        }))
    }

    // ── Composite value generators (safe: no control-flow nesting) ──

    fn gen_unary_value(&mut self, ty: &ast::Type) -> ast::Expr {
        let operator;
        let operand;
        if is_bool_type(ty) {
            operator = UnaryExprOp::Not;
            operand = self.gen_bool_leaf();
        } else if is_signed_type(ty) || is_float_type(ty) {
            operator = if self.next_bool() {
                UnaryExprOp::Sub
            } else {
                UnaryExprOp::Add
            };
            operand = self.gen_leaf_value(ty);
        } else {
            // Unsigned integral: unary minus is invalid.
            operator = UnaryExprOp::Add;
            operand = self.gen_leaf_value(ty);
        }
        ast::Expr::UnaryExpr(Box::new(ast::UnaryExpr {
            span: SrcSpan::default(),
            operator,
            operand,
        }))
    }

    fn gen_arith_value(&mut self, ty: &ast::Type) -> ast::Expr {
        let (operator, left_ty, right_ty) = if is_bool_type(ty) {
            let op = if self.next_bool() {
                BinExprOp::LogicAnd
            } else {
                BinExprOp::LogicOr
            };
            (op, bool_type(), bool_type())
        } else if is_integral_type(ty) {
            let ops = [
                BinExprOp::Add,
                BinExprOp::Sub,
                BinExprOp::Mul,
                BinExprOp::Div,
                BinExprOp::Mod,
            ];
            (ops[self.gen_index(ops.len())], ty.clone(), ty.clone())
        } else {
            let ops = [BinExprOp::Add, BinExprOp::Sub, BinExprOp::Mul, BinExprOp::Div];
            (ops[self.gen_index(ops.len())], ty.clone(), ty.clone())
        };

        let left = self.gen_operand(&left_ty);
        let right = self.gen_operand(&right_ty);

        ast::Expr::BinExpr(Box::new(ast::BinExpr {
            span: SrcSpan::default(),
            operator,
            left,
            right,
        }))
    }

    fn gen_cmp_value(&mut self) -> ast::Expr {
        // NOTE: `!=` is intentionally excluded. The parser maps the token
        // sequence `!` `=` to `SetLogicAnd` (a compound assignment) instead of
        // `LogicNe`, which makes `a != b` lower to an assignment on a
        // non-mutable place → E4003. Use the other relational operators only.
        let ops = [
            BinExprOp::LogicLt,
            BinExprOp::LogicGt,
            BinExprOp::LogicLe,
            BinExprOp::LogicGe,
            BinExprOp::LogicEq,
        ];
        let operator = ops[self.gen_index(ops.len())];
        let left = self.gen_int_leaf(&int32_type());
        let right = self.gen_int_leaf(&int32_type());
        ast::Expr::BinExpr(Box::new(ast::BinExpr {
            span: SrcSpan::default(),
            operator,
            left,
            right,
        }))
    }

    fn gen_shift_value(&mut self, ty: &ast::Type) -> ast::Expr {
        let operator = if self.next_bool() {
            BinExprOp::BitShl
        } else {
            BinExprOp::BitShr
        };
        let left = self.gen_operand(ty);
        let right = self.gen_operand(ty);
        ast::Expr::BinExpr(Box::new(ast::BinExpr {
            span: SrcSpan::default(),
            operator,
            left,
            right,
        }))
    }

    /// Generate an arithmetic operand: a leaf, a unary expr, or a parenthesized
    /// nested binary expr — but never control flow.
    fn gen_operand(&mut self, ty: &ast::Type) -> ast::Expr {
        match self.next_u64() % 3 {
            0 => self.gen_leaf_value(ty),
            1 => self.gen_unary_value(ty),
            _ => parenthesize_binary(self.gen_value(ty)),
        }
    }

    /// A boolean expression safe for use as an `if`/`while` condition.
    pub(crate) fn gen_bool_condition(&mut self) -> ast::Expr {
        match self.next_u64() % 4 {
            0 => ast::Expr::Boolean(ast::BooleanLit {
                span: SrcSpan::default(),
                value: true,
            }),
            1 => ast::Expr::Boolean(ast::BooleanLit {
                span: SrcSpan::default(),
                value: false,
            }),
            2 => ast::Expr::UnaryExpr(Box::new(ast::UnaryExpr {
                span: SrcSpan::default(),
                operator: UnaryExprOp::Not,
                operand: ast::Expr::Boolean(ast::BooleanLit {
                    span: SrcSpan::default(),
                    value: self.next_bool(),
                }),
            })),
            _ => self.gen_cmp_value(),
        }
    }

    /// Emit one non-divergent statement into `elements`.
    ///
    /// When `allow_control_flow` is true, `if`/`while` statements may be
    /// emitted (with shallow, loop/if-free bodies).  When false, only variable
    /// declarations and discarded value expressions are produced, ensuring
    /// control flow is never nested (which would trip a solver RefCell bug).
    pub(crate) fn emit_safe_statement(&mut self, elements: &mut Vec<BlockItem>, allow_control_flow: bool) {
        let roll = self.next_u64() % 100;
        if allow_control_flow && roll < 28 && self.budget_left() >= 6 {
            elements.push(BlockItem::Stmt(self.gen_control_stmt()));
        } else if roll < 70 {
            let ty = self.gen_concrete_type();
            let init = self.gen_value(&ty);
            let name = self.gen_unique_name("v");
            elements.push(BlockItem::Variable(LocalVariable {
                span: SrcSpan::default(),
                kind: LocalVariableKind::Var,
                attributes: None,
                mutability: None,
                name: name.into(),
                ty: Some(ty),
                initializer: Some(init),
            }));
        } else {
            let ty = self.gen_concrete_type();
            let expr = self.gen_value(&ty);
            elements.push(BlockItem::Stmt(expr));
        }
    }

    /// Emit an `if` or `while` statement with a shallow body.
    fn gen_control_stmt(&mut self) -> ast::Expr {
        if self.next_bool() {
            let condition = self.gen_bool_condition();
            let body = self.gen_shallow_block();
            ast::Expr::While(Box::new(ast::WhileLoop {
                span: SrcSpan::default(),
                condition: Some(condition),
                body,
            }))
        } else {
            let condition = self.gen_bool_condition();
            let true_branch = self.gen_shallow_block();
            let false_branch = self.gen_shallow_block();
            ast::Expr::If(Box::new(ast::If {
                span: SrcSpan::default(),
                condition,
                true_branch,
                false_branch: Some(ElseIf::Block(false_branch)),
            }))
        }
    }

    /// A block whose statements never contain further `if`/`while` control flow.
    fn gen_shallow_block(&mut self) -> ast::Block {
        let mut elements = Vec::new();
        let stmt_count = self.gen_index(2);
        for _ in 0..stmt_count {
            if self.budget_left() < 2 {
                break;
            }
            self.emit_safe_statement(&mut elements, false);
        }
        ast::Block {
            span: SrcSpan::default(),
            safety: None,
            elements,
        }
    }
}

/// Wrap a binary expression in parentheses so it parses correctly as an operand.
fn parenthesize_binary(expr: ast::Expr) -> ast::Expr {
    if matches!(expr, ast::Expr::BinExpr(_)) {
        ast::Expr::Parentheses(Box::new(ast::ExprParentheses {
            span: SrcSpan::default(),
            inner: expr,
        }))
    } else {
        expr
    }
}
