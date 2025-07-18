use crate::lexer::*;
use crate::parsetree::*;

pub struct Parser<'a> {
    lexer: &'a mut Lexer<'a>,
    types: TypeFactory<'a>,
}

pub enum ParserConstructionError {
    Error,
}

impl std::fmt::Display for ParserConstructionError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            ParserConstructionError::Error => write!(f, "Parser construction error"),
        }
    }
}

impl<'a> Parser<'a> {
    pub fn new(lexer: &'a mut Lexer<'a>) -> Result<Self, ParserConstructionError> {
        Ok(Parser {
            lexer,
            types: TypeFactory::new(),
        })
    }

    pub fn get_lexer(&mut self) -> &mut Lexer<'a> {
        self.lexer
    }

    pub fn parse(&mut self) -> Option<()> {
        // TODO: Develop nitrate parser

        let functor = self.types.get_function_type(
            Vec::from([
                ("arg1", self.types.get_i32(), None),
                ("arg2", self.types.get_i8(), None),
            ]),
            Some(self.types.get_f128()),
            Vec::from([Expr::new(
                InnerExpr::Float(FloatLit::new(1.0)),
                Metadata::default(),
            )]),
            false,
        );

        let mut tokens = Vec::new();
        functor.to_code(&mut tokens, &CodeFormat::default());

        println!("{:?}", tokens);

        None
    }
}
