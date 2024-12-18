#include <rapidjson/document.h>

#include <lsp/core/server.hh>
#include <lsp/lang/Parse.hh>
#include <lsp/route/RoutesList.hh>

using namespace rapidjson;

void do_declaration(const lsp::RequestMessage& req,
                    lsp::ResponseMessage& resp) {
  resp.error(lsp::ErrorCodes::RequestFailed, "Not implemented");
  return;

  /// TODO: Implement declaration request

  if (!req.params().HasMember("textDocument")) {
    resp.error(lsp::ErrorCodes::InvalidParams, "Missing textDocument");
    return;
  }

  if (!req.params()["textDocument"].IsObject()) {
    resp.error(lsp::ErrorCodes::InvalidParams, "textDocument is not an object");
    return;
  }

  if (!req.params()["textDocument"].HasMember("uri")) {
    resp.error(lsp::ErrorCodes::InvalidParams, "Missing textDocument.uri");
    return;
  }

  if (!req.params()["textDocument"]["uri"].IsString()) {
    resp.error(lsp::ErrorCodes::InvalidParams,
               "textDocument.uri is not a string");
    return;
  }

  if (!req.params().HasMember("position")) {
    resp.error(lsp::ErrorCodes::InvalidParams, "Missing position");
    return;
  }

  if (!req.params()["position"].IsObject()) {
    resp.error(lsp::ErrorCodes::InvalidParams, "position is not an object");
    return;
  }

  if (!req.params()["position"].HasMember("line")) {
    resp.error(lsp::ErrorCodes::InvalidParams, "Missing position.line");
    return;
  }

  if (!req.params()["position"]["line"].IsInt64()) {
    resp.error(lsp::ErrorCodes::InvalidParams,
               "position.line is not an integer");
    return;
  }

  if (!req.params()["position"].HasMember("character")) {
    resp.error(lsp::ErrorCodes::InvalidParams, "Missing position.character");
    return;
  }

  if (!req.params()["position"]["character"].IsInt64()) {
    resp.error(lsp::ErrorCodes::InvalidParams,
               "position.character is not an integer");
    return;
  }

  std::string_view uri(req.params()["textDocument"]["uri"].GetString(),
                       req.params()["textDocument"]["uri"].GetStringLength());
  uint64_t line = req.params()["position"]["line"].GetInt64();
  uint64_t character = req.params()["position"]["character"].GetInt64();

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

  // tree->root()->iterate([&](const Base* node) {
  //   if (!node->is_decl() || node->as<npar::Decl>()->get_name().view() !=
  //   current_word)
  //       [[likely]] {
  //     return true;
  //   }

  //   const npar::Decl* decl = node->as<npar::Decl>();

  //   auto start_pos = decl->get_offset();

  //   resp->SetObject();
  //   resp->AddMember("line", qlex_line(tree->lexer(), start_pos),
  //   resp->GetAllocator()); resp->AddMember("character",
  //   qlex_col(tree->lexer(), start_pos), resp->GetAllocator());

  //   return false;
  // });
}
