use crate::commands::lsp::{
    lsp::LspServer,
    rpc_server::{RpcRequest, RpcResponse},
};
use nitrate_diagnosis::{CompilerLog, FileId, intern_file_id};
use nitrate_translation::{
    parse::Parser,
    token_lexer::{Lexer, LexerError},
    tree_resolve::discover_symbols,
};
use serde_json::json;
use slog::info;

#[derive(serde::Serialize)]
struct CompletionItem {
    label: String,
    kind: u32,
    detail: Option<String>,
    documentation: Option<String>,
}

pub async fn rpc_method_text_document_completion(
    m: &mut LspServer,
    request: RpcRequest,
) -> anyhow::Result<RpcResponse> {
    info!(m.log, "LSP textDocument/completion requested");

    let params = request.params.clone();
    let uri = params
        .get("textDocument")
        .and_then(|td| td.get("uri"))
        .and_then(|u| u.as_str())
        .ok_or_else(|| anyhow::anyhow!("Missing textDocument.uri in completion request"))?;

    let text = m
        .filesystem
        .get(uri)
        .map(|fd| fd.content())
        .ok_or_else(|| anyhow::anyhow!("Missing file content for URI in completion request"))?;

    let lexer = match Lexer::new(text.as_bytes(), intern_file_id(uri)) {
        Ok(lex) => lex,
        Err(LexerError::SourceTooBig) => {
            anyhow::bail!("File too large for completion request");
        }
    };

    let log = CompilerLog::default();
    let package_name = "completion_package"; //FIXME: get package name
    let mut module = Parser::new(lexer, &log).parse_source(package_name.into());
    let symbols = discover_symbols(&mut module);

    let mut completions = Vec::new();
    for (symbol_name, _) in symbols {
        let parts = symbol_name.split("::").collect::<Vec<_>>();
        if let Some(last_part) = parts.last() {
            if last_part.is_empty() {
                continue;
            }

            completions.push(CompletionItem {
                label: last_part.to_string(),
                kind: 6, // Function kind
                detail: None,
                documentation: None,
            });
        }
    }

    let mut response = RpcResponse::from(request);
    response.result = serde_json::to_value(completions)?;
    Ok(response)
}
