use super::parse::Parser;
use crate::diagnosis::SyntaxErr;

use nitrate_nstring::NString;
use nitrate_token::Token;
use nitrate_tree::ast::{
    AssociatedItem, Enum, EnumVariant, FuncParam, FuncParams, Function, Generics, GlobalVariable, GlobalVariableKind,
    Impl, Import, Item, ItemPath, ItemPathSegment, ItemSyntaxError, Module, Struct, StructField, Trait, TypeAlias,
    TypeParam, UseTree,
};

impl Parser<'_, '_> {
    fn parse_generics(&mut self) -> Option<Generics> {
        fn parse_generic_parameter(this: &mut Parser) -> TypeParam {
            let pos = this.lexer.peek_pos();
            let name = this.parse_name(SyntaxErr::GenericMissingParameterName(pos));

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

        let eof = SyntaxErr::GenericParameterExpectedEnd(self.lexer.peek_pos());
        let limit = SyntaxErr::GenericParameterLimit(self.lexer.peek_pos());
        let end = SyntaxErr::GenericParameterExpectedEnd(self.lexer.peek_pos());

        let params =
            self.parse_comma_separated_list(&Token::Gt, 65_536, false, eof, limit, end, parse_generic_parameter);

        Some(Generics { params })
    }

    fn parse_module(&mut self) -> Module {
        let module_start_pos = self.lexer.peek_pos();

        assert!(self.lexer.peek_tok().token == Token::Mod);
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();

        let name = self
            .lexer
            .next_if_name()
            .unwrap_or_else(|| {
                let bug = SyntaxErr::ModuleMissingName(self.lexer.peek_pos());
                self.log.report(&bug);
                String::default()
            })
            .into();

        self.expect_open_brace();

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
        let mut segments = Vec::new();

        if !self.lexer.next_is(&Token::Colon) {
            let segment = self.lexer.next_if_name().unwrap_or_else(|| {
                let bug = SyntaxErr::PathExpectedName(self.lexer.peek_pos());
                self.log.report(&bug);
                "".into()
            });

            segments.push(ItemPathSegment { segment });
        }

        while !self.lexer.is_eof() {
            let rewind_pos = self.lexer.peek_pos();

            if !self.parse_double_colon() {
                if segments.is_empty() {
                    let bug = SyntaxErr::PathExpectedName(self.lexer.peek_pos());
                    self.log.report(&bug);
                }
                break;
            }

            let Some(segment) = self.lexer.next_if_name() else {
                self.lexer.rewind(rewind_pos);
                break;
            };

            segments.push(ItemPathSegment { segment });
        }

        ItemPath { segments }
    }

    fn parse_use(&mut self) -> Import {
        fn parse_use_tree(this: &mut Parser) -> UseTree {
            let path = this.parse_item_path();

            if this.parse_double_colon() {
                if this.lexer.skip_if(&Token::Star) {
                    return UseTree::UseAll { path };
                } else if this.lexer.skip_if(&Token::OpenBrace) {
                    let mut group = Vec::new();

                    while !this.lexer.is_eof() {
                        if this.lexer.skip_if(&Token::CloseBrace) {
                            break;
                        }

                        let subtree = parse_use_tree(this);
                        group.push(subtree);

                        if !this.lexer.skip_if(&Token::Comma) && !this.lexer.next_is(&Token::CloseBrace) {
                            let bug = SyntaxErr::ImportGroupExpectedEnd(this.lexer.peek_pos());
                            this.log.report(&bug);
                            this.lexer.skip_while(&Token::CloseBrace);
                            break;
                        }
                    }

                    UseTree::Group { path, group }
                } else {
                    let bug = SyntaxErr::ImportExpectedStarOrGroup(this.lexer.peek_pos());
                    this.log.report(&bug);
                    UseTree::Single { path }
                }
            } else if this.lexer.skip_if(&Token::As) {
                let alias = this.lexer.next_if_name().unwrap_or_else(|| {
                    let bug = SyntaxErr::ImportAliasMissingName(this.lexer.peek_pos());
                    this.log.report(&bug);
                    "".into()
                });

                UseTree::Alias {
                    path,
                    alias: NString::from(alias),
                }
            } else {
                UseTree::Single { path }
            }
        }

        assert!(self.lexer.peek_tok().token == Token::Use);
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();
        let use_tree = parse_use_tree(self);

        self.expect_semicolon();

        Import {
            visibility: None,
            attributes,
            use_tree,
            resolved: None,
        }
    }

    fn parse_type_alias(&mut self) -> TypeAlias {
        assert!(self.lexer.peek_tok().token == Token::Type);
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();

        let name = self.lexer.next_if_name().unwrap_or_else(|| {
            let bug = SyntaxErr::TypeAliasMissingName(self.lexer.peek_pos());
            self.log.report(&bug);
            "".into()
        });

        let name = NString::from(name);

        let generics = self.parse_generics();

        let aliased_type = if self.lexer.skip_if(&Token::Eq) {
            Some(self.parse_type())
        } else {
            None
        };

        self.expect_semicolon();

        TypeAlias {
            visibility: None,
            attributes,
            name,
            generics,
            alias_type: aliased_type,
        }
    }

    fn parse_enum(&mut self) -> Enum {
        fn parse_enum_variant(this: &mut Parser) -> EnumVariant {
            let attributes = this.parse_attributes();

            let name = this.lexer.next_if_name().unwrap_or_else(|| {
                let bug = SyntaxErr::EnumMissingVariantName(this.lexer.peek_pos());
                this.log.report(&bug);
                "".into()
            });

            let name = NString::from(name);

            let variant_type = if this.lexer.skip_if(&Token::OpenParen) {
                let ty = this.parse_type();

                this.expect_close_paren();

                Some(ty)
            } else {
                None
            };

            let value = if this.lexer.skip_if(&Token::Eq) {
                Some(this.parse_expression())
            } else {
                None
            };

            EnumVariant {
                attributes,
                name,
                ty: variant_type,
                default_value: value,
            }
        }

        assert!(self.lexer.peek_tok().token == Token::Enum);
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();

        let name = self.lexer.next_if_name().unwrap_or_else(|| {
            let bug = SyntaxErr::EnumMissingName(self.lexer.peek_pos());
            self.log.report(&bug);
            "".into()
        });

        let name = NString::from(name);

        let generics = self.parse_generics();

        self.expect_open_brace();

        let eof = SyntaxErr::EnumExpectedEnd(self.lexer.peek_pos());
        let limit = SyntaxErr::EnumVariantLimit(self.lexer.peek_pos());
        let end = SyntaxErr::EnumExpectedEnd(self.lexer.peek_pos());

        let variants =
            self.parse_comma_separated_list(&Token::CloseBrace, 65_536, true, eof, limit, end, parse_enum_variant);

        Enum {
            visibility: None,
            attributes,
            name,
            generics,
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

            let name = NString::from(name);

            this.expect_colon();

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
                ty: field_type,
                default_value: default,
            }
        }

        assert!(self.lexer.peek_tok().token == Token::Struct);
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();

        let name = self.lexer.next_if_name().unwrap_or_else(|| {
            let bug = SyntaxErr::StructureMissingName(self.lexer.peek_pos());
            self.log.report(&bug);
            "".into()
        });

        let name = NString::from(name);

        let generics = self.parse_generics();

        self.expect_open_brace();

        let eof = SyntaxErr::StructureExpectedEnd(self.lexer.peek_pos());
        let limit = SyntaxErr::StructureFieldLimit(self.lexer.peek_pos());
        let end = SyntaxErr::StructureExpectedEnd(self.lexer.peek_pos());

        let fields =
            self.parse_comma_separated_list(&Token::CloseBrace, 65_536, true, eof, limit, end, parse_struct_field);

        Struct {
            visibility: None,
            attributes,
            name,
            generics,
            fields,
        }
    }

    fn parse_associated_item(&mut self) -> AssociatedItem {
        let visibility = self.parse_visibility();

        match self.lexer.peek_tok().token {
            Token::Fn => {
                let mut func = self.parse_named_function();
                func.visibility = visibility;
                AssociatedItem::Method(func)
            }

            Token::Const => {
                let mut const_var = self.parse_global_variable();
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

                let bug = SyntaxErr::TraitDoesNotAllowItem(self.lexer.peek_pos());
                self.log.report(&bug);

                AssociatedItem::SyntaxError(ItemSyntaxError)
            }
        }
    }

    fn parse_trait(&mut self) -> Trait {
        assert!(self.lexer.peek_tok().token == Token::Trait);
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();

        let name = self.lexer.next_if_name().unwrap_or_else(|| {
            let bug = SyntaxErr::TraitMissingName(self.lexer.peek_pos());
            self.log.report(&bug);
            "".into()
        });

        let name = NString::from(name);

        let generics = self.parse_generics();

        self.expect_open_brace();

        let eof = SyntaxErr::TraitExpectedEnd(self.lexer.peek_pos());
        let limit = SyntaxErr::TraitItemLimit(self.lexer.peek_pos());
        let end = SyntaxErr::TraitExpectedEnd(self.lexer.peek_pos());

        let items = self.parse_comma_separated_list(&Token::CloseBrace, 65_536, true, eof, limit, end, |this| {
            this.parse_associated_item()
        });

        Trait {
            visibility: None,
            attributes,
            name,
            generics,
            items,
        }
    }

    fn parse_implementation(&mut self) -> Impl {
        assert!(self.lexer.peek_tok().token == Token::Impl);
        self.lexer.skip_tok();

        let generics = self.parse_generics();

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

        self.expect_open_brace();

        let eof = SyntaxErr::ImplExpectedEnd(self.lexer.peek_pos());
        let limit = SyntaxErr::ImplItemLimit(self.lexer.peek_pos());
        let end = SyntaxErr::ImplExpectedEnd(self.lexer.peek_pos());

        let items = self.parse_comma_separated_list(&Token::CloseBrace, 65_536, true, eof, limit, end, |this| {
            this.parse_associated_item()
        });

        Impl {
            generics,
            trait_path,
            for_type,
            items,
        }
    }

    fn parse_function_parameters(&mut self) -> FuncParams {
        fn parse_function_parameter(this: &mut Parser) -> FuncParam {
            let attributes = this.parse_attributes();

            let mutability = this.parse_mutability();

            let name = this.lexer.next_if_name().unwrap_or_else(|| {
                let bug = SyntaxErr::FunctionParameterMissingName(this.lexer.peek_pos());
                this.log.report(&bug);
                "".into()
            });

            let name = NString::from(name);

            this.expect_colon();

            let ty = this.parse_type();

            let default_value = if this.lexer.skip_if(&Token::Eq) {
                Some(this.parse_expression())
            } else {
                None
            };

            FuncParam {
                attributes,
                mutability,
                name,
                ty,
                default_value,
            }
        }

        self.expect_open_paren();

        let mut params = Vec::new();

        let eof = SyntaxErr::FunctionParametersExpectedEnd(self.lexer.peek_pos());
        let limit = SyntaxErr::FunctionParameterLimit(self.lexer.peek_pos());
        let end = SyntaxErr::FunctionParametersExpectedEnd(self.lexer.peek_pos());

        // Skip leading comma
        self.lexer.skip_if(&Token::Comma);

        while !self.lexer.skip_if(&Token::CloseParen) {
            if self.lexer.is_eof() {
                self.log.report(&eof);
                break;
            }

            Self::check_limit(params.len(), 65_536, &mut false, &limit, self.log);

            if self.lexer.skip_if(&Token::Dot) {
                if !self.lexer.skip_if(&Token::Dot) || !self.lexer.skip_if(&Token::Dot) {
                    self.log
                        .report(&SyntaxErr::FunctionParameterVariadicExpected(self.lexer.peek_pos()));
                }

                if !self.lexer.skip_if(&Token::CloseParen) {
                    self.log.report(&end);
                    self.lexer.skip_while(&Token::CloseParen);
                }

                return FuncParams { params, variadic: true };
            }

            let param = parse_function_parameter(self);
            params.push(param);

            if !self.lexer.skip_if(&Token::Comma) && !self.lexer.next_is(&Token::CloseParen) {
                self.log.report(&end);
                self.lexer.skip_while(&Token::CloseParen);
                break;
            }
        }

        FuncParams {
            params,
            variadic: false,
        }
    }

    fn parse_named_function(&mut self) -> Function {
        assert!(self.lexer.peek_tok().token == Token::Fn);
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();

        let name = self.lexer.next_if_name().unwrap_or_else(|| {
            let bug = SyntaxErr::FunctionMissingName(self.lexer.peek_pos());
            self.log.report(&bug);
            "".into()
        });

        let name = NString::from(name);

        let generics = self.parse_generics();
        let parameters = self.parse_function_parameters();

        let return_type = self.parse_return_type_arrow();

        let definition = if self.lexer.skip_if(&Token::Semi) {
            None
        } else {
            Some(self.parse_block())
        };

        Function {
            visibility: None,
            attributes,
            name,
            generics,
            parameters,
            return_type,
            definition,
        }
    }

    fn parse_global_variable(&mut self) -> GlobalVariable {
        let kind = match self.lexer.next_tok().token {
            Token::Static => GlobalVariableKind::Static,
            Token::Const => GlobalVariableKind::Const,
            _ => unreachable!(),
        };

        let attributes = self.parse_attributes();

        let mutability = self.parse_mutability();

        let name = self.lexer.next_if_name().unwrap_or_else(|| {
            let bug = SyntaxErr::VariableMissingName(self.lexer.peek_pos());
            self.log.report(&bug);
            "".into()
        });

        let name = NString::from(name);

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

        self.expect_semicolon();

        GlobalVariable {
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

        match self.lexer.peek_tok().token {
            Token::Mod => {
                let mut module = self.parse_module();
                module.visibility = visibility;
                Item::Module(Box::new(module))
            }

            Token::Use => {
                let mut import = self.parse_use();
                import.visibility = visibility;
                Item::Import(Box::new(import))
            }

            Token::Type => {
                let mut type_alias = self.parse_type_alias();
                type_alias.visibility = visibility;
                Item::TypeAlias(type_alias)
            }

            Token::Struct => {
                let mut struct_def = self.parse_struct();
                struct_def.visibility = visibility;
                Item::Struct(struct_def)
            }

            Token::Enum => {
                let mut enum_def = self.parse_enum();
                enum_def.visibility = visibility;
                Item::Enum(enum_def)
            }

            Token::Trait => {
                let mut trait_def = self.parse_trait();
                trait_def.visibility = visibility;
                Item::Trait(trait_def)
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
                Item::Function(func)
            }

            Token::Static | Token::Const => {
                let mut var = self.parse_global_variable();
                var.visibility = visibility;
                Item::Variable(var)
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
