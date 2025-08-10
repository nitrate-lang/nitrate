use super::parse::*;
use crate::parsetree::*;
use slog::error;

impl<'storage, 'logger, 'a> Parser<'storage, 'logger, 'a> {
    pub(crate) fn parse_if(&mut self) -> Option<ExprKey<'a>> {
        // TODO: Implement if expression parsing logic
        error!(self.log, "If expression parsing not implemented yet");
        None
    }

    pub(crate) fn parse_for(&mut self) -> Option<ExprKey<'a>> {
        // TODO: Implement for expression parsing logic
        error!(self.log, "For expression parsing not implemented yet");
        None
    }

    pub(crate) fn parse_while(&mut self) -> Option<ExprKey<'a>> {
        // TODO: Implement while expression parsing logic
        error!(self.log, "While expression parsing not implemented yet");
        None
    }

    pub(crate) fn parse_do(&mut self) -> Option<ExprKey<'a>> {
        // TODO: Implement do expression parsing logic
        error!(self.log, "Do expression parsing not implemented yet");
        None
    }

    pub(crate) fn parse_switch(&mut self) -> Option<ExprKey<'a>> {
        // TODO: Implement switch expression parsing logic
        error!(self.log, "Switch expression parsing not implemented yet");
        None
    }

    pub(crate) fn parse_break(&mut self) -> Option<ExprKey<'a>> {
        // TODO: Implement break expression parsing logic
        error!(self.log, "Break expression parsing not implemented yet");
        None
    }

    pub(crate) fn parse_continue(&mut self) -> Option<ExprKey<'a>> {
        // TODO: Implement continue expression parsing logic
        error!(self.log, "Continue expression parsing not implemented yet");
        None
    }

    pub(crate) fn parse_return(&mut self) -> Option<ExprKey<'a>> {
        // TODO: Implement return expression parsing logic
        error!(self.log, "Return expression parsing not implemented yet");
        None
    }

    pub(crate) fn parse_foreach(&mut self) -> Option<ExprKey<'a>> {
        // TODO: Implement foreach expression parsing logic
        error!(self.log, "Foreach expression parsing not implemented yet");
        None
    }

    pub(crate) fn parse_await(&mut self) -> Option<ExprKey<'a>> {
        // TODO: Implement await expression parsing logic
        error!(self.log, "Await expression parsing not implemented yet");
        None
    }

    pub(crate) fn parse_asm(&mut self) -> Option<ExprKey<'a>> {
        // TODO: Implement asm expression parsing logic
        error!(self.log, "Asm expression parsing not implemented yet");
        None
    }

    pub(crate) fn parse_assert(&mut self) -> Option<ExprKey<'a>> {
        // TODO: Implement assert expression parsing logic
        error!(self.log, "Assert expression parsing not implemented yet");
        None
    }
}
