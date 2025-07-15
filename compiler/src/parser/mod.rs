use crate::lexer::Lexer;

pub struct Parser<'a> {
    lexer: &'a mut Lexer<'a, 'a>,
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
    pub fn new(lexer: &'a mut Lexer<'a, 'a>) -> Result<Self, ParserConstructionError> {
        Ok(Parser { lexer })
    }

    pub fn get_lexer(&mut self) -> &mut Lexer<'a, 'a> {
        self.lexer
    }

    fn filename(&self) -> &str {
        self.lexer.current_position().filename()
    }

    pub fn parse(&mut self) -> Option<()> {
        // TODO: Develop nitrate parser
        None
    }
}
