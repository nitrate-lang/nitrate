use crate::ty::{fallback_expr, is_bool_type, is_float_type, is_integral_type, is_unit_type, unit_type};
use crate::{FuncInfo, Gen, MAX_ITEM_DEPTH, StructInfo};
use nitrate_translation::{
    nstring::NString,
    parsetree::ast::{self, *},
    token::IntegerKind,
};

impl Gen {
    fn select_item_kind(&mut self) -> ast::ItemKind {
        let choices: &[(ast::ItemKind, u32)] = &[
            (ast::ItemKind::Function, 45),
            (ast::ItemKind::Struct, 20),
            (ast::ItemKind::Enum, 8),
            (ast::ItemKind::Variable, 12),
            (ast::ItemKind::TypeAlias, 10),
            (ast::ItemKind::Module, 5),
        ];

        let total_weight: u32 = choices.iter().map(|(_, w)| w).sum();
        let mut roll = self.next_u64() as u32 % total_weight;
        for (kind, weight) in choices {
            if roll < *weight {
                return match kind {
                    ast::ItemKind::SyntaxError => ast::ItemKind::Function,
                    ast::ItemKind::Module if self.item_depth < MAX_ITEM_DEPTH => ast::ItemKind::Module,
                    ast::ItemKind::Function => ast::ItemKind::Function,
                    ast::ItemKind::Struct => ast::ItemKind::Struct,
                    ast::ItemKind::Enum => ast::ItemKind::Enum,
                    ast::ItemKind::TypeAlias => ast::ItemKind::TypeAlias,
                    ast::ItemKind::Variable => ast::ItemKind::Variable,
                    ast::ItemKind::Module | ast::ItemKind::Import | ast::ItemKind::Trait | ast::ItemKind::Impl => {
                        roll -= weight;
                        continue;
                    }
                };
            }
            roll -= weight;
        }
        ast::ItemKind::Function
    }

    pub(crate) fn gen_item(&mut self, argc: Option<u32>) -> ast::Item {
        if self.budget_left() < 2 {
            return self.gen_placeholder_function();
        }

        let kind = match argc {
            Some(_) => ast::ItemKind::Function,
            None => self.select_item_kind(),
        };

        match kind {
            ast::ItemKind::SyntaxError => unreachable!(),
            ast::ItemKind::Module => self.gen_item_module(),
            ast::ItemKind::Import => self.gen_placeholder_function(),
            ast::ItemKind::TypeAlias => self.gen_item_type_alias(),
            ast::ItemKind::Struct => self.gen_item_struct(),
            ast::ItemKind::Enum => self.gen_item_enum(),
            ast::ItemKind::Trait => self.gen_placeholder_function(),
            ast::ItemKind::Impl => self.gen_placeholder_function(),
            ast::ItemKind::Function => self.gen_item_function(argc),
            ast::ItemKind::Variable => self.gen_item_variable(),
        }
    }

    /// Generate a minimal placeholder function when budget is exhausted.
    pub(crate) fn gen_placeholder_function(&mut self) -> ast::Item {
        let name = self.gen_unique_name("empty");
        let func_info = FuncInfo {
            name: name.clone(),
            params: Vec::new(),
            return_type: None,
        };
        self.register_function(func_info);
        ast::Item::Function(ast::Function {
            span: SrcSpan::default(),
            visibility: None,
            attributes: None,
            name: name.into(),
            generics: None,
            parameters: ast::FuncParams {
                span: SrcSpan::default(),
                params: Vec::new(),
                variadic: false,
            },
            return_type: None,
            definition: Some(ast::Block {
                span: SrcSpan::default(),
                safety: None,
                elements: vec![],
            }),
            abi: None,
        })
    }

    pub(crate) fn gen_item_main(&mut self) -> ast::Item {
        self.has_main = true;
        self.push_frame();

        // main() returns either i32 or () (unit). These are the only valid
        // return types for an entry point in Nitrate.
        let return_type = if self.next_bool() {
            Some(crate::ty::int32_type())
        } else {
            None
        };

        let body_ty = return_type.clone().unwrap_or_else(unit_type);

        let definition = Some(self.gen_block_with_return(&body_ty));
        self.pop_frame();

        let func_info = FuncInfo {
            name: "main".to_string(),
            params: Vec::new(),
            return_type: return_type.clone(),
        };
        self.register_function(func_info);

        ast::Item::Function(ast::Function {
            span: SrcSpan::default(),
            visibility: None,
            attributes: None,
            name: "main".into(),
            generics: None,
            parameters: ast::FuncParams {
                span: SrcSpan::default(),
                params: Vec::new(),
                variadic: false,
            },
            return_type,
            definition,
            abi: None,
        })
    }

    fn gen_item_module(&mut self) -> ast::Item {
        // Spend budget *before* checking, based on weight of module item
        if !self.spend_budget_module() {
            return self.gen_placeholder_function();
        }
        if self.budget_left() < 3 {
            return self.gen_placeholder_function();
        }
        self.item_depth += 1;
        let item_count = 1 + self.gen_index(2);
        let mut items = Vec::with_capacity(item_count);
        for _ in 0..item_count {
            if self.budget_left() > 0 {
                items.push(self.gen_item(None));
            }
        }
        self.item_depth = self.item_depth.saturating_sub(1);

        ast::Item::Module(Box::new(ast::Module {
            span: SrcSpan::default(),
            visibility: None,
            attributes: None,
            name: self.gen_unique_name("module").into(),
            items,
        }))
    }

    fn gen_item_type_alias(&mut self) -> ast::Item {
        self.spend_budget_type_alias();
        let alias_type = Some(self.gen_type());
        ast::Item::TypeAlias(ast::TypeAlias {
            span: SrcSpan::default(),
            visibility: None,
            attributes: None,
            name: self.gen_unique_name("Type").into(),
            generics: None,
            alias_type,
        })
    }

    fn gen_item_struct(&mut self) -> ast::Item {
        if !self.spend_budget_struct() {
            return self.gen_placeholder_function();
        }
        let name = self.gen_unique_name("Struct");
        let field_count = 1 + self.gen_index(3);
        let mut fields: Vec<(String, ast::Type)> = Vec::with_capacity(field_count);
        let mut ast_fields = Vec::with_capacity(field_count);
        for i in 0..field_count {
            // Struct fields must use constructible types so struct inits can work
            let field_ty = self.gen_local_type();
            let field_name = format!("field_{}", i);
            fields.push((field_name.clone(), field_ty.clone()));
            ast_fields.push(ast::StructField {
                span: SrcSpan::default(),
                visibility: None,
                attributes: None,
                name: field_name.into(),
                ty: field_ty,
                default_value: None,
            });
        }
        self.register_struct(StructInfo {
            name: name.clone(),
            fields,
        });
        ast::Item::Struct(ast::Struct {
            span: SrcSpan::default(),
            visibility: None,
            attributes: None,
            name: name.into(),
            generics: None,
            fields: ast_fields,
        })
    }

    fn gen_item_enum(&mut self) -> ast::Item {
        if !self.spend_budget_enum() {
            return self.gen_placeholder_function();
        }
        let name = self.gen_unique_name("Enum");
        // Register the enum name so it can appear in TypePath generation
        self.register_enum(name.clone());
        let variant_count = 2 + self.gen_index(4);
        let mut variants = Vec::with_capacity(variant_count);
        for _ in 0..variant_count {
            let variant_name: NString = format!("Variant_{}", self.gen_unique_suffix()).into();
            // Enum variant payloads should be constructible types
            let payload_ty = if self.next_bool() {
                Some(self.gen_local_type())
            } else {
                None
            };
            variants.push(ast::EnumVariant {
                span: SrcSpan::default(),
                attributes: None,
                name: variant_name,
                ty: payload_ty,
                default_value: None,
            });
        }
        ast::Item::Enum(ast::Enum {
            span: SrcSpan::default(),
            visibility: None,
            attributes: None,
            name: name.into(),
            generics: None,
            variants,
        })
    }

    pub(crate) fn gen_item_function(&mut self, argc: Option<u32>) -> ast::Item {
        ast::Item::Function(self.gen_function(argc))
    }

    fn gen_item_variable(&mut self) -> ast::Item {
        if !self.spend_budget_variable() {
            return self.gen_placeholder_function();
        }
        let declared_ty = self.gen_const_initializable_type();
        let initializer = Some(self.gen_const_initializer(&declared_ty));
        ast::Item::Variable(ast::GlobalVariable {
            span: SrcSpan::default(),
            visibility: None,
            kind: ast::GlobalVariableKind::Const,
            attributes: None,
            mutability: None,
            name: self.gen_unique_name("CONST").into(),
            ty: Some(declared_ty),
            initializer,
        })
    }

    /// Generate a type that is safe for const initialization (numeric, bool, unit).
    fn gen_const_initializable_type(&mut self) -> ast::Type {
        match self.next_u64() % 5 {
            0 => ast::Type::Bool(ast::Bool {
                span: SrcSpan::default(),
            }),
            1 => ast::Type::Int32(ast::Int32 {
                span: SrcSpan::default(),
            }),
            2 => ast::Type::Float64(ast::Float64 {
                span: SrcSpan::default(),
            }),
            3 => ast::Type::USize(ast::USize {
                span: SrcSpan::default(),
            }),
            _ => ast::Type::Int64(ast::Int64 {
                span: SrcSpan::default(),
            }),
        }
    }

    fn gen_const_initializer(&mut self, ty: &ast::Type) -> ast::Expr {
        if is_bool_type(ty) {
            return ast::Expr::Boolean(ast::BooleanLit {
                span: SrcSpan::default(),
                value: self.next_bool(),
            });
        }
        if is_float_type(ty) {
            let bits = self.next_u64();
            let mantissa = (bits & 0xFFFFF) as u64;
            let raw = f64::from_bits(0x3FF0000000000000u64 | (mantissa << 32));
            let value = ordered_float::NotNan::new(raw).unwrap_or(ordered_float::NotNan::new(1.0).unwrap());
            return ast::Expr::Float(ast::FloatLit {
                span: SrcSpan::default(),
                value,
            });
        }
        if is_integral_type(ty) {
            let max_val = crate::ty::max_integer_value(ty);
            let value = if max_val <= 1 {
                0u128
            } else if max_val == u128::MAX {
                self.next_u64() as u128
            } else {
                (self.next_u64() as u128) % (max_val + 1)
            };
            return ast::Expr::Integer(Box::new(ast::IntegerLit {
                span: SrcSpan::default(),
                value,
                kind: IntegerKind::Dec,
            }));
        }
        if is_unit_type(ty) {
            return ast::Expr::Tuple(Box::new(ast::Tuple {
                span: SrcSpan::default(),
                elements: vec![],
            }));
        }
        // For any other type, return 0
        ast::Expr::Integer(Box::new(ast::IntegerLit {
            span: SrcSpan::default(),
            value: 0,
            kind: IntegerKind::Dec,
        }))
    }

    fn gen_block_with_return(&mut self, ret_ty: &ast::Type) -> ast::Block {
        if !self.spend_budget_block() {
            let ret_val = if is_unit_type(ret_ty) {
                None
            } else {
                Some(fallback_expr(ret_ty))
            };
            return ast::Block {
                span: SrcSpan::default(),
                safety: None,
                elements: vec![ast::BlockItem::Expr(ast::Expr::Return(Box::new(ast::Return {
                    span: SrcSpan::default(),
                    value: ret_val,
                })))],
            };
        }

        let stmt_count = self.gen_index(3);
        let mut elements = Vec::with_capacity(stmt_count + 1);
        for _ in 0..stmt_count {
            if self.budget_left() > 0 {
                if self.next_bool() {
                    let var_name_prefix = self.gen_unique_name("v");
                    // Use gen_local_type for block-local variable types
                    let init_ty = self.gen_local_type();
                    let init = self.gen_rvalue(&init_ty);
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
                    // Statement expression (discard value) — use constructible type
                    let t = self.gen_local_type();
                    let expr = self.gen_rvalue(&t);
                    elements.push(ast::BlockItem::Stmt(expr));
                }
            }
        }
        // Always end with a return
        let ret_val = if is_unit_type(ret_ty) {
            None
        } else {
            Some(self.gen_rvalue(ret_ty))
        };
        elements.push(ast::BlockItem::Expr(ast::Expr::Return(Box::new(ast::Return {
            span: SrcSpan::default(),
            value: ret_val,
        }))));
        ast::Block {
            span: SrcSpan::default(),
            safety: None,
            elements,
        }
    }

    fn gen_function(&mut self, argc: Option<u32>) -> ast::Function {
        let name = self.gen_unique_name("fn");

        let param_count = match argc {
            Some(n) => n as usize,
            None => self.gen_index(4),
        };
        let mut params = Vec::with_capacity(param_count);
        let mut param_types = Vec::with_capacity(param_count);
        for i in 0..param_count {
            // Function parameters should use constructible types so callers can pass values
            let param_ty = self.gen_local_type();
            param_types.push(param_ty.clone());
            params.push(ast::FuncParam {
                span: SrcSpan::default(),
                attributes: None,
                mutability: None,
                name: format!("a_{}", i).into(),
                ty: param_ty,
                default_value: None,
            });
        }

        let func_params = ast::FuncParams {
            span: SrcSpan::default(),
            params,
            variadic: false,
        };

        // Return type should also be constructible (avoid ref/ptr/fn returns for now)
        let return_type = if self.next_bool() {
            Some(self.gen_local_type())
        } else {
            None
        };

        let body_ty = return_type.clone().unwrap_or_else(unit_type);

        self.push_frame();
        for p in &func_params.params {
            let local_name: String = p.name.to_string();
            self.add_local(local_name, p.ty.clone());
        }

        let func_info = FuncInfo {
            name: name.clone(),
            params: param_types,
            return_type: return_type.clone(),
        };
        self.register_function(func_info);

        // Budget is spent based on function weight, not on individual block generation
        let definition = Some(self.gen_block_with_return(&body_ty));
        self.pop_frame();

        ast::Function {
            span: SrcSpan::default(),
            visibility: None,
            attributes: None,
            name: name.into(),
            generics: None,
            parameters: func_params,
            return_type,
            definition,
            abi: None,
        }
    }

    pub(crate) fn gen_unique_name(&mut self, prefix: &str) -> String {
        let counter = self.name_counter;
        self.name_counter = self.name_counter.wrapping_add(1);
        format!("{}_{}", prefix, counter)
    }

    pub(crate) fn gen_unique_suffix(&mut self) -> u64 {
        let counter = self.name_counter;
        self.name_counter = self.name_counter.wrapping_add(1);
        counter
    }
}
