use super::parse::Parser;
use crate::diagnosis::SyntaxErr;
use crate::helper::MAX_LIMIT;
use nitrate_tree::{SrcPos, SrcSpan};

use nitrate_token::Token;
use nitrate_tree::ast::{
    AssociatedItem, Enum, EnumVariant, ExternAbi, FuncParam, FuncParams, Function, Generics, GlobalVariable,
    GlobalVariableKind, Impl, Import, Item, ItemPath, ItemPathSegment, ItemSyntaxError, Module, Mutability, PathPrefix,
    ReferenceType, Spanned, Struct, StructField, Trait, Type, TypeAlias, TypeParam, TypePath, TypePathSegment, UseTree,
};

impl Parser<'_, '_> {
    fn parse_generics(&mut self) -> Option<Generics> {
        fn parse_generic_parameter(this: &mut Parser) -> TypeParam {
            let pos = this.lexer.peek_pos();
            let name_start = pos;
            let name = this.parse_nstring_name(SyntaxErr::GenericMissingParameterName(pos));

            // Consume optional type constraint after `:`
            // (TypeParam doesn't store constraints yet, so we just skip the type)
            if this.lexer.skip_if(&Token::Colon) {
                this.parse_type();
            }

            let default = if this.lexer.skip_if(&Token::Eq) {
                Some(this.parse_type())
            } else {
                None
            };

            let name_end = this.lexer.current_pos();

            TypeParam {
                span: SrcSpan::new(name_start, name_end),
                name,
                default_value: default,
            }
        }

        let pos = self.lexer.peek_pos();

        if !self.lexer.skip_if(&Token::Lt) {
            return None;
        }

        let eof = SyntaxErr::GenericParameterExpectedEnd(self.lexer.peek_pos());
        let limit = SyntaxErr::GenericParameterLimit(self.lexer.peek_pos());
        let end = SyntaxErr::GenericParameterExpectedEnd(self.lexer.peek_pos());

        let params =
            self.parse_comma_separated_list(&Token::Gt, MAX_LIMIT, false, eof, limit, end, parse_generic_parameter);

        Some(Generics {
            span: SrcSpan::new(pos, self.lexer.current_pos()), // updated after params parsed
            params,
        })
    }

    fn parse_module(&mut self) -> Module {
        let start = self.lexer.peek_pos();
        assert!(self.lexer.peek_tok().token == Token::Mod);
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();

        let err = SyntaxErr::ModuleMissingName(self.lexer.peek_pos());
        let name = self.parse_nstring_name(err);

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
            span: SrcSpan::new(start, self.lexer.current_pos()),
            visibility: None,
            attributes,
            name,
            items,
        }
    }

    #[allow(dead_code)]
    fn parse_item_path(&mut self) -> ItemPath {
        let mut segments = Vec::new();

        // Handle leading `::` (global path like `::std::mem`)
        let has_global_prefix = self.parse_double_colon();

        if !self.lexer.is_eof() {
            let name_start = self.lexer.peek_pos();

            // Detect if the first token is a path prefix keyword
            let prefix_info: Option<PathPrefix> = match self.lexer.peek_tok().token {
                Token::Crate => {
                    self.lexer.skip_tok();
                    Some(PathPrefix::Crate)
                }
                Token::Super => {
                    self.lexer.skip_tok();
                    let first_super_start = name_start;
                    let mut super_count = 1;

                    // Consume chained `::super` — each `super::` adds one level up
                    loop {
                        let rewind_pos = self.lexer.peek_pos();
                        if !self.parse_double_colon() {
                            break; // No trailing `::`, bare `super` or `super::super`
                        }
                        if self.lexer.next_is(&Token::Super) {
                            self.lexer.skip_tok();
                            super_count += 1;
                        } else {
                            // The token after `::` is not `super`.
                            // If it's a name, this is `super::name` and we parse it as the path.
                            // If it's `*`, `{`, `as`, `;`, etc., rewind and let parse_use_tree handle it.
                            if self.lexer.next_is(&Token::Star)
                                || self.lexer.next_is(&Token::OpenBrace)
                                || self.lexer.next_is(&Token::Semi)
                                || self.lexer.next_is(&Token::As)
                            {
                                self.lexer.rewind(rewind_pos);
                                break;
                            }

                            // It must be a segment name: push supers, then the name, then return
                            for i in 0..super_count {
                                segments.push(ItemPathSegment {
                                    span: SrcSpan::new(first_super_start, self.lexer.current_pos()),
                                    segment: "super".into(),
                                    prefix: if i == 0 { Some(PathPrefix::Super) } else { None },
                                });
                            }

                            let seg_name_start = self.lexer.peek_pos();
                            let Some(seg_name) = self.lexer.next_if_name() else {
                                self.lexer.rewind(rewind_pos);
                                break;
                            };
                            let seg_name_end = self.lexer.current_pos();
                            segments.push(ItemPathSegment {
                                span: SrcSpan::new(seg_name_start, seg_name_end),
                                segment: seg_name,
                                prefix: None,
                            });
                            return self.finish_path(segments);
                        }
                    }

                    // No trailing name found — it's `super` or `super::super` (bare).
                    for i in 0..super_count {
                        segments.push(ItemPathSegment {
                            span: SrcSpan::new(first_super_start, self.lexer.current_pos()),
                            segment: "super".into(),
                            prefix: if i == 0 { Some(PathPrefix::Super) } else { None },
                        });
                    }
                    return self.finish_path(segments);
                }
                Token::SelfKeyword | Token::SelfType => {
                    self.lexer.skip_tok();
                    Some(PathPrefix::SelfPath)
                }
                _ => None,
            };

            if let Some(prefix) = prefix_info {
                // We consumed a prefix keyword (`crate` or `self`).
                // Push the prefix segment.
                segments.push(ItemPathSegment {
                    span: SrcSpan::new(name_start, self.lexer.current_pos()),
                    segment: match prefix {
                        PathPrefix::Crate => "crate",
                        PathPrefix::SelfPath => "self",
                        PathPrefix::Super => "super",
                    }
                    .into(),
                    prefix: Some(prefix),
                });

                let rewind_pos = self.lexer.peek_pos();
                if !self.parse_double_colon() {
                    // Bare keyword (e.g., `use crate;` or `use self;`) — we're done
                    return self.finish_path(segments);
                }

                // We consumed `::`. Check if the next token is a use-tree
                // operator (`*`, `{`, `as`). If so, rewind and let parse_use_tree handle it.
                if self.lexer.next_is(&Token::Star)
                    || self.lexer.next_is(&Token::OpenBrace)
                    || self.lexer.next_is(&Token::Semi)
                    || self.lexer.next_is(&Token::As)
                {
                    self.lexer.rewind(rewind_pos);
                    // segments already has the prefix — return it so parse_use_tree
                    // can consume `::*` or `::{...}` after the path.
                    return self.finish_path(segments);
                }

                // After prefix::, parse the first real segment name directly.
                let seg_name_start = self.lexer.peek_pos();
                let err = SyntaxErr::PathExpectedName(self.lexer.peek_pos());
                let segment = self.parse_string_name(err);
                let seg_name_end = self.lexer.current_pos();

                if !segment.is_empty() {
                    segments.push(ItemPathSegment {
                        span: SrcSpan::new(seg_name_start, seg_name_end),
                        segment,
                        prefix: None,
                    });
                }
            } else {
                // Regular first segment name (or empty for global `::name`)
                if has_global_prefix {
                    // The global `::` prefix consumed. Parse the first name from here.
                }
                let first_seg_start = self.lexer.peek_pos();
                let err = SyntaxErr::PathExpectedName(self.lexer.peek_pos());
                let segment = self.parse_string_name(err);
                let first_seg_end = self.lexer.current_pos();

                if !segment.is_empty() {
                    segments.push(ItemPathSegment {
                        span: SrcSpan::new(first_seg_start, first_seg_end),
                        segment,
                        prefix: None,
                    });
                }
                // Fall through to parse remaining `::name` pairs
            }
        }

        // Parse remaining segments separated by `::`
        while !self.lexer.is_eof() {
            let rewind_pos = self.lexer.peek_pos();

            if !self.parse_double_colon() {
                if segments.is_empty() {
                    let bug = SyntaxErr::PathExpectedName(self.lexer.peek_pos());
                    self.log.report(&bug);
                }
                break;
            }

            // After `::`, parse the segment name (or prefix keyword)
            let name_start = self.lexer.peek_pos();
            let seg_prefix = match self.lexer.peek_tok().token {
                Token::Crate => {
                    self.lexer.skip_tok();
                    Some(PathPrefix::Crate)
                }
                Token::Super => {
                    self.lexer.skip_tok();
                    Some(PathPrefix::Super)
                }
                Token::SelfKeyword | Token::SelfType => {
                    self.lexer.skip_tok();
                    Some(PathPrefix::SelfPath)
                }
                _ => None,
            };

            let name_end = self.lexer.current_pos();
            let segment = if seg_prefix.is_some() {
                match seg_prefix {
                    Some(PathPrefix::Crate) => "crate".into(),
                    Some(PathPrefix::Super) => "super".into(),
                    Some(PathPrefix::SelfPath) => "self".into(),
                    None => unreachable!(),
                }
            } else {
                let Some(seg) = self.lexer.next_if_name() else {
                    self.lexer.rewind(rewind_pos);
                    break;
                };
                seg
            };

            segments.push(ItemPathSegment {
                span: SrcSpan::new(name_start, name_end),
                segment,
                prefix: seg_prefix,
            });
        }

        self.finish_path(segments)
    }

    fn finish_path(&self, segments: Vec<ItemPathSegment>) -> ItemPath {
        let path_start = segments.first().map(|s| s.span.start).unwrap_or(SrcPos::default());
        let path_end = segments.last().map(|s| s.span.end).unwrap_or(SrcPos::default());

        ItemPath {
            span: SrcSpan::new(path_start, path_end),
            segments,
        }
    }

    fn parse_use(&mut self) -> Import {
        fn parse_use_tree(this: &mut Parser) -> UseTree {
            let path = this.parse_item_path();

            if this.parse_double_colon() {
                if this.lexer.skip_if(&Token::Star) {
                    UseTree::UseAll {
                        span: SrcSpan::new(path.span().start, this.lexer.current_pos()),
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
                        span: SrcSpan::new(path.span().start, this.lexer.current_pos()),
                        path,
                        group,
                    }
                } else {
                    let bug = SyntaxErr::ImportExpectedStarOrGroup(this.lexer.peek_pos());
                    this.log.report(&bug);
                    UseTree::Single {
                        span: path.span(),
                        path,
                    }
                }
            } else if this.lexer.skip_if(&Token::As) {
                let err = SyntaxErr::ImportAliasMissingName(this.lexer.peek_pos());
                let alias = this.parse_string_name(err);
                let alias_end = this.lexer.current_pos();

                UseTree::Alias {
                    span: SrcSpan::new(path.span().start, alias_end),
                    path,
                    alias: alias.into(),
                }
            } else {
                UseTree::Single {
                    span: path.span(),
                    path,
                }
            }
        }

        let start = self.lexer.peek_pos();
        assert!(self.lexer.peek_tok().token == Token::Use);
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();
        let use_tree = parse_use_tree(self);

        self.expect_semicolon();

        Import {
            span: SrcSpan::new(start, self.lexer.current_pos()),
            visibility: None,
            attributes,
            use_tree,
            resolved: None,
        }
    }

    fn parse_type_alias(&mut self) -> TypeAlias {
        let start = self.lexer.peek_pos();
        assert!(self.lexer.peek_tok().token == Token::Type);
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();

        let err = SyntaxErr::TypeAliasMissingName(self.lexer.peek_pos());
        let name = self.parse_nstring_name(err);

        let generics = self.parse_generics();

        let aliased_type = if self.lexer.skip_if(&Token::Eq) {
            Some(self.parse_type())
        } else {
            None
        };

        self.expect_semicolon();

        TypeAlias {
            span: SrcSpan::new(start, self.lexer.current_pos()),
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

            let name_start = this.lexer.peek_pos();
            let err = SyntaxErr::EnumMissingVariantName(this.lexer.peek_pos());
            let name = this.parse_nstring_name(err);

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
                span: SrcSpan::new(name_start, this.lexer.current_pos()),
                attributes,
                name,
                ty: variant_type,
                default_value: value,
            }
        }

        let start = self.lexer.peek_pos();
        assert!(self.lexer.peek_tok().token == Token::Enum);
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();

        let err = SyntaxErr::EnumMissingName(self.lexer.peek_pos());
        let name = self.parse_nstring_name(err);

        let generics = self.parse_generics();

        self.expect_open_brace();

        let eof = SyntaxErr::EnumExpectedEnd(self.lexer.peek_pos());
        let limit = SyntaxErr::EnumVariantLimit(self.lexer.peek_pos());
        let end = SyntaxErr::EnumExpectedEnd(self.lexer.peek_pos());

        let variants =
            self.parse_comma_separated_list(&Token::CloseBrace, MAX_LIMIT, true, eof, limit, end, parse_enum_variant);

        Enum {
            span: SrcSpan::new(start, self.lexer.current_pos()),
            visibility: None,
            attributes,
            name,
            generics,
            variants,
        }
    }

    fn parse_struct(&mut self) -> Struct {
        fn parse_struct_field(this: &mut Parser) -> StructField {
            let field_start = this.lexer.peek_pos();
            let visibility = this.parse_visibility();
            let attributes = this.parse_attributes();

            let err = SyntaxErr::StructureMissingFieldName(this.lexer.peek_pos());
            let name = this.parse_nstring_name(err);

            this.expect_colon();

            let field_type = this.parse_type();

            let default = if this.lexer.skip_if(&Token::Eq) {
                Some(this.parse_expression())
            } else {
                None
            };

            StructField {
                span: SrcSpan::new(field_start, this.lexer.current_pos()),
                visibility,
                attributes,
                name,
                ty: field_type,
                default_value: default,
            }
        }

        let start = self.lexer.peek_pos();
        assert!(self.lexer.peek_tok().token == Token::Struct);
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();

        let err = SyntaxErr::StructureMissingName(self.lexer.peek_pos());
        let name = self.parse_nstring_name(err);

        let generics = self.parse_generics();

        self.expect_open_brace();

        let eof = SyntaxErr::StructureExpectedEnd(self.lexer.peek_pos());
        let limit = SyntaxErr::StructureFieldLimit(self.lexer.peek_pos());
        let end = SyntaxErr::StructureExpectedEnd(self.lexer.peek_pos());

        let fields =
            self.parse_comma_separated_list(&Token::CloseBrace, MAX_LIMIT, true, eof, limit, end, parse_struct_field);

        Struct {
            span: SrcSpan::new(start, self.lexer.current_pos()),
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
                let err_pos = self.lexer.peek_pos();
                self.lexer.skip_tok();

                let bug = SyntaxErr::TraitDoesNotAllowItem(self.lexer.peek_pos());
                self.log.report(&bug);

                AssociatedItem::SyntaxError(ItemSyntaxError {
                    span: SrcSpan::new(err_pos, self.lexer.current_pos()),
                })
            }
        }
    }

    fn parse_trait(&mut self) -> Trait {
        let start = self.lexer.peek_pos();
        assert!(self.lexer.peek_tok().token == Token::Trait);
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();

        let err = SyntaxErr::TraitMissingName(self.lexer.peek_pos());
        let name = self.parse_nstring_name(err);

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
            span: SrcSpan::new(start, self.lexer.current_pos()),
            visibility: None,
            attributes,
            name,
            generics,
            items,
        }
    }

    fn parse_implementation(&mut self) -> Impl {
        let start = self.lexer.peek_pos();
        assert!(self.lexer.peek_tok().token == Token::Impl);
        self.lexer.skip_tok();

        let generics = self.parse_generics();

        let (trait_path, for_type) = if self.lexer.skip_if(&Token::Trait) {
            // impl trait Type for Type { ... }
            let path = self.parse_type_path();

            if !self.lexer.skip_if(&Token::For) {
                let bug = SyntaxErr::ImplMissingFor(self.lexer.peek_pos());
                self.log.report(&bug);
            }

            (Some(path), self.parse_type())
        } else {
            // Could be: impl Type { ... } or impl Type for Type { ... }
            let first_type = self.parse_type();

            if self.lexer.skip_if(&Token::For) {
                // impl TraitType for Type { ... }
                // Extract TypePath from Type if possible
                let trait_path = match &first_type {
                    Type::TypePath(tp) => Some(*tp.clone()),
                    _ => None,
                };
                (trait_path, self.parse_type())
            } else {
                // impl Type { ... }
                (None, first_type)
            }
        };

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
            span: SrcSpan::new(start, self.lexer.current_pos()),
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
                let self_start = rewind_pos; // start of `&`
                // &self - create a reference type for the parameter
                self.lexer.skip_tok(); // consume self
                let name = "self".into();
                let self_ty_end = self.lexer.current_pos();
                let ref_span = SrcSpan::new(self_start, self_ty_end);
                let ty = Type::ReferenceType(Box::new(ReferenceType {
                    span: ref_span,
                    lifetime: None,
                    exclusivity: None,
                    mutability: None,
                    to: Type::TypePath(Box::new(TypePath {
                        span: ref_span,
                        segments: vec![TypePathSegment {
                            span: ref_span,
                            name: "Self".to_string(),
                            type_arguments: None,
                        }],
                        resolved_path: None,
                    })),
                }));
                return Some(FuncParam {
                    span: SrcSpan::new(self_start, self_ty_end),
                    attributes: None,
                    mutability: None,
                    name,
                    ty,
                    default_value: None,
                });
            } else if self.lexer.skip_if(&Token::Mut) && self.lexer.next_is(&Token::SelfKeyword) {
                let self_start = rewind_pos; // start of `&`
                // &mut self
                self.lexer.skip_tok(); // consume self
                let name = "self".into();
                let self_ty_end = self.lexer.current_pos();
                let ref_span = SrcSpan::new(self_start, self_ty_end);
                let ty = Type::ReferenceType(Box::new(ReferenceType {
                    span: ref_span,
                    lifetime: None,
                    exclusivity: None,
                    mutability: Some(Mutability::Mut),
                    to: Type::TypePath(Box::new(TypePath {
                        span: ref_span,
                        segments: vec![TypePathSegment {
                            span: ref_span,
                            name: "Self".to_string(),
                            type_arguments: None,
                        }],
                        resolved_path: None,
                    })),
                }));
                return Some(FuncParam {
                    span: SrcSpan::new(self_start, self_ty_end),
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
            let self_start = self.lexer.peek_pos();
            self.lexer.skip_tok(); // consume self
            let name = "self".into();
            let self_ty_end = self.lexer.current_pos();
            let inner_span = SrcSpan::new(self_start, self_ty_end);
            let ty = Type::TypePath(Box::new(TypePath {
                span: inner_span,
                segments: vec![TypePathSegment {
                    span: inner_span,
                    name: "Self".to_string(),
                    type_arguments: None,
                }],
                resolved_path: None,
            }));
            return Some(FuncParam {
                span: SrcSpan::new(self_start, self_ty_end),
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
        let paren_start = self.lexer.peek_pos();
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
                        span: SrcSpan::new(paren_start, self.lexer.current_pos()),
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
                    span: SrcSpan::new(paren_start, self.lexer.current_pos()),
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
            span: SrcSpan::new(paren_start, self.lexer.current_pos()),
            params,
            variadic: false,
        }
    }

    fn parse_named_function(&mut self) -> Function {
        let start = self.lexer.peek_pos();
        assert!(self.lexer.peek_tok().token == Token::Fn);
        self.lexer.skip_tok();

        let attributes = self.parse_attributes();

        let err = SyntaxErr::FunctionMissingName(self.lexer.peek_pos());
        let name = self.parse_nstring_name(err);

        let generics = self.parse_generics();
        let parameters = self.parse_function_parameters();

        let return_type = self.parse_return_type_arrow();

        let definition = if self.lexer.skip_if(&Token::Semi) {
            None
        } else {
            Some(self.parse_block())
        };

        Function {
            span: SrcSpan::new(start, self.lexer.current_pos()),
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
        let start = self.lexer.peek_pos();
        let kind = match self.lexer.next_tok().token {
            Token::Static => GlobalVariableKind::Static,
            Token::Const => GlobalVariableKind::Const,
            _ => unreachable!(),
        };

        let attributes = self.parse_attributes();

        let mutability = self.parse_mutability();

        let err = SyntaxErr::VariableMissingName(self.lexer.peek_pos());
        let name = self.parse_nstring_name(err);

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
            span: SrcSpan::new(start, self.lexer.current_pos()),
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
                let abi_start = self.lexer.peek_pos();
                self.lexer.skip_tok();
                let abi_end = self.lexer.current_pos();
                Some(ExternAbi {
                    span: SrcSpan::new(abi_start, abi_end),
                    name: abi_name.clone().into(),
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
                        span: SrcSpan::new(item_pos_begin, self.lexer.current_pos()),
                    })
                } else {
                    block_items.remove(0)
                }
            }

            _ => {
                self.lexer.skip_tok();

                let bug = SyntaxErr::ExpectedItem(item_pos_begin.clone());
                self.log.report(&bug);

                Item::SyntaxError(ItemSyntaxError {
                    span: SrcSpan::new(item_pos_begin, self.lexer.current_pos()),
                })
            }
        }
    }
}
