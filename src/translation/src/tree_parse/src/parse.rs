use crate::{
    resolve_import::{ImportContext, resolve_imports},
    resolve_path::resolve_paths,
};
use nitrate_diagnosis::CompilerLog;
use nitrate_nstring::NString;
use nitrate_token_lexer::Lexer;
use nitrate_tree::ast::Module;
use std::{ops::Deref, path::PathBuf};

pub struct Parser<'a, 'log> {
    pub(crate) lexer: Lexer<'a>,
    pub(crate) log: &'log CompilerLog,
}

pub struct ResolveCtx {
    pub package_search_paths: Vec<PathBuf>,
}

impl<'a, 'log> Parser<'a, 'log> {
    pub fn new(lexer: Lexer<'a>, log: &'log CompilerLog) -> Self {
        Parser { lexer, log: log }
    }

    pub fn parse_source(&mut self, package_name: NString, resolve: Option<ResolveCtx>) -> Module {
        let current_file = self.lexer.peek_tok().fileid;

        let mut items = Vec::new();

        while !self.lexer.is_eof() {
            let item = self.parse_item();
            items.push(item);
        }

        let mut module = Module {
            name: package_name.clone(),
            visibility: None,
            items,
            attributes: None,
        };

        if let Some(resolve_ctx) = resolve {
            if let Some(fileid) = current_file {
                let import_ctx = ImportContext::new(package_name, PathBuf::from(fileid.deref()))
                    .with_package_search_paths(resolve_ctx.package_search_paths);
                resolve_imports(&import_ctx, &mut module, self.log);
            }

            resolve_paths(&mut module, self.log);
        }

        module
    }
}
