use super::parse::Parser;
use crate::diagnosis::SyntaxErr;
use crate::helper::MAX_LIMIT;
use nitrate_tree::ByteSpan;

use nitrate_nstring::NString;
use nitrate_token::Token;
use nitrate_tree::ast::{
    AssociatedItem, Enum, EnumVariant, ExternAbi, FuncParam, FuncParams, Function, Generics, GlobalVariable,
    GlobalVariableKind, Impl, Import, Item, ItemPath, ItemPathSegment, ItemSyntaxError, Module, Mutability,
    ReferenceType, Struct, StructField, Trait, Type, TypeAlias, TypeParam, TypePath, TypePathSegment, UseTree,
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
                span: ByteSpan::default(),
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
            self.parse_comma_separated_list(&Token::Gt, MAX_LIMIT, false, eof, limit, end, parse_generic_parameter);

        Some(Generics {
            span: ByteSpan::default(),
            params,
        })
    }

    fn parse_module(&mut self) -> Module {
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

            if !already_reported_too_many_items && items.len() >= MAX_LIMIT {
                already_reported_too_many_items = true;

                let bug = SyntaxErr::ModuleItemLimit(self.lexer.peek_pos());
                self.log.report(&bug);
            }

            let item = self.parse_item();
            items.push(item);
        }

        Module {
            span: ByteSpan::default(),
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

            segments.push(ItemPathSegment {
                span: ByteSpan::default(),
                segment,
            });
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

            segments.push(ItemPathSegment {
                span: ByteSpan::default(),
                segment,
            });
        }

        ItemPath {
            span: ByteSpan::default(),
            segments,
        }
    }

    fn parse_use(&mut self) -> Import {
        fn parse_use_tree(this: &mut Parser) -> UseTree {
            let path = this.parse_item_path();

            if this.parse_double_colon() {
                if this.lexer.skip_if(&Token::Star) {
                    UseTree::UseAll {
                        span: ByteSpan::default(),
                        path,
                    }
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

                    UseTree::Group {
                        span: ByteSpan::default(),
                        path,
                        group,
                    }
                } else {
                    let bug = SyntaxErr::ImportExpectedStarOrGroup(this.lexer.peek_pos());
                    this.log.report(&bug);
                    UseTree::Single {
                        span: ByteSpan::default(),
                        path,
                    }
                }
            } else if this.lexer.skip_if(&Token::As) {
                let alias = this.lexer.next_if_name().unwrap_or_else(|| {
                    let bug = SyntaxErr::ImportAliasMissingName(this.lexer.peek_pos());
                    this.log.report(&bug);
                    "".into()
                });

                UseTree::Alias {
                    span: ByteSpan::default(),
                    path,
                    alias: NString::from(alias),
                }
            } else {
                UseTree::Single {
                    span: ByteSpan::default(),
                    path,
                }
            }
        }

        assert!(self.lexer.peek_tok().token == Token::Use);
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();
        let use_tree = parse_use_tree(self);

        self.expect_semicolon();

        Import {
            span: ByteSpan::default(),
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
            span: ByteSpan::default(),
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
                span: ByteSpan::default(),
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
            self.parse_comma_separated_list(&Token::CloseBrace, MAX_LIMIT, true, eof, limit, end, parse_enum_variant);

        Enum {
            span: ByteSpan::default(),
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
                span: ByteSpan::default(),
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
            self.parse_comma_separated_list(&Token::CloseBrace, MAX_LIMIT, true, eof, limit, end, parse_struct_field);

        Struct {
            span: ByteSpan::default(),
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

                AssociatedItem::SyntaxError(ItemSyntaxError {
                    span: ByteSpan::default(),
                })
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

        let mut items = Vec::new();
        let mut already_reported_too_many_items = false;

        while !self.lexer.skip_if(&Token::CloseBrace) {
            if self.lexer.is_eof() {
                let bug = SyntaxErr::TraitExpectedEnd(self.lexer.peek_pos());
                self.log.report(&bug);
                break;
            }

            if !already_reported_too_many_items && items.len() >= MAX_LIMIT {
                already_reported_too_many_items = true;
                let bug = SyntaxErr::TraitItemLimit(self.lexer.peek_pos());
                self.log.report(&bug);
            }

            items.push(self.parse_associated_item());
        }

        Trait {
            span: ByteSpan::default(),
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

        let mut items = Vec::new();
        let mut already_reported_too_many_items = false;

        while !self.lexer.skip_if(&Token::CloseBrace) {
            if self.lexer.is_eof() {
                let bug = SyntaxErr::ImplExpectedEnd(self.lexer.peek_pos());
                self.log.report(&bug);
                break;
            }

            if !already_reported_too_many_items && items.len() >= MAX_LIMIT {
                already_reported_too_many_items = true;
                let bug = SyntaxErr::ImplItemLimit(self.lexer.peek_pos());
                self.log.report(&bug);
            }

            items.push(self.parse_associated_item());
        }

        Impl {
            span: ByteSpan::default(),
            generics,
            trait_path,
            for_type,
            items,
        }
    }

    fn parse_self_parameter(&mut self) -> Option<FuncParam> {
        // Check for self parameter syntax: self, &self, &mut self, mut self
        if !self.lexer.next_is(&Token::SelfKeyword) && !self.lexer.next_is(&Token::And) {
            return None;
        }

        // Try to parse &self or &mut self
        let rewind_pos = self.lexer.current_pos();
        let is_ref = self.lexer.skip_if(&Token::And);

        if is_ref {
            // If we saw &, the next token must be self or mut self
            if self.lexer.next_is(&Token::SelfKeyword) {
                // &self - create a reference type for the parameter
                self.lexer.skip_tok(); // consume self
                let name = NString::from("self");
                let ty = Type::ReferenceType(Box::new(ReferenceType {
                    span: ByteSpan::default(),
                    lifetime: None,
                    exclusivity: None,
                    mutability: None,
                    to: Type::TypePath(Box::new(TypePath {
                        span: ByteSpan::default(),
                        segments: vec![TypePathSegment {
                            span: ByteSpan::default(),
                            name: "Self".to_string(),
                            type_arguments: None,
                        }],
                        resolved_path: None,
                    })),
                }));
                return Some(FuncParam {
                    span: ByteSpan::default(),
                    attributes: None,
                    mutability: None,
                    name,
                    ty,
                    default_value: None,
                });
            } else if self.lexer.skip_if(&Token::Mut) && self.lexer.next_is(&Token::SelfKeyword) {
                // &mut self
                self.lexer.skip_tok(); // consume self
                let name = NString::from("self");
                let ty = Type::ReferenceType(Box::new(ReferenceType {
                    span: ByteSpan::default(),
                    lifetime: None,
                    exclusivity: None,
                    mutability: Some(Mutability::Mut),
                    to: Type::TypePath(Box::new(TypePath {
                        span: ByteSpan::default(),
                        segments: vec![TypePathSegment {
                            span: ByteSpan::default(),
                            name: "Self".to_string(),
                            type_arguments: None,
                        }],
                        resolved_path: None,
                    })),
                }));
                return Some(FuncParam {
                    span: ByteSpan::default(),
                    attributes: None,
                    mutability: None,
                    name,
                    ty,
                    default_value: None,
                });
            }

            // Not a valid self parameter, rewind
            self.lexer.rewind(rewind_pos);
            return None;
        }

        // self or mut self (without &)
        if self.lexer.next_is(&Token::SelfKeyword) {
            self.lexer.skip_tok(); // consume self
            let name = NString::from("self");
            let ty = Type::TypePath(Box::new(TypePath {
                span: ByteSpan::default(),
                segments: vec![TypePathSegment {
                    span: ByteSpan::default(),
                    name: "Self".to_string(),
                    type_arguments: None,
                }],
                resolved_path: None,
            }));
            return Some(FuncParam {
                span: ByteSpan::default(),
                attributes: None,
                mutability: None,
                name,
                ty,
                default_value: None,
            });
        }

        None
    }

    fn parse_function_parameters(&mut self) -> FuncParams {
        self.expect_open_paren();

        let mut params = Vec::new();

        let eof = SyntaxErr::FunctionParametersExpectedEnd(self.lexer.peek_pos());
        let limit = SyntaxErr::FunctionParameterLimit(self.lexer.peek_pos());
        let end = SyntaxErr::FunctionParametersExpectedEnd(self.lexer.peek_pos());

        // Skip leading comma
        self.lexer.skip_if(&Token::Comma);

        // Try to parse self parameter first
        if let Some(self_param) = self.parse_self_parameter() {
            params.push(self_param);

            // After self parameter, expect comma or close paren
            if !self.lexer.skip_if(&Token::Comma) && !self.lexer.next_is(&Token::CloseParen) {
                // If we have a self parameter without trailing comma and not at close paren,
                // it might be followed by more params - report error and skip
                if !self.lexer.next_is(&Token::CloseParen) {
                    self.log
                        .report(&SyntaxErr::FunctionParametersExpectedEnd(self.lexer.peek_pos()));
                    self.lexer.skip_while(&Token::CloseParen);
                    return FuncParams {
                        span: ByteSpan::default(),
                        params,
                        variadic: false,
                    };
                }
            }
        }

        while !self.lexer.skip_if(&Token::CloseParen) {
            if self.lexer.is_eof() {
                self.log.report(&eof);
                break;
            }

            let mut limit_reported = false;
            Self::check_limit(params.len(), MAX_LIMIT, &mut limit_reported, &limit, self.log);

            if self.lexer.skip_if(&Token::Dot) {
                if !self.lexer.skip_if(&Token::Dot) || !self.lexer.skip_if(&Token::Dot) {
                    self.log
                        .report(&SyntaxErr::FunctionParameterVariadicExpected(self.lexer.peek_pos()));
                }

                if !self.lexer.skip_if(&Token::CloseParen) {
                    self.log.report(&end);
                    self.lexer.skip_while(&Token::CloseParen);
                }

                return FuncParams {
                    span: ByteSpan::default(),
                    params,
                    variadic: true,
                };
            }

            let param = self.parse_common_func_param(true);
            params.push(param);

            if !self.lexer.skip_if(&Token::Comma) && !self.lexer.next_is(&Token::CloseParen) {
                self.log.report(&end);
                self.lexer.skip_while(&Token::CloseParen);
                break;
            }
        }

        FuncParams {
            span: ByteSpan::default(),
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
            span: ByteSpan::default(),
            visibility: None,
            attributes,
            name,
            generics,
            parameters,
            return_type,
            definition,
            abi: None,
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
            span: ByteSpan::default(),
            visibility: None,
            kind,
            attributes,
            mutability,
            name,
            ty: var_type,
            initializer,
        }
    }

    fn parse_abi(&mut self) -> Option<ExternAbi> {
        // Parse an optional ABI string like "C" after extern keyword
        match self.lexer.peek_tok().token {
            Token::String(ref abi_name) => {
                self.lexer.skip_tok();
                Some(ExternAbi {
                    span: ByteSpan::default(),
                    name: NString::from(abi_name.clone()),
                })
            }
            _ => None,
        }
    }

    pub(crate) fn parse_item(&mut self) -> Item {
        let visibility = self.parse_visibility();

        // Check for extern keyword before other items
        let extern_abi = if self.lexer.skip_if(&Token::Extern) {
            self.parse_abi()
        } else {
            None
        };

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
                func.abi = extern_abi;
                Item::Function(func)
            }

            Token::Static | Token::Const => {
                let mut var = self.parse_global_variable();
                var.visibility = visibility;
                Item::Variable(var)
            }

            Token::OpenBrace if extern_abi.is_some() => {
                // This is an extern block { ... }
                // Parse the block items - each function gets the ABI
                let mut block_items = Vec::new();
                self.expect_open_brace();

                while !self.lexer.skip_if(&Token::CloseBrace) {
                    if self.lexer.is_eof() {
                        let bug = SyntaxErr::ExpectedItem(self.lexer.peek_pos());
                        self.log.report(&bug);
                        break;
                    }

                    let mut block_item = self.parse_item();
                    // Apply ABI to functions inside the extern block
                    if let Item::Function(ref mut func) = block_item
                        && func.abi.is_none()
                    {
                        func.abi = extern_abi.clone();
                    }
                    block_items.push(block_item);
                }

                // If no visibility set and there are items, wrap in a placeholder
                if block_items.is_empty() {
                    Item::SyntaxError(ItemSyntaxError {
                        span: ByteSpan::default(),
                    })
                } else {
                    block_items.remove(0)
                }
            }

            _ => {
                self.lexer.skip_tok();

                let bug = SyntaxErr::ExpectedItem(item_pos_begin);
                self.log.report(&bug);

                Item::SyntaxError(ItemSyntaxError {
                    span: ByteSpan::default(),
                })
            }
        }
    }
}
