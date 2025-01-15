#include <rapidjson/document.h>

#include <lsp/core/server.hh>
#include <lsp/lang/Parse.hh>
#include <lsp/route/RoutesList.hh>

using namespace rapidjson;

void DoDeclaration(const lsp::RequestMessage& req,
                    lsp::ResponseMessage& resp) {
  resp.Error(lsp::ErrorCodes::RequestFailed, "Not implemented");
  return;

  /// TODO: Implement declaration request

  if (!req.Params().HasMember("textDocument")) {
    resp.Error(lsp::ErrorCodes::InvalidParams, "Missing textDocument");
    return;
  }

  if (!req.Params()["textDocument"].IsObject()) {
    resp.Error(lsp::ErrorCodes::InvalidParams, "textDocument is not an object");
    return;
  }

  if (!req.Params()["textDocument"].HasMember("uri")) {
    resp.Error(lsp::ErrorCodes::InvalidParams, "Missing textDocument.uri");
    return;
  }

  if (!req.Params()["textDocument"]["uri"].IsString()) {
    resp.Error(lsp::ErrorCodes::InvalidParams,
               "textDocument.uri is not a string");
    return;
  }

  if (!req.Params().HasMember("position")) {
    resp.Error(lsp::ErrorCodes::InvalidParams, "Missing position");
    return;
  }

  if (!req.Params()["position"].IsObject()) {
    resp.Error(lsp::ErrorCodes::InvalidParams, "position is not an object");
    return;
  }

  if (!req.Params()["position"].HasMember("line")) {
    resp.Error(lsp::ErrorCodes::InvalidParams, "Missing position.line");
    return;
  }

  if (!req.Params()["position"]["line"].IsInt64()) {
    resp.Error(lsp::ErrorCodes::InvalidParams,
               "position.line is not an integer");
    return;
  }

  if (!req.Params()["position"].HasMember("character")) {
    resp.Error(lsp::ErrorCodes::InvalidParams, "Missing position.character");
    return;
  }

  if (!req.Params()["position"]["character"].IsInt64()) {
    resp.Error(lsp::ErrorCodes::InvalidParams,
               "position.character is not an integer");
    return;
  }

  std::string_view uri(req.Params()["textDocument"]["uri"].GetString(),
                       req.Params()["textDocument"]["uri"].GetStringLength());
  uint64_t line = req.Params()["position"]["line"].GetInt64();
  uint64_t character = req.Params()["position"]["character"].GetInt64();

  (void)line;
  (void)character;
  // std::string current_word;  /// TODO: Get the current word

  // lang::ParseTree tree = lang::ParseTreeCache::the().get(uri,
  // true).value_or(nullptr); if (!tree) {
  //   resp.error(lsp::ErrorCodes::InvalidParams, "No parse tree for this
  //   document"); return;
  // }

  /// TODO: Also must take into account the current scope
  /// TODO: Also must take into name resolution / namespaces
  /// TODO: If the semantics are illegal where there are duplicate names, emit
  /// the first one?
  /// TODO: Fuck, we don't know the original file name because of the macro
  /// preprocessing
  /// TODO: The macros (like @import) completely loose source location
  /// information
  /// TODO: Its like there are two types of locations, static locations and
  /// post-processed locations. We want static (unprocessed) locations herenow.
  /// How do i get that? My whole codebase is not legacy...
}
