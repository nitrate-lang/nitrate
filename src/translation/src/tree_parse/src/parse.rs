use nitrate_diagnosis::CompilerLog;
use nitrate_nstring::NString;
use nitrate_token::Token;
use nitrate_token_lexer::Lexer;
use nitrate_tree::ByteSpan;
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

    /// Get the start byte offset of the current peek token.
    pub(crate) fn current_offset(&mut self) -> u32 {
        self.lexer.peek_pos().offset
    }

    /// Get the end byte offset of the last consumed token.
    pub(crate) fn current_pos(&self) -> u32 {
        self.lexer.current_pos().offset
    }

    /// Create a ByteSpan from a start offset to current end position.
    pub(crate) fn span_from(&self, start: u32) -> ByteSpan {
        ByteSpan::new(start, self.lexer.current_pos().offset)
    }

    pub(crate) fn peek_tok(&mut self) -> Token {
        self.lexer.peek_tok().token
    }

    pub(crate) fn next_is(&mut self, matches: &Token) -> bool {
        self.lexer.next_is(matches)
    }

    pub(crate) fn skip_tok(&mut self) {
        self.lexer.skip_tok();
    }

    pub(crate) fn skip_if(&mut self, matches: &Token) -> bool {
        self.lexer.skip_if(matches)
    }

    pub(crate) fn skip_while(&mut self, not: &Token) {
        self.lexer.skip_while(not);
    }

    pub fn is_eof(&mut self) -> bool {
        self.lexer.is_eof()
    }

    pub(crate) fn next_if_name(&mut self) -> Option<String> {
        self.lexer.next_if_name()
    }

    pub(crate) fn rewind(&mut self, pos: nitrate_token::SourcePosition) {
        self.lexer.rewind(pos);
    }

    pub fn parse_source(&mut self, package_name: NString) -> nitrate_tree::ast::Module {
        let mut items = Vec::new();

        // The module span starts at offset 0 (beginning of source file).
        // Must be captured before disabling trivia to include leading whitespace/comments.
        let module_start = 0u32;

        // Disable trivia so the lexer skips whitespace/comments when parsing items.
        // Trivia is preserved for reconstruction via ByteSpan ranges on individual items.
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
            span: ByteSpan::new(module_start, module_end),
        }
    }
}
