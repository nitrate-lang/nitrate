use nitrate_diagnosis::CompilerLog;
use nitrate_nstring::NString;
use nitrate_token::Token;
use nitrate_token_lexer::Lexer;
use nitrate_tree::SrcSpan;
use std::path::PathBuf;

pub struct Parser<'a, 'log> {
    pub(crate) lexer: Lexer<'a>,
    pub(crate) log: &'log CompilerLog,
}

pub struct ResolveCtx {
    pub package_search_paths: Vec<PathBuf>,
}

impl<'a, 'log> Parser<'a, 'log> {
    pub fn new(lexer: Lexer<'a>, log: &'log CompilerLog) -> Self {
        Parser { lexer, log }
    }

    /// Get the source bytes for reconstruction.
    pub fn source(&self) -> &'a [u8] {
        self.lexer.source
    }

    /// Get the end byte offset of the last consumed token.
    pub(crate) fn current_pos(&self) -> u32 {
        self.lexer.current_pos().offset
    }

    pub fn is_eof(&mut self) -> bool {
        self.lexer.is_eof()
    }

    /// Peek at the token immediately after a `{` to determine if it looks like
    /// a struct field start vs a block body.
    ///
    /// Returns `true` if the token following `{` could be the start of a struct
    /// field initializer (e.g., `Foo { x: 1 }`, `Foo {}`, `Foo { : 1 }`).
    /// Returns `false` if it looks like a block body (e.g., `items { break; }`).
    pub(crate) fn peek_is_struct_field_start(&mut self) -> bool {
        let saved = self.lexer.current_pos();
        // Skip the `{` to peek at what comes next
        self.lexer.skip_tok();
        let result = match self.lexer.peek_tok().token {
            // Empty struct: `Foo {}`
            Token::CloseBrace => true,
            // Struct with named field: `Foo { x: 1 }` or `Foo { x 1 }`
            Token::Name(_) | Token::SelfKeyword => true,
            // Struct with anonymous field: `Foo { : 1 }` (error path)
            Token::Colon => true,
            // Struct with attributes: `Foo { #[attr] x: 1 }`
            Token::OpenBracket => true,
            // Statements and keywords that only appear in blocks, not struct fields
            Token::Break | Token::Continue | Token::Ret | Token::Let | Token::Var => false,
            Token::If | Token::For | Token::While | Token::Match | Token::Fn => false,
            Token::OpenBrace | Token::Unsafe | Token::Safe | Token::Await => false,
            // Anything else might be an expression, treat as block
            _ => false,
        };
        self.lexer.rewind(saved);
        result
    }

    pub fn parse_source(&mut self, package_name: NString) -> nitrate_tree::ast::Module {
        let mut items = Vec::new();

        // The module span starts at offset 0 (beginning of source file).
        // Must be captured before disabling trivia to include leading whitespace/comments.
        let module_start = 0u32;

        // Disable trivia so the lexer skips whitespace/comments when parsing items.
        // Trivia is preserved for reconstruction via SrcPos ranges on individual items.
        self.lexer.disable_trivia();

        while !self.is_eof() {
            let item = self.parse_item();
            items.push(item);
        }

        let module_end = self.current_pos();

        nitrate_tree::ast::Module {
            name: package_name.clone(),
            visibility: None,
            items,
            attributes: None,
            span: SrcSpan::new(module_start, module_end),
        }
    }
}
