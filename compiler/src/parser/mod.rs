use crate::lexer::*;
use crate::parsetree::*;

pub struct Parser<'a> {
    lexer: &'a mut Lexer<'a>,
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
        Ok(Parser { lexer })
    }

    pub fn get_lexer(&mut self) -> &mut Lexer<'a> {
        self.lexer
    }

    pub fn parse(&mut self) -> Option<()> {
        // TODO: Develop nitrate parser

        let func = Builder::get_function()
            .with_name("main")
            .with_return_type(Builder::get_i32())
            .with_definition(
                Builder::get_block()
                    .add_expression(
                        Builder::get_return()
                            .with_value(Builder::get_integer().with_u32(10).build())
                            .build(),
                    )
                    .build(),
            )
            .build();

        let mut tokens = Vec::new();
        func.to_code(&mut tokens, &CodeFormat::default());

        println!("{:?}", tokens);

        None
    }
}
