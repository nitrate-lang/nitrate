use crate::Gen;
use nitrate_translation::{
    nstring::NString,
    parsetree::ast::{self, *},
};
use std::unreachable;

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
        for i in 0..choices.len() {
            if roll < choices[i].1 {
                return match choices[i].0 {
                    ast::ItemKind::SyntaxError => ast::ItemKind::SyntaxError,
                    ast::ItemKind::Module => ast::ItemKind::Module,
                    ast::ItemKind::Import => ast::ItemKind::Import,
                    ast::ItemKind::TypeAlias => ast::ItemKind::TypeAlias,
                    ast::ItemKind::Struct => ast::ItemKind::Struct,
                    ast::ItemKind::Enum => ast::ItemKind::Enum,
                    ast::ItemKind::Trait => ast::ItemKind::Trait,
                    ast::ItemKind::Impl => ast::ItemKind::Impl,
                    ast::ItemKind::Function => ast::ItemKind::Function,
                    ast::ItemKind::Variable => ast::ItemKind::Variable,
                };
            }
            roll -= choices[i].1;
        }
        ast::ItemKind::Function
    }

    pub(crate) fn gen_item(&mut self, argc: Option<u32>) -> ast::Item {
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

    fn gen_item_module(&mut self) -> ast::Item {
        let item_count = 1 + self.gen_index(3);
        let mut items = Vec::with_capacity(item_count);
        for _ in 0..item_count {
            items.push(self.gen_item(None));
        }
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
            let name = import_names[self.gen_index(import_names.len())].to_string();
            segments.push(ast::ItemPathSegment {
                span: SrcSpan::default(),
                segment: name,
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
        self.register_struct(name.clone());
        let field_count = 1 + self.gen_index(6);
        let mut fields = Vec::with_capacity(field_count);
        for i in 0..field_count {
            fields.push(ast::StructField {
                span: SrcSpan::default(),
                visibility: None,
                attributes: None,
                name: format!("field_{i}").into(),
                ty: self.gen_type(),
                default_value: None,
            });
        }
        ast::Item::Struct(ast::Struct {
            span: SrcSpan::default(),
            visibility: None,
            attributes: None,
            name: name.into(),
            generics: None,
            fields,
        })
    }

    fn gen_item_enum(&mut self) -> ast::Item {
        let variant_count = 2 + self.gen_index(5);
        let mut variants = Vec::with_capacity(variant_count);
        for _i in 0..variant_count {
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
            name: self.gen_unique_name("Enum").into(),
            generics: None,
            variants,
        })
    }

    fn gen_item_trait(&mut self) -> ast::Item {
        let method_count = 1 + self.gen_index(3);
        let mut items = Vec::with_capacity(method_count);
        for _ in 0..method_count {
            let params = ast::FuncParams {
                span: SrcSpan::default(),
                params: Vec::new(),
                variadic: false,
            };
            items.push(ast::AssociatedItem::Method(ast::Function {
                span: SrcSpan::default(),
                visibility: Some(ast::Visibility::Public),
                attributes: None,
                name: self.gen_unique_name("method").into(),
                generics: None,
                parameters: params,
                return_type: Some(self.gen_type()),
                definition: None,
                abi: None,
            }));
        }
        ast::Item::Trait(ast::Trait {
            span: SrcSpan::default(),
            visibility: None,
            attributes: None,
            name: self.gen_unique_name("Trait").into(),
            generics: None,
            items,
        })
    }

    fn gen_item_impl(&mut self) -> ast::Item {
        let for_type = self.gen_type();
        let method_count = 1 + self.gen_index(2);
        let mut items = Vec::with_capacity(method_count);
        for _ in 0..method_count {
            let func = self.gen_function(None);
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

    fn gen_function(&mut self, argc: Option<u32>) -> ast::Function {
        let name = if self.known_functions.is_empty() {
            "main".to_string()
        } else {
            self.gen_unique_name("fn")
        };
        self.register_function(name.clone());

        let param_count = match argc {
            Some(n) => n as usize,
            None => self.gen_index(6),
        };
        let mut params = Vec::with_capacity(param_count);
        for i in 0..param_count {
            let param_ty = self.gen_type();
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
            let s: &str = &p.name;
            let local_name: String = String::from(s);
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

    fn gen_item_function(&mut self, argc: Option<u32>) -> ast::Item {
        ast::Item::Function(self.gen_function(argc))
    }

    fn gen_item_variable(&mut self) -> ast::Item {
        let ty = self.gen_type();
        let initializer = if self.next_bool() {
            Some(self.gen_rvalue(&ty))
        } else {
            None
        };
        ast::Item::Variable(ast::GlobalVariable {
            span: SrcSpan::default(),
            visibility: None,
            kind: if self.next_bool() {
                ast::GlobalVariableKind::Const
            } else {
                ast::GlobalVariableKind::Static
            },
            attributes: None,
            mutability: if self.next_bool() {
                Some(ast::Mutability::Mut)
            } else {
                None
            },
            name: self.gen_unique_name("VAR").into(),
            ty: Some(ty),
            initializer,
        })
    }

    fn gen_block_with_return(&mut self, ret_ty: &ast::Type) -> ast::Block {
        let stmt_count = self.gen_index(5);
        let mut elements = Vec::with_capacity(stmt_count + 1);
        for _ in 0..stmt_count {
            if self.next_bool() {
                let var_name = format!("v_{}", self.next_u64() & 0xFFF);
                let init_ty = self.gen_type();
                let init = self.gen_rvalue(&init_ty);
                self.add_local(var_name.clone(), init_ty.clone());
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
                let stmt_ty = self.gen_type();
                let expr = self.gen_rvalue(&stmt_ty);
                elements.push(ast::BlockItem::Stmt(expr));
            }
        }
        if self.next_bool() {
            elements.push(ast::BlockItem::Expr(self.gen_rvalue(ret_ty)));
        } else {
            let ret_val = if self.next_bool() {
                Some(self.gen_rvalue(ret_ty))
            } else {
                None
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
        let suffix = self.gen_unique_suffix();
        let mut name = String::from(prefix);
        name.push('_');
        name.push_str(&suffix.to_string());
        name
    }

    fn gen_unique_suffix(&mut self) -> u64 {
        self.next_u64() & 0xFFFF
    }
}
