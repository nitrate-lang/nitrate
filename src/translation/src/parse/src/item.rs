use super::parse::Parser;
use crate::bugs::SyntaxBug;

use nitrate_parsetree::kind::{
    AssociatedItem, Block, ConstVariable, Enum, EnumVariant, FunctionParameter, GenericParameter,
    Impl, Import, Item, Module, NamedFunction, StaticVariable, Struct, StructField, Trait,
    TypeAlias,
};
use nitrate_tokenize::{Keyword, Op, Punct, Token};

impl Parser<'_, '_> {
    fn parse_generic_parameters(&mut self) -> Vec<GenericParameter> {
        fn parse_generic_parameter(this: &mut Parser) -> GenericParameter {
            let name = this.lexer.next_if_name().unwrap_or_else(|| {
                let bug = SyntaxBug::GenericMissingParameterName(this.lexer.peek_pos());
                this.bugs.push(&bug);
                "".into()
            });

            let default = if this.lexer.skip_if(&Token::Op(Op::Set)) {
                Some(this.parse_type())
            } else {
                None
            };

            GenericParameter { name, default }
        }

        let mut parameters = Vec::new();

        if !self.lexer.skip_if(&Token::Op(Op::LogicLt)) {
            return parameters;
        }

        while !self.lexer.skip_if(&Token::Op(Op::LogicGt)) {
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

            if !self.lexer.skip_if(&Token::Punct(Punct::Comma))
                && !self.lexer.next_is(&Token::Op(Op::LogicGt))
            {
                let bug = SyntaxBug::GenericParameterExpectedEnd(self.lexer.peek_pos());
                self.bugs.push(&bug);

                self.lexer.skip_while(&Token::Op(Op::LogicGt));
                break;
            }
        }

        parameters
    }

    fn parse_module(&mut self) -> Module {
        let module_start_pos = self.lexer.peek_pos();

        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Mod));
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();

        let name = self.lexer.next_if_name().unwrap_or_else(|| {
            let bug = SyntaxBug::ModuleMissingName(self.lexer.peek_pos());
            self.bugs.push(&bug);
            "".into()
        });

        if !self.lexer.skip_if(&Token::Punct(Punct::LeftBrace)) {
            let bug = SyntaxBug::ExpectedOpeningBrace(self.lexer.peek_pos());
            self.bugs.push(&bug);
        }

        let mut items = Vec::new();
        let mut already_reported_too_many_items = false;

        while !self.lexer.skip_if(&Token::Punct(Punct::RightBrace)) {
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
            attributes,
            name,
            items,
        }
    }

    fn parse_import(&mut self) -> Import {
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Import));
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();
        let path = self.parse_path();

        let alias = if self.lexer.skip_if(&Token::Op(Op::As)) {
            Some(self.lexer.next_if_name().unwrap_or_else(|| {
                let bug = SyntaxBug::ImportMissingAliasName(self.lexer.peek_pos());
                self.bugs.push(&bug);
                "".into()
            }))
        } else {
            None
        };

        if !self.lexer.skip_if(&Token::Punct(Punct::Semicolon)) {
            let bug = SyntaxBug::ExpectedSemicolon(self.lexer.peek_pos());
            self.bugs.push(&bug);
        }

        Import {
            attributes,
            path,
            alias,
        }
    }

    fn parse_type_alias(&mut self) -> TypeAlias {
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Type));
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();

        let name = self.lexer.next_if_name().unwrap_or_else(|| {
            let bug = SyntaxBug::TypeAliasMissingName(self.lexer.peek_pos());
            self.bugs.push(&bug);
            "".into()
        });

        let type_params = self.parse_generic_parameters();

        let aliased_type = if self.lexer.skip_if(&Token::Op(Op::Set)) {
            Some(self.parse_type())
        } else {
            None
        };

        if !self.lexer.skip_if(&Token::Punct(Punct::Semicolon)) {
            let bug = SyntaxBug::ExpectedSemicolon(self.lexer.peek_pos());
            self.bugs.push(&bug);
        }

        TypeAlias {
            attributes,
            name,
            type_params,
            aliased_type,
        }
    }

    fn parse_enum_variant(&mut self) -> EnumVariant {
        let attributes = self.parse_attributes();

        let name = self.lexer.next_if_name().unwrap_or_else(|| {
            let bug = SyntaxBug::EnumMissingVariantName(self.lexer.peek_pos());
            self.bugs.push(&bug);
            "".into()
        });

        let variant_type = if self.lexer.next_is(&Token::Punct(Punct::LeftParen)) {
            Some(self.parse_type())
        } else {
            None
        };

        let value = if self.lexer.skip_if(&Token::Op(Op::Set)) {
            Some(self.parse_expression())
        } else {
            None
        };

        EnumVariant {
            attributes,
            name,
            variant_type,
            value,
        }
    }

    fn parse_enum(&mut self) -> Enum {
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Enum));
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();

        let name = self.lexer.next_if_name().unwrap_or_else(|| {
            let bug = SyntaxBug::EnumMissingName(self.lexer.peek_pos());
            self.bugs.push(&bug);
            "".into()
        });

        let type_params = self.parse_generic_parameters();

        if !self.lexer.skip_if(&Token::Punct(Punct::LeftBrace)) {
            let bug = SyntaxBug::ExpectedOpeningBrace(self.lexer.peek_pos());
            self.bugs.push(&bug);
        }

        let mut variants = Vec::new();
        let mut already_reported_too_many_variants = false;

        while !self.lexer.skip_if(&Token::Punct(Punct::RightBrace)) {
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

            let variant = self.parse_enum_variant();
            variants.push(variant);

            if !self.lexer.skip_if(&Token::Punct(Punct::Comma))
                && !self.lexer.next_is(&Token::Punct(Punct::RightBrace))
            {
                let bug = SyntaxBug::EnumExpectedEnd(self.lexer.peek_pos());
                self.bugs.push(&bug);
                self.lexer.skip_while(&Token::Punct(Punct::RightBrace));

                break;
            }
        }

        Enum {
            attributes,
            name,
            type_params,
            variants,
        }
    }

    fn parse_struct_field(&mut self) -> StructField {
        let attributes = self.parse_attributes();

        let name = self.lexer.next_if_name().unwrap_or_else(|| {
            let bug = SyntaxBug::StructureMissingFieldName(self.lexer.peek_pos());
            self.bugs.push(&bug);
            "".into()
        });

        if !self.lexer.skip_if(&Token::Punct(Punct::Colon)) {
            let bug = SyntaxBug::ExpectedColon(self.lexer.peek_pos());
            self.bugs.push(&bug);
        }

        let field_type = self.parse_type();

        let default = if self.lexer.skip_if(&Token::Op(Op::Set)) {
            Some(self.parse_expression())
        } else {
            None
        };

        StructField {
            attributes,
            name,
            field_type,
            default,
        }
    }

    fn parse_struct(&mut self) -> Struct {
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Struct));
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();

        let name = self.lexer.next_if_name().unwrap_or_else(|| {
            let bug = SyntaxBug::StructureMissingName(self.lexer.peek_pos());
            self.bugs.push(&bug);
            "".into()
        });

        let type_params = self.parse_generic_parameters();

        if !self.lexer.skip_if(&Token::Punct(Punct::LeftBrace)) {
            let bug = SyntaxBug::ExpectedOpeningBrace(self.lexer.peek_pos());
            self.bugs.push(&bug);
        }

        let mut fields = Vec::new();
        let mut already_reported_too_many_fields = false;

        while !self.lexer.skip_if(&Token::Punct(Punct::RightBrace)) {
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

            let field = self.parse_struct_field();
            fields.push(field);

            if !self.lexer.skip_if(&Token::Punct(Punct::Comma))
                && !self.lexer.next_is(&Token::Punct(Punct::RightBrace))
            {
                let bug = SyntaxBug::StructureExpectedEnd(self.lexer.peek_pos());
                self.bugs.push(&bug);
                self.lexer.skip_while(&Token::Punct(Punct::RightBrace));
                break;
            }
        }

        Struct {
            attributes,
            name,
            type_params,
            fields,
            methods: Vec::new(),
        }
    }

    fn parse_associated_item(&mut self) -> AssociatedItem {
        match self.lexer.peek_t() {
            Token::Keyword(Keyword::Fn) => {
                let func = self.parse_named_function();
                AssociatedItem::Method(func)
            }

            Token::Keyword(Keyword::Const) => {
                let const_var = self.parse_const_variable();
                AssociatedItem::ConstantItem(const_var)
            }

            Token::Keyword(Keyword::Type) => {
                let type_alias = self.parse_type_alias();
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
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Trait));
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();

        let name = self.lexer.next_if_name().unwrap_or_else(|| {
            let bug = SyntaxBug::TraitMissingName(self.lexer.peek_pos());
            self.bugs.push(&bug);
            "".into()
        });

        let type_params = self.parse_generic_parameters();

        if !self.lexer.skip_if(&Token::Punct(Punct::LeftBrace)) {
            let bug = SyntaxBug::ExpectedOpeningBrace(self.lexer.peek_pos());
            self.bugs.push(&bug);
        }

        let mut items = Vec::new();
        let mut already_reported_too_many_items = false;

        while !self.lexer.skip_if(&Token::Punct(Punct::RightBrace)) {
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
            attributes,
            name,
            type_params,
            items,
        }
    }

    fn parse_implementation(&mut self) -> Impl {
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Impl));
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();

        let type_params = self.parse_generic_parameters();

        let trait_path = if self.lexer.skip_if(&Token::Keyword(Keyword::Trait)) {
            let path = self.parse_path();

            if !self.lexer.skip_if(&Token::Keyword(Keyword::For)) {
                let bug = SyntaxBug::ImplMissingFor(self.lexer.peek_pos());
                self.bugs.push(&bug);
            }

            Some(path)
        } else {
            None
        };

        let for_type = self.parse_type();

        if !self.lexer.skip_if(&Token::Punct(Punct::LeftBrace)) {
            let bug = SyntaxBug::ExpectedOpeningBrace(self.lexer.peek_pos());
            self.bugs.push(&bug);
        }

        let mut items = Vec::new();
        let mut already_reported_too_many_items = false;

        while !self.lexer.skip_if(&Token::Punct(Punct::RightBrace)) {
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
            attributes,
            type_params,
            trait_path,
            for_type,
            items,
        }
    }

    fn parse_named_function_parameter(&mut self) -> FunctionParameter {
        let attributes = self.parse_attributes();

        let name = self.lexer.next_if_name().unwrap_or_else(|| {
            let bug = SyntaxBug::FunctionParameterMissingName(self.lexer.peek_pos());
            self.bugs.push(&bug);
            "".into()
        });

        let param_type = if self.lexer.skip_if(&Token::Punct(Punct::Colon)) {
            Some(self.parse_type())
        } else {
            None
        };

        let default = if self.lexer.skip_if(&Token::Op(Op::Set)) {
            Some(self.parse_expression())
        } else {
            None
        };

        FunctionParameter {
            name,
            param_type,
            default,
            attributes,
        }
    }

    fn parse_named_function_parameters(&mut self) -> Vec<FunctionParameter> {
        let mut params = Vec::new();

        if !self.lexer.skip_if(&Token::Punct(Punct::LeftParen)) {
            let bug = SyntaxBug::ExpectedOpeningParen(self.lexer.peek_pos());
            self.bugs.push(&bug);
        }

        self.lexer.skip_if(&Token::Punct(Punct::Comma));

        let mut already_reported_too_many_parameters = false;

        while !self.lexer.skip_if(&Token::Punct(Punct::RightParen)) {
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

            let param = self.parse_named_function_parameter();
            params.push(param);

            if !self.lexer.skip_if(&Token::Punct(Punct::Comma))
                && !self.lexer.next_is(&Token::Punct(Punct::RightParen))
            {
                let bug = SyntaxBug::FunctionParametersExpectedEnd(self.lexer.peek_pos());
                self.bugs.push(&bug);
                self.lexer.skip_while(&Token::Punct(Punct::RightParen));
                break;
            }
        }

        params
    }

    fn parse_named_function(&mut self) -> NamedFunction {
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Fn));
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();

        let name = self.lexer.next_if_name().unwrap_or_else(|| {
            let bug = SyntaxBug::FunctionMissingName(self.lexer.peek_pos());
            self.bugs.push(&bug);
            "".into()
        });

        let type_params = self.parse_generic_parameters();
        let parameters = self.parse_named_function_parameters();

        let return_type = if self.lexer.skip_if(&Token::Op(Op::Arrow)) {
            Some(self.parse_type())
        } else {
            None
        };

        let definition = if self.lexer.next_is(&Token::Punct(Punct::LeftBrace)) {
            let body = self.parse_block();
            Some(body)
        } else if self.lexer.skip_if(&Token::Op(Op::BlockArrow)) {
            let single = self.parse_expression();

            if !self.lexer.skip_if(&Token::Punct(Punct::Semicolon)) {
                let bug = SyntaxBug::ExpectedSemicolon(self.lexer.peek_pos());
                self.bugs.push(&bug);
            }

            Some(Block {
                elements: vec![single],
                ends_with_semi: false,
            })
        } else if self.lexer.skip_if(&Token::Punct(Punct::Semicolon)) {
            None
        } else {
            let bug = SyntaxBug::FunctionExpectedBody(self.lexer.peek_pos());
            self.bugs.push(&bug);
            self.lexer.skip_while(&Token::Punct(Punct::RightBrace));
            None
        };

        NamedFunction {
            attributes,
            name,
            type_params,
            parameters,
            return_type,
            definition,
        }
    }

    fn parse_static_variable(&mut self) -> StaticVariable {
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Static));
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();

        let is_mutable = self.lexer.skip_if(&Token::Keyword(Keyword::Mut));
        if !is_mutable {
            self.lexer.skip_if(&Token::Keyword(Keyword::Const));
        }

        let name = self.lexer.next_if_name().unwrap_or_else(|| {
            let bug = SyntaxBug::StaticVariableMissingName(self.lexer.peek_pos());
            self.bugs.push(&bug);
            "".into()
        });

        let var_type = if self.lexer.skip_if(&Token::Punct(Punct::Colon)) {
            Some(self.parse_type())
        } else {
            None
        };

        let initializer = if self.lexer.skip_if(&Token::Op(Op::Set)) {
            Some(self.parse_expression())
        } else {
            None
        };

        if !self.lexer.skip_if(&Token::Punct(Punct::Semicolon)) {
            let bug = SyntaxBug::ExpectedSemicolon(self.lexer.peek_pos());
            self.bugs.push(&bug);
        }

        StaticVariable {
            attributes,
            is_mutable,
            name,
            var_type,
            initializer,
        }
    }

    fn parse_const_variable(&mut self) -> ConstVariable {
        assert!(self.lexer.peek_t() == Token::Keyword(Keyword::Const));
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();

        let name = self.lexer.next_if_name().unwrap_or_else(|| {
            let bug = SyntaxBug::ConstVariableMissingName(self.lexer.peek_pos());
            self.bugs.push(&bug);
            "".into()
        });

        let var_type = if self.lexer.skip_if(&Token::Punct(Punct::Colon)) {
            Some(self.parse_type())
        } else {
            None
        };

        let initializer = if self.lexer.skip_if(&Token::Op(Op::Set)) {
            Some(self.parse_expression())
        } else {
            None
        };

        if !self.lexer.skip_if(&Token::Punct(Punct::Semicolon)) {
            let bug = SyntaxBug::ExpectedSemicolon(self.lexer.peek_pos());
            self.bugs.push(&bug);
        }

        ConstVariable {
            attributes,
            name,
            var_type,
            initializer,
        }
    }

    pub(crate) fn parse_item(&mut self) -> Item {
        let item_pos_begin = self.lexer.peek_pos();

        match self.lexer.peek_t() {
            Token::Keyword(Keyword::Mod) => {
                let module = self.parse_module();
                Item::Module(Box::new(module))
            }

            Token::Keyword(Keyword::Import) => {
                let import = self.parse_import();
                Item::Import(Box::new(import))
            }

            Token::Keyword(Keyword::Type) => {
                let type_alias = self.parse_type_alias();
                Item::TypeAlias(Box::new(type_alias))
            }

            Token::Keyword(Keyword::Struct) => {
                let struct_def = self.parse_struct();
                Item::Struct(Box::new(struct_def))
            }

            Token::Keyword(Keyword::Enum) => {
                let enum_def = self.parse_enum();
                Item::Enum(Box::new(enum_def))
            }

            Token::Keyword(Keyword::Trait) => {
                let trait_def = self.parse_trait();
                Item::Trait(Box::new(trait_def))
            }

            Token::Keyword(Keyword::Impl) => {
                let impl_def = self.parse_implementation();
                Item::Impl(Box::new(impl_def))
            }

            Token::Keyword(Keyword::Fn) => {
                let func = self.parse_named_function();
                Item::NamedFunction(Box::new(func))
            }

            Token::Keyword(Keyword::Static) => {
                let static_var = self.parse_static_variable();
                Item::StaticVariable(Box::new(static_var))
            }

            Token::Keyword(Keyword::Const) => {
                let const_var = self.parse_const_variable();
                Item::ConstVariable(Box::new(const_var))
            }

            Token::Name(_)
            | Token::Integer(_)
            | Token::Float(_)
            | Token::String(_)
            | Token::BString(_)
            | Token::Keyword(_)
            | Token::Op(_)
            | Token::Punct(_)
            | Token::Comment(_)
            | Token::Eof => {
                self.lexer.skip_tok();

                let bug = SyntaxBug::ExpectedItem(item_pos_begin);
                self.bugs.push(&bug);

                Item::SyntaxError
            }
        }
    }
}
