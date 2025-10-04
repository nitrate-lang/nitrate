use std::sync::{Arc, RwLock};

use super::parse::Parser;
use crate::diagnosis::SyntaxErr;

use nitrate_parsetree::{
    kind::{
        AssociatedItem, Enum, EnumVariant, FuncParam, FuncParams, Function, Impl, Import, Item,
        ItemPath, ItemPathSegment, ItemSyntaxError, Module, Mutability, Struct, StructField, Trait,
        TypeAlias, TypeParam, TypeParams, Variable, VariableKind, Visibility,
    },
    tag::{
        intern_enum_variant_name, intern_function_name, intern_import_name, intern_module_name,
        intern_parameter_name, intern_struct_field_name, intern_trait_name, intern_type_name,
        intern_variable_name,
    },
};
use nitrate_tokenize::Token;

impl Parser<'_, '_> {
    fn parse_type_params(&mut self) -> Option<TypeParams> {
        fn parse_generic_parameter(this: &mut Parser) -> TypeParam {
            let name = this.lexer.next_if_name().unwrap_or_else(|| {
                let bug = SyntaxErr::GenericMissingParameterName(this.lexer.peek_pos());
                this.log.report(&bug);
                "".into()
            });

            let name = intern_parameter_name(name);

            let default = if this.lexer.skip_if(&Token::Eq) {
                Some(this.parse_type())
            } else {
                None
            };

            TypeParam {
                name,
                default_value: default,
            }
        }

        if !self.lexer.skip_if(&Token::Lt) {
            return None;
        }

        let mut params = Vec::new();

        while !self.lexer.skip_if(&Token::Gt) {
            if self.lexer.is_eof() {
                let bug = SyntaxErr::GenericParameterExpectedEnd(self.lexer.peek_pos());
                self.log.report(&bug);
                break;
            }

            const MAX_GENERIC_PARAMETERS: usize = 65_536;

            if params.len() >= MAX_GENERIC_PARAMETERS {
                let bug = SyntaxErr::GenericParameterLimit(self.lexer.peek_pos());
                self.log.report(&bug);
                break;
            }

            let param = parse_generic_parameter(self);
            params.push(param);

            if !self.lexer.skip_if(&Token::Comma) && !self.lexer.next_is(&Token::Gt) {
                let bug = SyntaxErr::GenericParameterExpectedEnd(self.lexer.peek_pos());
                self.log.report(&bug);

                self.lexer.skip_while(&Token::Gt);
                break;
            }
        }

        Some(TypeParams { params })
    }

    fn parse_module(&mut self) -> Module {
        let module_start_pos = self.lexer.peek_pos();

        assert!(self.lexer.peek_t() == Token::Mod);
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();

        let name = self.lexer.next_if_name().unwrap_or_else(|| {
            let bug = SyntaxErr::ModuleMissingName(self.lexer.peek_pos());
            self.log.report(&bug);
            "".into()
        });

        let name = intern_module_name(name);

        if !self.lexer.skip_if(&Token::OpenBrace) {
            let bug = SyntaxErr::ExpectedOpenBrace(self.lexer.peek_pos());
            self.log.report(&bug);
        }

        let mut items = Vec::new();
        let mut already_reported_too_many_items = false;

        while !self.lexer.skip_if(&Token::CloseBrace) {
            if self.lexer.is_eof() {
                let bug = SyntaxErr::ModuleExpectedEnd(self.lexer.peek_pos());
                self.log.report(&bug);
                break;
            }

            const MAX_ITEMS_PER_MODULE: usize = 65_536;

            if !already_reported_too_many_items && items.len() >= MAX_ITEMS_PER_MODULE {
                already_reported_too_many_items = true;

                let bug = SyntaxErr::ModuleItemLimit(module_start_pos.clone());
                self.log.report(&bug);
            }

            let item = self.parse_item();
            items.push(item);
        }

        Module {
            visibility: None,
            attributes,
            name,
            items,
        }
    }

    #[allow(dead_code)]
    fn parse_item_path(&mut self) -> ItemPath {
        fn parse_double_colon(this: &mut Parser) -> bool {
            if !this.lexer.skip_if(&Token::Colon) {
                return false;
            }

            if !this.lexer.skip_if(&Token::Colon) {
                let bug = SyntaxErr::ExpectedColon(this.lexer.peek_pos());
                this.log.report(&bug);
                return false;
            }

            true
        }

        let mut segments = Vec::new();

        if parse_double_colon(self) {
            segments.push(ItemPathSegment { segment: "".into() });
        }

        while !self.lexer.is_eof() {
            let Some(segment) = self.lexer.next_if_name() else {
                let bug = SyntaxErr::PathExpectedName(self.lexer.peek_pos());
                self.log.report(&bug);
                break;
            };

            segments.push(ItemPathSegment { segment });

            if !parse_double_colon(self) {
                break;
            }
        }

        ItemPath { segments }
    }

    fn parse_import(&mut self) -> Import {
        assert!(self.lexer.peek_t() == Token::Use);
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();
        let import_name = self.lexer.next_if_name().unwrap_or_else(|| {
            let bug = SyntaxErr::ImportMissingName(self.lexer.peek_pos());
            self.log.report(&bug);
            "".into()
        });

        if !self.lexer.skip_if(&Token::Semi) {
            let bug = SyntaxErr::ExpectedSemicolon(self.lexer.peek_pos());
            self.log.report(&bug);
        }

        Import {
            visibility: None,
            attributes,
            import_name: intern_import_name(import_name),
            items: None,
            resolved: None,
        }
    }

    fn parse_type_alias(&mut self) -> TypeAlias {
        assert!(self.lexer.peek_t() == Token::Type);
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();

        let name = self.lexer.next_if_name().unwrap_or_else(|| {
            let bug = SyntaxErr::TypeAliasMissingName(self.lexer.peek_pos());
            self.log.report(&bug);
            "".into()
        });

        let name = intern_type_name(name);

        let type_params = self.parse_type_params();

        let aliased_type = if self.lexer.skip_if(&Token::Eq) {
            Some(self.parse_type())
        } else {
            None
        };

        if !self.lexer.skip_if(&Token::Semi) {
            let bug = SyntaxErr::ExpectedSemicolon(self.lexer.peek_pos());
            self.log.report(&bug);
        }

        TypeAlias {
            visibility: None,
            attributes,
            name,
            type_params,
            alias_type: aliased_type,
        }
    }

    fn parse_enum(&mut self) -> Enum {
        fn parse_enum_variant(this: &mut Parser) -> EnumVariant {
            let visibility = this.parse_visibility();
            let attributes = this.parse_attributes();

            let name = this.lexer.next_if_name().unwrap_or_else(|| {
                let bug = SyntaxErr::EnumMissingVariantName(this.lexer.peek_pos());
                this.log.report(&bug);
                "".into()
            });

            let name = intern_enum_variant_name(name);

            let variant_type = if this.lexer.next_is(&Token::OpenParen) {
                Some(this.parse_type())
            } else {
                None
            };

            let value = if this.lexer.skip_if(&Token::Eq) {
                Some(this.parse_expression())
            } else {
                None
            };

            EnumVariant {
                visibility,
                attributes,
                name,
                variant_type,
                value,
            }
        }

        assert!(self.lexer.peek_t() == Token::Enum);
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();

        let name = self.lexer.next_if_name().unwrap_or_else(|| {
            let bug = SyntaxErr::EnumMissingName(self.lexer.peek_pos());
            self.log.report(&bug);
            "".into()
        });

        let name = intern_type_name(name);

        let type_params = self.parse_type_params();

        if !self.lexer.skip_if(&Token::OpenBrace) {
            let bug = SyntaxErr::ExpectedOpenBrace(self.lexer.peek_pos());
            self.log.report(&bug);
        }

        let mut variants = Vec::new();
        let mut already_reported_too_many_variants = false;

        while !self.lexer.skip_if(&Token::CloseBrace) {
            if self.lexer.is_eof() {
                let bug = SyntaxErr::EnumExpectedEnd(self.lexer.peek_pos());
                self.log.report(&bug);
                break;
            }

            const MAX_ENUM_VARIANTS: usize = 65_536;

            if !already_reported_too_many_variants && variants.len() >= MAX_ENUM_VARIANTS {
                already_reported_too_many_variants = true;

                let bug = SyntaxErr::EnumVariantLimit(self.lexer.peek_pos());
                self.log.report(&bug);
            }

            let variant = parse_enum_variant(self);
            variants.push(variant);

            if !self.lexer.skip_if(&Token::Comma) && !self.lexer.next_is(&Token::CloseBrace) {
                let bug = SyntaxErr::EnumExpectedEnd(self.lexer.peek_pos());
                self.log.report(&bug);
                self.lexer.skip_while(&Token::CloseBrace);

                break;
            }
        }

        Enum {
            visibility: None,
            attributes,
            name,
            type_params,
            variants,
        }
    }

    fn parse_struct(&mut self) -> Struct {
        fn parse_struct_field(this: &mut Parser) -> StructField {
            let visibility = this.parse_visibility();
            let attributes = this.parse_attributes();

            let name = this.lexer.next_if_name().unwrap_or_else(|| {
                let bug = SyntaxErr::StructureMissingFieldName(this.lexer.peek_pos());
                this.log.report(&bug);
                "".into()
            });

            let name = intern_struct_field_name(name);

            if !this.lexer.skip_if(&Token::Colon) {
                let bug = SyntaxErr::ExpectedColon(this.lexer.peek_pos());
                this.log.report(&bug);
            }

            let field_type = this.parse_type();

            let default = if this.lexer.skip_if(&Token::Eq) {
                Some(this.parse_expression())
            } else {
                None
            };

            StructField {
                visibility,
                attributes,
                name,
                field_type,
                default_value: default,
            }
        }

        assert!(self.lexer.peek_t() == Token::Struct);
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();

        let name = self.lexer.next_if_name().unwrap_or_else(|| {
            let bug = SyntaxErr::StructureMissingName(self.lexer.peek_pos());
            self.log.report(&bug);
            "".into()
        });

        let name = intern_type_name(name);

        let type_params = self.parse_type_params();

        if !self.lexer.skip_if(&Token::OpenBrace) {
            let bug = SyntaxErr::ExpectedOpenBrace(self.lexer.peek_pos());
            self.log.report(&bug);
        }

        let mut fields = Vec::new();
        let mut already_reported_too_many_fields = false;

        while !self.lexer.skip_if(&Token::CloseBrace) {
            if self.lexer.is_eof() {
                let bug = SyntaxErr::StructureExpectedEnd(self.lexer.peek_pos());
                self.log.report(&bug);
                break;
            }

            const MAX_STRUCT_FIELDS: usize = 65_536;

            if !already_reported_too_many_fields && fields.len() >= MAX_STRUCT_FIELDS {
                already_reported_too_many_fields = true;

                let bug = SyntaxErr::StructureFieldLimit(self.lexer.peek_pos());
                self.log.report(&bug);
            }

            let field = parse_struct_field(self);
            fields.push(field);

            if !self.lexer.skip_if(&Token::Comma) && !self.lexer.next_is(&Token::CloseBrace) {
                let bug = SyntaxErr::StructureExpectedEnd(self.lexer.peek_pos());
                self.log.report(&bug);
                self.lexer.skip_while(&Token::CloseBrace);
                break;
            }
        }

        Struct {
            visibility: None,
            attributes,
            name,
            type_params,
            fields,
            methods: Vec::new(),
        }
    }

    fn parse_associated_item(&mut self) -> AssociatedItem {
        let visibility = self.parse_visibility();

        match self.lexer.peek_t() {
            Token::Fn => {
                let mut func = self.parse_named_function();
                func.visibility = visibility;
                AssociatedItem::Method(Arc::new(RwLock::new(func)))
            }

            Token::Const => {
                let mut const_var = self.parse_variable();
                const_var.visibility = visibility;
                AssociatedItem::ConstantItem(Arc::new(RwLock::new(const_var)))
            }

            Token::Type => {
                let mut type_alias = self.parse_type_alias();
                type_alias.visibility = visibility;
                AssociatedItem::TypeAlias(Arc::new(RwLock::new(type_alias)))
            }

            _ => {
                self.lexer.skip_tok();

                let bug = SyntaxErr::TraitDoesNotAllowItem(self.lexer.peek_pos());
                self.log.report(&bug);

                AssociatedItem::SyntaxError(ItemSyntaxError)
            }
        }
    }

    fn parse_trait(&mut self) -> Trait {
        assert!(self.lexer.peek_t() == Token::Trait);
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();

        let name = self.lexer.next_if_name().unwrap_or_else(|| {
            let bug = SyntaxErr::TraitMissingName(self.lexer.peek_pos());
            self.log.report(&bug);
            "".into()
        });

        let name = intern_trait_name(name);

        let type_params = self.parse_type_params();

        if !self.lexer.skip_if(&Token::OpenBrace) {
            let bug = SyntaxErr::ExpectedOpenBrace(self.lexer.peek_pos());
            self.log.report(&bug);
        }

        let mut items = Vec::new();
        let mut already_reported_too_many_items = false;

        while !self.lexer.skip_if(&Token::CloseBrace) {
            if self.lexer.is_eof() {
                let bug = SyntaxErr::TraitExpectedEnd(self.lexer.peek_pos());
                self.log.report(&bug);
                break;
            }

            const MAX_TRAIT_ITEMS: usize = 65_536;

            if !already_reported_too_many_items && items.len() >= MAX_TRAIT_ITEMS {
                already_reported_too_many_items = true;

                let bug = SyntaxErr::TraitItemLimit(self.lexer.peek_pos());
                self.log.report(&bug);
            }

            let item = self.parse_associated_item();
            items.push(item);
        }

        Trait {
            visibility: None,
            attributes,
            name,
            type_params,
            items,
        }
    }

    fn parse_implementation(&mut self) -> Impl {
        assert!(self.lexer.peek_t() == Token::Impl);
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();

        let type_params = self.parse_type_params();

        let trait_path = if self.lexer.skip_if(&Token::Trait) {
            let path = self.parse_type_path();

            if !self.lexer.skip_if(&Token::For) {
                let bug = SyntaxErr::ImplMissingFor(self.lexer.peek_pos());
                self.log.report(&bug);
            }

            Some(path)
        } else {
            None
        };

        let for_type = self.parse_type();

        if !self.lexer.skip_if(&Token::OpenBrace) {
            let bug = SyntaxErr::ExpectedOpenBrace(self.lexer.peek_pos());
            self.log.report(&bug);
        }

        let mut items = Vec::new();
        let mut already_reported_too_many_items = false;

        while !self.lexer.skip_if(&Token::CloseBrace) {
            if self.lexer.is_eof() {
                let bug = SyntaxErr::ImplExpectedEnd(self.lexer.peek_pos());
                self.log.report(&bug);
                break;
            }

            const MAX_IMPL_ITEMS: usize = 65_536;

            if !already_reported_too_many_items && items.len() >= MAX_IMPL_ITEMS {
                already_reported_too_many_items = true;

                let bug = SyntaxErr::ImplItemLimit(self.lexer.peek_pos());
                self.log.report(&bug);
            }

            let item = self.parse_associated_item();
            items.push(item);
        }

        Impl {
            attributes,
            type_params,
            trait_path,
            for_type,
            items,
        }
    }

    fn parse_function_parameters(&mut self) -> FuncParams {
        fn parse_function_parameter(this: &mut Parser) -> FuncParam {
            let attributes = this.parse_attributes();

            let mut mutability = None;
            if this.lexer.skip_if(&Token::Mut) {
                mutability = Some(Mutability::Mutable);
            } else if this.lexer.skip_if(&Token::Const) {
                mutability = Some(Mutability::Const);
            }

            let name = this.lexer.next_if_name().unwrap_or_else(|| {
                let bug = SyntaxErr::FunctionParameterMissingName(this.lexer.peek_pos());
                this.log.report(&bug);
                "".into()
            });

            let name = intern_parameter_name(name);

            let param_type = if this.lexer.skip_if(&Token::Colon) {
                Some(this.parse_type())
            } else {
                None
            };

            let default_value = if this.lexer.skip_if(&Token::Eq) {
                Some(this.parse_expression())
            } else {
                None
            };

            FuncParam {
                attributes,
                mutability,
                name,
                param_type,
                default_value,
            }
        }

        let mut params = Vec::new();

        if !self.lexer.skip_if(&Token::OpenParen) {
            let bug = SyntaxErr::ExpectedOpenParen(self.lexer.peek_pos());
            self.log.report(&bug);
        }

        self.lexer.skip_if(&Token::Comma);

        let mut already_reported_too_many_parameters = false;

        while !self.lexer.skip_if(&Token::CloseParen) {
            if self.lexer.is_eof() {
                let bug = SyntaxErr::FunctionParametersExpectedEnd(self.lexer.peek_pos());
                self.log.report(&bug);
                break;
            }

            const MAX_FUNCTION_PARAMETERS: usize = 65_536;

            if !already_reported_too_many_parameters && params.len() >= MAX_FUNCTION_PARAMETERS {
                already_reported_too_many_parameters = true;

                let bug = SyntaxErr::FunctionParameterLimit(self.lexer.peek_pos());
                self.log.report(&bug);
            }

            let param = parse_function_parameter(self);
            params.push(param);

            if !self.lexer.skip_if(&Token::Comma) && !self.lexer.next_is(&Token::CloseParen) {
                let bug = SyntaxErr::FunctionParametersExpectedEnd(self.lexer.peek_pos());
                self.log.report(&bug);
                self.lexer.skip_while(&Token::CloseParen);
                break;
            }
        }

        FuncParams { params }
    }

    fn parse_named_function(&mut self) -> Function {
        assert!(self.lexer.peek_t() == Token::Fn);
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();

        let name = self.lexer.next_if_name().unwrap_or_else(|| {
            let bug = SyntaxErr::FunctionMissingName(self.lexer.peek_pos());
            self.log.report(&bug);
            "".into()
        });

        let name = intern_function_name(name);

        let type_params = self.parse_type_params();
        let parameters = self.parse_function_parameters();

        let return_type = if self.lexer.skip_if(&Token::Minus) {
            if !self.lexer.skip_if(&Token::Gt) {
                let bug = SyntaxErr::ExpectedArrow(self.lexer.peek_pos());
                self.log.report(&bug);
            }

            Some(self.parse_type())
        } else {
            None
        };

        let definition = if self.lexer.skip_if(&Token::Semi) {
            None
        } else {
            Some(self.parse_block())
        };

        Function {
            visibility: None,
            attributes,
            name,
            type_params,
            parameters,
            return_type,
            definition,
        }
    }

    fn parse_visibility(&mut self) -> Option<Visibility> {
        if self.lexer.skip_if(&Token::Pub) {
            Some(Visibility::Public)
        } else if self.lexer.skip_if(&Token::Sec) {
            Some(Visibility::Private)
        } else if self.lexer.skip_if(&Token::Pro) {
            Some(Visibility::Protected)
        } else {
            None
        }
    }

    pub(crate) fn parse_variable(&mut self) -> Variable {
        let kind = match self.lexer.next_t() {
            Token::Static => VariableKind::Static,
            Token::Const => VariableKind::Const,
            Token::Let => VariableKind::Let,
            Token::Var => VariableKind::Var,
            _ => unreachable!(),
        };

        let attributes = self.parse_attributes();

        let mut mutability = None;
        if self.lexer.skip_if(&Token::Mut) {
            mutability = Some(Mutability::Mutable);
        } else if self.lexer.skip_if(&Token::Const) {
            mutability = Some(Mutability::Const);
        }

        let name = self.lexer.next_if_name().unwrap_or_else(|| {
            let bug = SyntaxErr::VariableMissingName(self.lexer.peek_pos());
            self.log.report(&bug);
            "".into()
        });

        let name = intern_variable_name(name);

        let var_type = if self.lexer.skip_if(&Token::Colon) {
            Some(self.parse_type())
        } else {
            None
        };

        let initializer = if self.lexer.skip_if(&Token::Eq) {
            Some(self.parse_expression())
        } else {
            None
        };

        if !self.lexer.skip_if(&Token::Semi) {
            let bug = SyntaxErr::ExpectedSemicolon(self.lexer.peek_pos());
            self.log.report(&bug);
        }

        Variable {
            visibility: None,
            kind,
            attributes,
            mutability,
            name,
            ty: var_type,
            initializer,
        }
    }

    pub(crate) fn parse_item(&mut self) -> Item {
        let visibility = self.parse_visibility();

        let item_pos_begin = self.lexer.peek_pos();

        match self.lexer.peek_t() {
            Token::Mod => {
                let mut module = self.parse_module();
                module.visibility = visibility;
                Item::Module(Box::new(module))
            }

            Token::Use => {
                let mut import = self.parse_import();
                import.visibility = visibility;
                Item::Import(Box::new(import))
            }

            Token::Type => {
                let mut type_alias = self.parse_type_alias();
                type_alias.visibility = visibility;
                Item::TypeAlias(Arc::new(RwLock::new(type_alias)))
            }

            Token::Struct => {
                let mut struct_def = self.parse_struct();
                struct_def.visibility = visibility;
                Item::Struct(Arc::new(RwLock::new(struct_def)))
            }

            Token::Enum => {
                let mut enum_def = self.parse_enum();
                enum_def.visibility = visibility;
                Item::Enum(Arc::new(RwLock::new(enum_def)))
            }

            Token::Trait => {
                let mut trait_def = self.parse_trait();
                trait_def.visibility = visibility;
                Item::Trait(Arc::new(RwLock::new(trait_def)))
            }

            Token::Impl => {
                let impl_def = self.parse_implementation();
                if visibility.is_some() {
                    let bug = SyntaxErr::ImplCannotBeVisible(item_pos_begin);
                    self.log.report(&bug);
                }
                Item::Impl(Box::new(impl_def))
            }

            Token::Fn => {
                let mut func = self.parse_named_function();
                func.visibility = visibility;
                Item::Function(Arc::new(RwLock::new(func)))
            }

            Token::Static | Token::Const | Token::Let | Token::Var => {
                let mut var = self.parse_variable();
                var.visibility = visibility;
                Item::Variable(Arc::new(RwLock::new(var)))
            }

            _ => {
                self.lexer.skip_tok();

                let bug = SyntaxErr::ExpectedItem(item_pos_begin);
                self.log.report(&bug);

                Item::SyntaxError(ItemSyntaxError)
            }
        }
    }
}
