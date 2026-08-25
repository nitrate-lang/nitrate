use crate::ty::{int32_type, unit_type};
use crate::{Gen, MAX_ITEM_DEPTH};
use nitrate_translation::parsetree::ast::{self, *};

// Item kind codes for weighted selection.
const I_STRUCT: u8 = 0;
const I_ENUM: u8 = 1;
const I_CONST: u8 = 2;
const I_TYPE_ALIAS: u8 = 3;
const I_MODULE: u8 = 4;
const I_FUNCTION: u8 = 5;

impl Gen {
    fn select_item_kind(&mut self) -> u8 {
        let mut opts: Vec<(u32, u8)> = vec![
            (30, I_FUNCTION),
            (22, I_STRUCT),
            (16, I_ENUM),
            (14, I_CONST),
            (10, I_TYPE_ALIAS),
        ];
        if self.item_depth < MAX_ITEM_DEPTH {
            opts.push((8, I_MODULE));
        }
        self.pick_weighted(&opts)
    }

    pub(crate) fn gen_item(&mut self) -> ast::Item {
        if self.budget_left() < 3 {
            return self.gen_item_type_alias();
        }
        match self.select_item_kind() {
            I_STRUCT => self.gen_item_struct(),
            I_ENUM => self.gen_item_enum(),
            I_CONST => self.gen_item_variable(),
            I_TYPE_ALIAS => self.gen_item_type_alias(),
            I_MODULE => self.gen_item_module(),
            _ => self.gen_item_function(),
        }
    }

    pub(crate) fn gen_item_main(&mut self) -> ast::Item {
        // main() may return either i32 or unit; both are valid entry points.
        let return_type = if self.next_bool() { Some(int32_type()) } else { None };
        let body_ty = return_type.clone().unwrap_or_else(unit_type);
        let body = self.gen_function_body(&body_ty);

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
            definition: Some(body),
            abi: None,
        })
    }

    pub(crate) fn gen_item_function(&mut self) -> ast::Item {
        if !self.spend_budget(10) {
            return self.gen_item_type_alias();
        }
        let name = self.gen_unique_name("fn");

        // 0..=3 parameters with concrete, constructible types.
        let param_count = self.gen_index(4);
        let mut params = Vec::with_capacity(param_count);
        for i in 0..param_count {
            let param_ty = self.gen_concrete_type();
            params.push(ast::FuncParam {
                span: SrcSpan::default(),
                attributes: None,
                mutability: None,
                name: format!("a_{}", i).into(),
                ty: param_ty,
                default_value: None,
            });
        }

        // Return type: unit (None) or a concrete type.
        let return_type = if self.next_bool() {
            Some(self.gen_concrete_type())
        } else {
            None
        };
        let body_ty = return_type.clone().unwrap_or_else(unit_type);
        let body = self.gen_function_body(&body_ty);

        ast::Item::Function(ast::Function {
            span: SrcSpan::default(),
            visibility: None,
            attributes: None,
            name: name.into(),
            generics: None,
            parameters: ast::FuncParams {
                span: SrcSpan::default(),
                params,
                variadic: false,
            },
            return_type,
            definition: Some(body),
            abi: None,
        })
    }

    /// A function body: 0-2 non-divergent statements, then a `ret` ending.
    /// The return value type matches `ret_ty`.
    fn gen_function_body(&mut self, ret_ty: &ast::Type) -> ast::Block {
        let mut elements = Vec::new();
        let stmt_count = self.gen_index(3);
        for _ in 0..stmt_count {
            if self.budget_left() < 2 {
                break;
            }
            self.emit_safe_statement(&mut elements, true);
        }
        if crate::ty::is_unit_type(ret_ty) {
            elements.push(BlockItem::Stmt(ast::Expr::Return(Box::new(ast::Return {
                span: SrcSpan::default(),
                value: None,
            }))));
        } else {
            let value = self.gen_value(ret_ty);
            elements.push(BlockItem::Stmt(ast::Expr::Return(Box::new(ast::Return {
                span: SrcSpan::default(),
                value: Some(value),
            }))));
        }
        ast::Block {
            span: SrcSpan::default(),
            safety: None,
            elements,
        }
    }

    fn gen_item_struct(&mut self) -> ast::Item {
        if !self.spend_budget(5) {
            return self.gen_item_type_alias();
        }
        let name = self.gen_unique_name("Struct");
        let field_count = 1 + self.gen_index(4);
        let mut ast_fields = Vec::with_capacity(field_count);
        for i in 0..field_count {
            let field_ty = self.gen_field_type();
            ast_fields.push(ast::StructField {
                span: SrcSpan::default(),
                visibility: None,
                attributes: None,
                name: format!("field_{}", i).into(),
                ty: field_ty,
                default_value: None,
            });
        }
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
        if !self.spend_budget(5) {
            return self.gen_item_type_alias();
        }
        let name = self.gen_unique_name("Enum");
        let variant_count = 2 + self.gen_index(3);
        let mut variants = Vec::with_capacity(variant_count);
        for i in 0..variant_count {
            let payload_ty = if self.next_bool() {
                Some(self.gen_field_type())
            } else {
                None
            };
            variants.push(ast::EnumVariant {
                span: SrcSpan::default(),
                attributes: None,
                name: format!("Variant_{}", i).into(),
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

    fn gen_item_variable(&mut self) -> ast::Item {
        if !self.spend_budget(3) {
            return self.gen_item_type_alias();
        }
        let declared_ty = self.gen_concrete_type();
        let initializer = self.gen_leaf_value(&declared_ty);
        ast::Item::Variable(ast::GlobalVariable {
            span: SrcSpan::default(),
            visibility: None,
            kind: ast::GlobalVariableKind::Const,
            attributes: None,
            mutability: None,
            name: self.gen_unique_name("CONST").into(),
            ty: Some(declared_ty),
            initializer: Some(initializer),
        })
    }

    fn gen_item_type_alias(&mut self) -> ast::Item {
        if !self.spend_budget(2) {
            // Not enough budget even for a type alias: still emit one so the
            // caller always gets a valid item.
            self.budget = 0;
        }
        let alias_type = Some(self.gen_concrete_type());
        ast::Item::TypeAlias(ast::TypeAlias {
            span: SrcSpan::default(),
            visibility: None,
            attributes: None,
            name: self.gen_unique_name("Type").into(),
            generics: None,
            alias_type,
        })
    }

    fn gen_item_module(&mut self) -> ast::Item {
        if !self.spend_budget(4) {
            return self.gen_item_type_alias();
        }
        self.item_depth += 1;
        let item_count = 1 + self.gen_index(3);
        let mut items = Vec::with_capacity(item_count);
        for _ in 0..item_count {
            if self.budget_left() < 3 {
                break;
            }
            items.push(self.gen_item());
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
}
