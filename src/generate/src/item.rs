use crate::{FuncInfo, Gen, MAX_ITEM_DEPTH, StructInfo};
use nitrate_translation::{
    nstring::NString,
    parsetree::ast::{self, *},
};

impl Gen {
    fn select_item_kind(&mut self) -> ast::ItemKind {
        let choices: &[(ast::ItemKind, u32)] = &[
            (ast::ItemKind::Function, 40),
            (ast::ItemKind::Struct, 15),
            (ast::ItemKind::Enum, 8),
            (ast::ItemKind::Variable, 10),
            (ast::ItemKind::TypeAlias, 8),
            (ast::ItemKind::Import, 5),
            (ast::ItemKind::Trait, 2),
            (ast::ItemKind::Impl, 3),
            (ast::ItemKind::Module, 1),
        ];

        let total_weight: u32 = choices.iter().map(|(_, w)| w).sum();
        let mut roll = self.next_u64() as u32 % total_weight;
        for (kind, weight) in choices {
            if roll < *weight {
                return match kind {
                    ast::ItemKind::SyntaxError => ast::ItemKind::Function,
                    ast::ItemKind::Module if self.item_depth < MAX_ITEM_DEPTH => ast::ItemKind::Module,
                    ast::ItemKind::Import => ast::ItemKind::Import,
                    ast::ItemKind::TypeAlias => ast::ItemKind::TypeAlias,
                    ast::ItemKind::Struct => ast::ItemKind::Struct,
                    ast::ItemKind::Enum => ast::ItemKind::Enum,
                    ast::ItemKind::Trait => ast::ItemKind::Trait,
                    ast::ItemKind::Impl => ast::ItemKind::Impl,
                    ast::ItemKind::Function => ast::ItemKind::Function,
                    ast::ItemKind::Variable => ast::ItemKind::Variable,
                    ast::ItemKind::Module => {
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
        if !self.spend_budget(2) {
            let name = self.gen_unique_name("empty");
            let func_info = FuncInfo {
                name: name.clone(),
                params: Vec::new(),
                return_type: None,
            };
            self.register_function(func_info);
            return ast::Item::Function(ast::Function {
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
            });
        }

        let kind = match argc {
            Some(_) => ast::ItemKind::Function,
            None => self.select_item_kind(),
        };

        match kind {
            ast::ItemKind::SyntaxError => unreachable!(),
            ast::ItemKind::Module => self.gen_item_module(),
            ast::ItemKind::Import => self.gen_item_import(),
            ast::ItemKind::TypeAlias => self.gen_item_type_alias(),
            ast::ItemKind::Struct => self.gen_item_struct(),
            ast::ItemKind::Enum => self.gen_item_enum(),
            ast::ItemKind::Trait => self.gen_item_trait(),
            ast::ItemKind::Impl => self.gen_item_impl(),
            ast::ItemKind::Function => self.gen_item_function(argc),
            ast::ItemKind::Variable => self.gen_item_variable(),
        }
    }

    pub(crate) fn gen_item_main(&mut self) -> ast::Item {
        self.has_main = true;
        self.push_frame();

        let return_type = if self.next_bool() {
            Some(ast::Type::Int32(ast::Int32 {
                span: SrcSpan::default(),
            }))
        } else {
            None
        };

        let body_ty = return_type.clone().unwrap_or_else(|| {
            ast::Type::TupleType(Box::new(ast::TupleType {
                span: SrcSpan::default(),
                element_types: vec![],
            }))
        });

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
        if !self.spend_budget(3) {
            return self.gen_item(None);
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

    fn gen_item_import(&mut self) -> ast::Item {
        let seg_count = 1 + self.gen_index(3);
        let import_names = ["std", "core", "math", "io", "fs", "net", "util", "prelude"];
        let mut segments = Vec::with_capacity(seg_count);
        for _ in 0..seg_count {
            segments.push(ast::ItemPathSegment {
                span: SrcSpan::default(),
                segment: import_names[self.gen_index(import_names.len())].to_string(),
                prefix: None,
            });
        }
        let use_tree = ast::UseTree::Single {
            span: SrcSpan::default(),
            path: ast::ItemPath {
                span: SrcSpan::default(),
                segments,
            },
        };
        ast::Item::Import(Box::new(ast::Import {
            span: SrcSpan::default(),
            visibility: None,
            attributes: None,
            use_tree,
            resolved: None,
        }))
    }

    fn gen_item_type_alias(&mut self) -> ast::Item {
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
        let name = self.gen_unique_name("Struct");
        let field_count = 1 + self.gen_index(6);
        let mut fields: Vec<(String, ast::Type)> = Vec::with_capacity(field_count);
        let mut ast_fields = Vec::with_capacity(field_count);
        for i in 0..field_count {
            let field_ty = self.gen_type();
            let field_name = format!("field_{i}");
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
        let name = self.gen_unique_name("Enum");
        // Register as a struct-like type path for referencing.
        self.register_struct(StructInfo {
            name: name.clone(),
            fields: Vec::new(),
        });
        let variant_count = 2 + self.gen_index(5);
        let mut variants = Vec::with_capacity(variant_count);
        for _ in 0..variant_count {
            let variant_name: NString = format!("Variant_{}", self.gen_unique_suffix()).into();
            variants.push(ast::EnumVariant {
                span: SrcSpan::default(),
                attributes: None,
                name: variant_name,
                ty: if self.next_bool() { Some(self.gen_type()) } else { None },
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

    fn gen_item_trait(&mut self) -> ast::Item {
        let trait_name = self.gen_unique_name("Trait");
        self.register_struct(StructInfo {
            name: trait_name.clone(),
            fields: Vec::new(),
        });
        let method_count = 1 + self.gen_index(3);
        let mut items = Vec::with_capacity(method_count);
        for _ in 0..method_count {
            let param_count = self.next_u64() as usize % 3;
            let params: Vec<ast::FuncParam> = (0..param_count)
                .map(|i| ast::FuncParam {
                    span: SrcSpan::default(),
                    attributes: None,
                    mutability: None,
                    name: format!("a_{i}").into(),
                    ty: self.gen_type(),
                    default_value: None,
                })
                .collect();
            let func_params = ast::FuncParams {
                span: SrcSpan::default(),
                params,
                variadic: false,
            };
            items.push(ast::AssociatedItem::Method(ast::Function {
                span: SrcSpan::default(),
                visibility: Some(ast::Visibility::Public),
                attributes: None,
                name: self.gen_unique_name("method").into(),
                generics: None,
                parameters: func_params,
                return_type: Some(self.gen_type()),
                definition: None,
                abi: None,
            }));
        }
        ast::Item::Trait(ast::Trait {
            span: SrcSpan::default(),
            visibility: None,
            attributes: None,
            name: trait_name.into(),
            generics: None,
            items,
        })
    }

    fn gen_item_impl(&mut self) -> ast::Item {
        let for_type = if self.has_any_struct() {
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
            ast::Type::Int32(ast::Int32 {
                span: SrcSpan::default(),
            })
        };

        let method_count = 1 + self.gen_index(2);
        let mut items = Vec::with_capacity(method_count);
        for _ in 0..method_count {
            let func = self.gen_function_local(None);
            items.push(ast::AssociatedItem::Method(func));
        }
        ast::Item::Impl(Box::new(ast::Impl {
            span: SrcSpan::default(),
            generics: None,
            trait_path: None,
            for_type,
            items,
        }))
    }

    fn gen_function_local(&mut self, argc: Option<u32>) -> ast::Function {
        let name = self.gen_unique_name("method");

        let param_count = match argc {
            Some(n) => n as usize,
            None => self.gen_index(4),
        };
        let mut params = Vec::with_capacity(param_count);
        let mut param_types = Vec::with_capacity(param_count);
        for i in 0..param_count {
            let param_ty = self.gen_type();
            param_types.push(param_ty.clone());
            params.push(ast::FuncParam {
                span: SrcSpan::default(),
                attributes: None,
                mutability: None,
                name: format!("a_{i}").into(),
                ty: param_ty,
                default_value: None,
            });
        }

        let func_params = ast::FuncParams {
            span: SrcSpan::default(),
            params,
            variadic: false,
        };

        let return_type = if self.next_bool() { Some(self.gen_type()) } else { None };

        let body_ty = return_type.clone().unwrap_or_else(|| {
            ast::Type::TupleType(Box::new(ast::TupleType {
                span: SrcSpan::default(),
                element_types: vec![],
            }))
        });

        self.push_frame();
        for p in &func_params.params {
            let local_name: String = p.name.to_string();
            self.add_local(local_name, p.ty.clone());
        }

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

    fn gen_function(&mut self, argc: Option<u32>) -> ast::Function {
        let name = self.gen_unique_name("fn");

        let param_count = match argc {
            Some(n) => n as usize,
            None => self.gen_index(6),
        };
        let mut params = Vec::with_capacity(param_count);
        let mut param_types = Vec::with_capacity(param_count);
        for i in 0..param_count {
            let param_ty = self.gen_type();
            param_types.push(param_ty.clone());
            params.push(ast::FuncParam {
                span: SrcSpan::default(),
                attributes: None,
                mutability: None,
                name: format!("a_{i}").into(),
                ty: param_ty,
                default_value: None,
            });
        }

        let func_params = ast::FuncParams {
            span: SrcSpan::default(),
            params,
            variadic: false,
        };

        let return_type = if self.next_bool() { Some(self.gen_type()) } else { None };

        let body_ty = return_type.clone().unwrap_or_else(|| {
            ast::Type::TupleType(Box::new(ast::TupleType {
                span: SrcSpan::default(),
                element_types: vec![],
            }))
        });

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

    fn gen_item_function(&mut self, argc: Option<u32>) -> ast::Item {
        ast::Item::Function(self.gen_function(argc))
    }

    fn gen_item_variable(&mut self) -> ast::Item {
        let declared_ty = self.gen_type();
        let kind = if self.next_bool() {
            ast::GlobalVariableKind::Const
        } else {
            ast::GlobalVariableKind::Static
        };
        let mutability = if matches!(kind, ast::GlobalVariableKind::Const) {
            None
        } else if self.next_bool() {
            Some(ast::Mutability::Mut)
        } else {
            None
        };
        let initializer = if self.next_bool() {
            let init_expr = self.gen_const_initializer(&declared_ty);
            Some(init_expr)
        } else {
            None
        };
        ast::Item::Variable(ast::GlobalVariable {
            span: SrcSpan::default(),
            visibility: None,
            kind,
            attributes: None,
            mutability,
            name: self.gen_unique_name("VAR").into(),
            ty: Some(declared_ty),
            initializer,
        })
    }

    fn gen_const_initializer(&mut self, ty: &ast::Type) -> ast::Expr {
        match ty {
            ast::Type::Bool(_) => ast::Expr::Boolean(ast::BooleanLit {
                span: SrcSpan::default(),
                value: self.next_bool(),
            }),
            ast::Type::Float32(_) | ast::Type::Float64(_) => {
                let bits = self.next_u64();
                let raw = f64::from_bits((bits >> 8) | 0x3FF0000000000000);
                let value = ordered_float::NotNan::new(raw).unwrap_or(ordered_float::NotNan::new(1.0).unwrap());
                ast::Expr::Float(ast::FloatLit {
                    span: SrcSpan::default(),
                    value,
                })
            }
            _ => ast::Expr::Integer(Box::new(ast::IntegerLit {
                span: SrcSpan::default(),
                value: self.next_u64() as u128,
                kind: nitrate_translation::token::IntegerKind::Dec,
            })),
        }
    }

    fn gen_block_with_return(&mut self, ret_ty: &ast::Type) -> ast::Block {
        let stmt_count = self.gen_index(5);
        let mut elements = Vec::with_capacity(stmt_count + 1);
        for _ in 0..stmt_count {
            if self.next_bool() {
                let var_name_prefix = format!("v_{}", self.next_u64() & 0xFFF);
                let init_ty = self.gen_type();
                let init = self.gen_rvalue(&init_ty);
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
                let stmt_ty = self.gen_type();
                let expr = self.gen_rvalue(&stmt_ty);
                elements.push(ast::BlockItem::Stmt(expr));
            }
        }
        if self.next_bool() {
            elements.push(ast::BlockItem::Expr(self.gen_rvalue(ret_ty)));
        } else {
            let ret_val = if ret_ty_is_unit(ret_ty) {
                None
            } else {
                Some(self.gen_rvalue(ret_ty))
            };
            elements.push(ast::BlockItem::Expr(ast::Expr::Return(Box::new(ast::Return {
                span: SrcSpan::default(),
                value: ret_val,
            }))));
        }
        ast::Block {
            span: SrcSpan::default(),
            safety: None,
            elements,
        }
    }

    fn gen_unique_name(&mut self, prefix: &str) -> String {
        let counter = self.name_counter;
        self.name_counter = self.name_counter.wrapping_add(1);
        format!("{}_{}", prefix, counter)
    }

    fn gen_unique_suffix(&mut self) -> u64 {
        let counter = self.name_counter;
        self.name_counter = self.name_counter.wrapping_add(1);
        counter
    }
}

fn ret_ty_is_unit(ty: &ast::Type) -> bool {
    matches!(ty, ast::Type::TupleType(t) if t.element_types.is_empty())
}
