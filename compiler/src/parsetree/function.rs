use super::expression::Expr;
use super::expression::{CodeFormat, ToCode};
use super::types::Type;
use crate::lexer::{Identifier, Keyword, Operator, Punctuation, Token};
use crate::parsetree::{Block, InnerType};

#[derive(Debug, Clone, PartialEq, PartialOrd, Hash)]
pub struct Function<'a> {
    parameters: Vec<(&'a str, Type<'a>, Option<Expr<'a>>)>,
    return_type: Option<Box<Type<'a>>>,
    attributes: Vec<Expr<'a>>,
    name: &'a str,
    definition: Option<Block<'a>>,
}

impl<'a> Function<'a> {
    pub fn new_definition(
        name: &'a str,
        parameters: Vec<(&'a str, Type<'a>, Option<Expr<'a>>)>,
        return_type: Option<Box<Type<'a>>>,
        attributes: Vec<Expr<'a>>,
        definition: Block<'a>,
    ) -> Self {
        Function {
            name,
            parameters,
            return_type,
            attributes,
            definition: Some(definition),
        }
    }

    pub fn new_declaration(
        name: &'a str,
        parameters: Vec<(&'a str, Type<'a>, Option<Expr<'a>>)>,
        return_type: Option<Box<Type<'a>>>,
        attributes: Vec<Expr<'a>>,
    ) -> Self {
        Function {
            name,
            parameters,
            return_type,
            attributes,
            definition: None,
        }
    }

    pub fn parameters(&self) -> &Vec<(&'a str, Type<'a>, Option<Expr<'a>>)> {
        &self.parameters
    }

    pub fn parameters_mut(&mut self) -> &mut Vec<(&'a str, Type<'a>, Option<Expr<'a>>)> {
        &mut self.parameters
    }

    pub fn return_type(&self) -> Option<&Type<'a>> {
        self.return_type.as_deref()
    }

    pub fn return_type_mut(&mut self) -> Option<&mut Type<'a>> {
        self.return_type.as_deref_mut()
    }

    pub fn attributes(&self) -> &Vec<Expr<'a>> {
        &self.attributes
    }

    pub fn attributes_mut(&mut self) -> &mut Vec<Expr<'a>> {
        &mut self.attributes
    }

    pub fn name(&self) -> &'a str {
        self.name
    }

    pub fn definition(&self) -> Option<&Block<'a>> {
        self.definition.as_ref()
    }

    pub fn definition_mut(&mut self) -> Option<&mut Block<'a>> {
        self.definition.as_mut()
    }
}

impl<'a> ToCode<'a> for Function<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        tokens.push(Token::Keyword(Keyword::Fn));

        if !self.attributes().is_empty() {
            tokens.push(Token::Punctuation(Punctuation::LeftBracket));
            for (i, attr) in self.attributes().iter().enumerate() {
                (i > 0).then(|| tokens.push(Token::Punctuation(Punctuation::Comma)));
                attr.to_code(tokens, options);
            }
            tokens.push(Token::Punctuation(Punctuation::RightBracket));
        }

        if !self.name().is_empty() {
            tokens.push(Token::Identifier(Identifier::new(self.name())));
        }

        tokens.push(Token::Punctuation(Punctuation::LeftParenthesis));
        for (i, (name, ty, default)) in self.parameters().iter().enumerate() {
            (i > 0).then(|| tokens.push(Token::Punctuation(Punctuation::Comma)));

            tokens.push(Token::Identifier(Identifier::new(name)));

            if !matches!(**ty, InnerType::InferType) {
                tokens.push(Token::Punctuation(Punctuation::Colon));
                ty.to_code(tokens, options);
            }

            if let Some(default_expr) = default {
                tokens.push(Token::Operator(Operator::Set));
                default_expr.to_code(tokens, options);
            }
        }
        tokens.push(Token::Punctuation(Punctuation::RightParenthesis));

        if let Some(return_type) = self.return_type() {
            if !matches!(**return_type, InnerType::InferType) {
                tokens.push(Token::Punctuation(Punctuation::Colon));
                return_type.to_code(tokens, options);
            }
        }

        if let Some(definition) = self.definition() {
            definition.to_code(tokens, options);
        }
    }
}

#[derive(Debug, Default, Clone, PartialEq, PartialOrd, Hash)]
pub struct FunctionBuilder<'a> {
    parameters: Vec<(&'a str, Type<'a>, Option<Expr<'a>>)>,
    return_type: Option<Box<Type<'a>>>,
    attributes: Vec<Expr<'a>>,
    name: &'a str,
    definition: Option<Block<'a>>,
}

impl<'a> FunctionBuilder<'a> {
    pub fn new(name: &'a str) -> Self {
        FunctionBuilder {
            name,
            parameters: Vec::new(),
            return_type: None,
            attributes: Vec::new(),
            definition: None,
        }
    }

    pub fn with_name(mut self, name: &'a str) -> Self {
        self.name = name;
        self
    }

    pub fn with_parameter(
        mut self,
        name: &'a str,
        ty: Type<'a>,
        default: Option<Expr<'a>>,
    ) -> Self {
        self.parameters.push((name, ty, default));
        self
    }

    pub fn with_return_type(mut self, ty: Type<'a>) -> Self {
        self.return_type = Some(Box::new(ty));
        self
    }

    pub fn with_attribute(mut self, attr: Expr<'a>) -> Self {
        self.attributes.push(attr);
        self
    }

    pub fn with_definition(mut self, definition: Block<'a>) -> Self {
        self.definition = Some(definition);
        self
    }

    pub fn build(self) -> Function<'a> {
        if let Some(definition) = self.definition {
            Function::new_definition(
                self.name,
                self.parameters,
                self.return_type,
                self.attributes,
                definition,
            )
        } else {
            Function::new_declaration(
                self.name,
                self.parameters,
                self.return_type,
                self.attributes,
            )
        }
    }
}
