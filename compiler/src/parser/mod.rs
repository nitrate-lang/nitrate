use crate::lexer::*;
use crate::parsetree::*;

pub struct Parser<'a> {
    lexer: Lexer<'a>,
}

impl<'a> Parser<'a> {
    pub fn new(lexer: Lexer<'a>) -> Self {
        // Builder::init();

        Parser { lexer }
    }

    pub fn into_lexer(self) -> Lexer<'a> {
        self.lexer
    }

    pub fn get_lexer(&self) -> &Lexer<'a> {
        &self.lexer
    }

    pub fn get_lexer_mut(&mut self) -> &mut Lexer<'a> {
        &mut self.lexer
    }

    pub fn parse(&mut self) -> Option<ExprRef<'a>> {
        // TODO: Develop nitrate parser

        let token = self.lexer.next_token();

        // Test ownership and coping
        match token.into_token() {
            // Token::Identifier(name) => {
            //     let func = Builder::get_function()
            //         .with_name(name.name())
            //         .with_return_type(Builder::get_i32())
            //         .with_definition(
            //             Builder::get_block()
            //                 .add_statement(
            //                     Builder::get_return()
            //                         .with_value(Builder::get_integer().with_u32(10).build())
            //                         .build(),
            //                 )
            //                 .build(),
            //         )
            //         .build();

            //     Some(func)
            // }
            _ => None,
        }
    }
}
