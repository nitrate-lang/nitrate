use super::parse::Parser;
use crate::bugs::SyntaxBug;

use nitrate_parsetree::{
    kind::{
        AssociatedItem, Enum, EnumVariant, FunctionParameter, GenericParameter, Impl, Import, Item,
        Module, Mutability, NamedFunction, Struct, StructField, Trait, TypeAlias, Variable,
        VariableKind, Visibility,
    },
    tag::{
        intern_enum_variant_name_id, intern_function_name_id, intern_import_alias_name_id,
        intern_module_name_id, intern_parameter_name_id, intern_struct_field_name_id,
        intern_trait_name_id, intern_type_name_id, intern_variable_name_id,
    },
};
use nitrate_tokenize::Token;

impl Parser<'_, '_> {
    fn parse_generic_parameters(&mut self) -> Option<Vec<GenericParameter>> {
        fn parse_generic_parameter(this: &mut Parser) -> GenericParameter {
            let name = this.lexer.next_if_name().unwrap_or_else(|| {
                let bug = SyntaxBug::GenericMissingParameterName(this.lexer.peek_pos());
                this.bugs.push(&bug);
                "".into()
            });

            let name = intern_parameter_name_id(name);

            let default = if this.lexer.skip_if(&Token::Eq) {
                Some(this.parse_type())
            } else {
                None
            };

            GenericParameter { name, default }
        }

        if !self.lexer.skip_if(&Token::Lt) {
            return None;
        }

        let mut parameters = Vec::new();

        while !self.lexer.skip_if(&Token::Gt) {
            if self.lexer.is_eof() {
                let bug = SyntaxBug::GenericParameterExpectedEnd(self.lexer.peek_pos());
                self.bugs.push(&bug);
                break;
            }

            const MAX_GENERIC_PARAMETERS: usize = 65_536;

            if parameters.len() >= MAX_GENERIC_PARAMETERS {
                let bug = SyntaxBug::GenericParameterLimit(self.lexer.peek_pos());
                self.bugs.push(&bug);
                break;
            }

            let param = parse_generic_parameter(self);
            parameters.push(param);

            if !self.lexer.skip_if(&Token::Comma) && !self.lexer.next_is(&Token::Gt) {
                let bug = SyntaxBug::GenericParameterExpectedEnd(self.lexer.peek_pos());
                self.bugs.push(&bug);

                self.lexer.skip_while(&Token::Gt);
                break;
            }
        }

        Some(parameters)
    }

    fn parse_module(&mut self) -> Module {
        let module_start_pos = self.lexer.peek_pos();

        assert!(self.lexer.peek_t() == Token::Mod);
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();

        let name = self.lexer.next_if_name().unwrap_or_else(|| {
            let bug = SyntaxBug::ModuleMissingName(self.lexer.peek_pos());
            self.bugs.push(&bug);
            "".into()
        });

        let name = intern_module_name_id(name);

        if !self.lexer.skip_if(&Token::OpenBrace) {
            let bug = SyntaxBug::ExpectedOpenBrace(self.lexer.peek_pos());
            self.bugs.push(&bug);
        }

        let mut items = Vec::new();
        let mut already_reported_too_many_items = false;

        while !self.lexer.skip_if(&Token::CloseBrace) {
            if self.lexer.is_eof() {
                let bug = SyntaxBug::ModuleExpectedEnd(self.lexer.peek_pos());
                self.bugs.push(&bug);
                break;
            }

            const MAX_ITEMS_PER_MODULE: usize = 65_536;

            if !already_reported_too_many_items && items.len() >= MAX_ITEMS_PER_MODULE {
                already_reported_too_many_items = true;

                let bug = SyntaxBug::ModuleItemLimit(module_start_pos.clone());
                self.bugs.push(&bug);
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

    fn parse_import(&mut self) -> Import {
        assert!(self.lexer.peek_t() == Token::Import);
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();
        let path = self.parse_path();

        let alias = if self.lexer.skip_if(&Token::As) {
            let name = self.lexer.next_if_name().unwrap_or_else(|| {
                let bug = SyntaxBug::ImportMissingAliasName(self.lexer.peek_pos());
                self.bugs.push(&bug);
                "".into()
            });

            Some(intern_import_alias_name_id(name))
        } else {
            None
        };

        if !self.lexer.skip_if(&Token::Semi) {
            let bug = SyntaxBug::ExpectedSemicolon(self.lexer.peek_pos());
            self.bugs.push(&bug);
        }

        Import {
            visibility: None,
            attributes,
            path,
            alias,
        }
    }

    fn parse_type_alias(&mut self) -> TypeAlias {
        assert!(self.lexer.peek_t() == Token::Type);
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();

        let name = self.lexer.next_if_name().unwrap_or_else(|| {
            let bug = SyntaxBug::TypeAliasMissingName(self.lexer.peek_pos());
            self.bugs.push(&bug);
            "".into()
        });

        let name = intern_type_name_id(name);

        let type_params = self.parse_generic_parameters();

        let aliased_type = if self.lexer.skip_if(&Token::Eq) {
            Some(self.parse_type())
        } else {
            None
        };

        if !self.lexer.skip_if(&Token::Semi) {
            let bug = SyntaxBug::ExpectedSemicolon(self.lexer.peek_pos());
            self.bugs.push(&bug);
        }

        TypeAlias {
            visibility: None,
            attributes,
            name,
            type_params,
            aliased_type,
        }
    }

    fn parse_enum(&mut self) -> Enum {
        fn parse_enum_variant(this: &mut Parser) -> EnumVariant {
            let visibility = this.parse_visibility();
            let attributes = this.parse_attributes();

            let name = this.lexer.next_if_name().unwrap_or_else(|| {
                let bug = SyntaxBug::EnumMissingVariantName(this.lexer.peek_pos());
                this.bugs.push(&bug);
                "".into()
            });

            let name = intern_enum_variant_name_id(name);

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
            let bug = SyntaxBug::EnumMissingName(self.lexer.peek_pos());
            self.bugs.push(&bug);
            "".into()
        });

        let name = intern_type_name_id(name);

        let type_params = self.parse_generic_parameters();

        if !self.lexer.skip_if(&Token::OpenBrace) {
            let bug = SyntaxBug::ExpectedOpenBrace(self.lexer.peek_pos());
            self.bugs.push(&bug);
        }

        let mut variants = Vec::new();
        let mut already_reported_too_many_variants = false;

        while !self.lexer.skip_if(&Token::CloseBrace) {
            if self.lexer.is_eof() {
                let bug = SyntaxBug::EnumExpectedEnd(self.lexer.peek_pos());
                self.bugs.push(&bug);
                break;
            }

            const MAX_ENUM_VARIANTS: usize = 65_536;

            if !already_reported_too_many_variants && variants.len() >= MAX_ENUM_VARIANTS {
                already_reported_too_many_variants = true;

                let bug = SyntaxBug::EnumVariantLimit(self.lexer.peek_pos());
                self.bugs.push(&bug);
            }

            let variant = parse_enum_variant(self);
            variants.push(variant);

            if !self.lexer.skip_if(&Token::Comma) && !self.lexer.next_is(&Token::CloseBrace) {
                let bug = SyntaxBug::EnumExpectedEnd(self.lexer.peek_pos());
                self.bugs.push(&bug);
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
                let bug = SyntaxBug::StructureMissingFieldName(this.lexer.peek_pos());
                this.bugs.push(&bug);
                "".into()
            });

            let name = intern_struct_field_name_id(name);

            if !this.lexer.skip_if(&Token::Colon) {
                let bug = SyntaxBug::ExpectedColon(this.lexer.peek_pos());
                this.bugs.push(&bug);
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
                default,
            }
        }

        assert!(self.lexer.peek_t() == Token::Struct);
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();

        let name = self.lexer.next_if_name().unwrap_or_else(|| {
            let bug = SyntaxBug::StructureMissingName(self.lexer.peek_pos());
            self.bugs.push(&bug);
            "".into()
        });

        let name = intern_type_name_id(name);

        let type_params = self.parse_generic_parameters();

        if !self.lexer.skip_if(&Token::OpenBrace) {
            let bug = SyntaxBug::ExpectedOpenBrace(self.lexer.peek_pos());
            self.bugs.push(&bug);
        }

        let mut fields = Vec::new();
        let mut already_reported_too_many_fields = false;

        while !self.lexer.skip_if(&Token::CloseBrace) {
            if self.lexer.is_eof() {
                let bug = SyntaxBug::StructureExpectedEnd(self.lexer.peek_pos());
                self.bugs.push(&bug);
                break;
            }

            const MAX_STRUCT_FIELDS: usize = 65_536;

            if !already_reported_too_many_fields && fields.len() >= MAX_STRUCT_FIELDS {
                already_reported_too_many_fields = true;

                let bug = SyntaxBug::StructureFieldLimit(self.lexer.peek_pos());
                self.bugs.push(&bug);
            }

            let field = parse_struct_field(self);
            fields.push(field);

            if !self.lexer.skip_if(&Token::Comma) && !self.lexer.next_is(&Token::CloseBrace) {
                let bug = SyntaxBug::StructureExpectedEnd(self.lexer.peek_pos());
                self.bugs.push(&bug);
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
                AssociatedItem::Method(func)
            }

            Token::Const => {
                let mut const_var = self.parse_variable();
                const_var.visibility = visibility;
                AssociatedItem::ConstantItem(const_var)
            }

            Token::Type => {
                let mut type_alias = self.parse_type_alias();
                type_alias.visibility = visibility;
                AssociatedItem::TypeAlias(type_alias)
            }

            _ => {
                self.lexer.skip_tok();

                let bug = SyntaxBug::TraitDoesNotAllowItem(self.lexer.peek_pos());
                self.bugs.push(&bug);

                AssociatedItem::SyntaxError
            }
        }
    }

    fn parse_trait(&mut self) -> Trait {
        assert!(self.lexer.peek_t() == Token::Trait);
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();

        let name = self.lexer.next_if_name().unwrap_or_else(|| {
            let bug = SyntaxBug::TraitMissingName(self.lexer.peek_pos());
            self.bugs.push(&bug);
            "".into()
        });

        let name = intern_trait_name_id(name);

        let type_params = self.parse_generic_parameters();

        if !self.lexer.skip_if(&Token::OpenBrace) {
            let bug = SyntaxBug::ExpectedOpenBrace(self.lexer.peek_pos());
            self.bugs.push(&bug);
        }

        let mut items = Vec::new();
        let mut already_reported_too_many_items = false;

        while !self.lexer.skip_if(&Token::CloseBrace) {
            if self.lexer.is_eof() {
                let bug = SyntaxBug::TraitExpectedEnd(self.lexer.peek_pos());
                self.bugs.push(&bug);
                break;
            }

            const MAX_TRAIT_ITEMS: usize = 65_536;

            if !already_reported_too_many_items && items.len() >= MAX_TRAIT_ITEMS {
                already_reported_too_many_items = true;

                let bug = SyntaxBug::TraitItemLimit(self.lexer.peek_pos());
                self.bugs.push(&bug);
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

        let type_params = self.parse_generic_parameters();

        let trait_path = if self.lexer.skip_if(&Token::Trait) {
            let path = self.parse_type_path();

            if !self.lexer.skip_if(&Token::For) {
                let bug = SyntaxBug::ImplMissingFor(self.lexer.peek_pos());
                self.bugs.push(&bug);
            }

            Some(path)
        } else {
            None
        };

        let for_type = self.parse_type();

        if !self.lexer.skip_if(&Token::OpenBrace) {
            let bug = SyntaxBug::ExpectedOpenBrace(self.lexer.peek_pos());
            self.bugs.push(&bug);
        }

        let mut items = Vec::new();
        let mut already_reported_too_many_items = false;

        while !self.lexer.skip_if(&Token::CloseBrace) {
            if self.lexer.is_eof() {
                let bug = SyntaxBug::ImplExpectedEnd(self.lexer.peek_pos());
                self.bugs.push(&bug);
                break;
            }

            const MAX_IMPL_ITEMS: usize = 65_536;

            if !already_reported_too_many_items && items.len() >= MAX_IMPL_ITEMS {
                already_reported_too_many_items = true;

                let bug = SyntaxBug::ImplItemLimit(self.lexer.peek_pos());
                self.bugs.push(&bug);
            }

            let item = self.parse_associated_item();
            items.push(item);
        }

        Impl {
            visibility: None,
            attributes,
            type_params,
            trait_path,
            for_type,
            items,
        }
    }

    fn parse_named_function_parameters(&mut self) -> Vec<FunctionParameter> {
        fn parse_named_function_parameter(this: &mut Parser) -> FunctionParameter {
            let attributes = this.parse_attributes();

            let mut mutability = None;
            if this.lexer.skip_if(&Token::Mut) {
                mutability = Some(Mutability::Mutable);
            } else if this.lexer.skip_if(&Token::Const) {
                mutability = Some(Mutability::Const);
            }

            let name = this.lexer.next_if_name().unwrap_or_else(|| {
                let bug = SyntaxBug::FunctionParameterMissingName(this.lexer.peek_pos());
                this.bugs.push(&bug);
                "".into()
            });

            let name = intern_parameter_name_id(name);

            let param_type = if this.lexer.skip_if(&Token::Colon) {
                Some(this.parse_type())
            } else {
                None
            };

            let default = if this.lexer.skip_if(&Token::Eq) {
                Some(this.parse_expression())
            } else {
                None
            };

            FunctionParameter {
                attributes,
                mutability,
                name,
                param_type,
                default,
            }
        }

        let mut params = Vec::new();

        if !self.lexer.skip_if(&Token::OpenParen) {
            let bug = SyntaxBug::ExpectedOpenParen(self.lexer.peek_pos());
            self.bugs.push(&bug);
        }

        self.lexer.skip_if(&Token::Comma);

        let mut already_reported_too_many_parameters = false;

        while !self.lexer.skip_if(&Token::CloseParen) {
            if self.lexer.is_eof() {
                let bug = SyntaxBug::FunctionParametersExpectedEnd(self.lexer.peek_pos());
                self.bugs.push(&bug);
                break;
            }

            const MAX_FUNCTION_PARAMETERS: usize = 65_536;

            if !already_reported_too_many_parameters && params.len() >= MAX_FUNCTION_PARAMETERS {
                already_reported_too_many_parameters = true;

                let bug = SyntaxBug::FunctionParameterLimit(self.lexer.peek_pos());
                self.bugs.push(&bug);
            }

            let param = parse_named_function_parameter(self);
            params.push(param);

            if !self.lexer.skip_if(&Token::Comma) && !self.lexer.next_is(&Token::CloseParen) {
                let bug = SyntaxBug::FunctionParametersExpectedEnd(self.lexer.peek_pos());
                self.bugs.push(&bug);
                self.lexer.skip_while(&Token::CloseParen);
                break;
            }
        }

        params
    }

    fn parse_named_function(&mut self) -> NamedFunction {
        assert!(self.lexer.peek_t() == Token::Fn);
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();

        let name = self.lexer.next_if_name().unwrap_or_else(|| {
            let bug = SyntaxBug::FunctionMissingName(self.lexer.peek_pos());
            self.bugs.push(&bug);
            "".into()
        });

        let name = intern_function_name_id(name);

        let type_params = self.parse_generic_parameters();
        let parameters = self.parse_named_function_parameters();

        let return_type = if self.lexer.skip_if(&Token::Minus) {
            if !self.lexer.skip_if(&Token::Gt) {
                let bug = SyntaxBug::ExpectedArrow(self.lexer.peek_pos());
                self.bugs.push(&bug);
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

        NamedFunction {
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
            let bug = SyntaxBug::VariableMissingName(self.lexer.peek_pos());
            self.bugs.push(&bug);
            "".into()
        });

        let name = intern_variable_name_id(name);

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
            let bug = SyntaxBug::ExpectedSemicolon(self.lexer.peek_pos());
            self.bugs.push(&bug);
        }

        Variable {
            visibility: None,
            kind,
            attributes,
            mutability,
            name,
            var_type,
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

            Token::Import => {
                let mut import = self.parse_import();
                import.visibility = visibility;
                Item::Import(Box::new(import))
            }

            Token::Type => {
                let mut type_alias = self.parse_type_alias();
                type_alias.visibility = visibility;
                Item::TypeAlias(Box::new(type_alias))
            }

            Token::Struct => {
                let mut struct_def = self.parse_struct();
                struct_def.visibility = visibility;
                Item::Struct(Box::new(struct_def))
            }

            Token::Enum => {
                let mut enum_def = self.parse_enum();
                enum_def.visibility = visibility;
                Item::Enum(Box::new(enum_def))
            }

            Token::Trait => {
                let mut trait_def = self.parse_trait();
                trait_def.visibility = visibility;
                Item::Trait(Box::new(trait_def))
            }

            Token::Impl => {
                let mut impl_def = self.parse_implementation();
                impl_def.visibility = visibility;
                Item::Impl(Box::new(impl_def))
            }

            Token::Fn => {
                let mut func = self.parse_named_function();
                func.visibility = visibility;
                Item::NamedFunction(Box::new(func))
            }

            Token::Static | Token::Const | Token::Let | Token::Var => {
                let mut var = self.parse_variable();
                var.visibility = visibility;
                Item::Variable(Box::new(var))
            }

            _ => {
                self.lexer.skip_tok();

                let bug = SyntaxBug::ExpectedItem(item_pos_begin);
                self.bugs.push(&bug);

                Item::SyntaxError
            }
        }
    }
}
