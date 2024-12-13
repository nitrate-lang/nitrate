#include <nitrate-core/Error.h>
#include <rapidjson/document.h>

#include <cctype>
#include <cstdint>
#include <lsp/core/SyncFS.hh>
#include <lsp/core/server.hh>
#include <lsp/lang/Format.hh>
#include <lsp/route/RoutesList.hh>
#include <nitrate-core/Classes.hh>
#include <nitrate-lexer/Classes.hh>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/Classes.hh>
#include <sstream>
#include <string>

using namespace rapidjson;

void do_formatting(const lsp::RequestMessage& req, lsp::ResponseMessage& resp) {
  struct Position {
    size_t line = 0;
    size_t character = 0;
  };

  struct Range {
    Position start;
    Position end;
  };

  struct FormattingOptions {
    size_t tabSize = 0;
    bool insertSpaces = false;
  };

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

  if (!req.params().HasMember("options")) {
    resp.error(lsp::ErrorCodes::InvalidParams, "Missing options");
    return;
  }

  if (!req.params()["options"].IsObject()) {
    resp.error(lsp::ErrorCodes::InvalidParams, "options is not an object");
    return;
  }

  if (!req.params()["options"].HasMember("tabSize")) {
    resp.error(lsp::ErrorCodes::InvalidParams, "Missing options.tabSize");
    return;
  }

  if (!req.params()["options"]["tabSize"].IsInt()) {
    resp.error(lsp::ErrorCodes::InvalidParams,
               "options.tabSize is not an integer");
    return;
  }

  if (req.params()["options"]["tabSize"].GetUint() == 0) {
    resp.error(lsp::ErrorCodes::InvalidParams, "options.tabSize is 0");
    return;
  }

  if (!req.params()["options"].HasMember("insertSpaces")) {
    resp.error(lsp::ErrorCodes::InvalidParams, "Missing options.insertSpaces");
    return;
  }

  if (!req.params()["options"]["insertSpaces"].IsBool()) {
    resp.error(lsp::ErrorCodes::InvalidParams,
               "options.insertSpaces is not a boolean");
    return;
  }

  FormattingOptions options;
  options.tabSize = req.params()["options"]["tabSize"].GetInt();
  options.insertSpaces = req.params()["options"]["insertSpaces"].GetBool();

  std::string uri = req.params()["textDocument"]["uri"].GetString();
  auto file_opt = SyncFS::the().open(uri);
  if (!file_opt.has_value()) {
    resp.error(lsp::ErrorCodes::InternalError, "Failed to open file");
    return;
  }
  auto file = file_opt.value();

  std::stringstream ss(*file->content());

  qcore_env env;
  qlex lexer(ss, uri.c_str(), env.get());
  nr_syn parser(lexer.get(), env.get());

  npar_node_t* root = nullptr;
  if (!npar_do(parser.get(), &root)) {
    resp.error(lsp::ErrorCodes::InternalError, "Failed to parse document");
    return;
  }

  if (!npar_check(parser.get(), root)) {
    resp.error(lsp::ErrorCodes::InternalError, "Failed to parse document");
    return;
  }

  LOG(INFO) << "Requested document format";

  std::stringstream formatted_ss;
  if (!lsp::fmt::FormatterFactory::create(lsp::fmt::Styleguide::Cambrian,
                                          formatted_ss)
           ->format(root)) {
    resp.error(lsp::ErrorCodes::InternalError, "Failed to format document");
    return;
  }

  auto formatted = formatted_ss.str();

  file->replace(0, -1, formatted);

  ///==========================================================
  /// Send the whole new file contents

  resp->SetArray();
  Value edit(kObjectType);
  edit.AddMember("range", Value(kObjectType), resp->GetAllocator());
  edit["range"].AddMember("start", Value(kObjectType), resp->GetAllocator());
  edit["range"]["start"].AddMember("line", 0, resp->GetAllocator());
  edit["range"]["start"].AddMember("character", 0, resp->GetAllocator());
  edit["range"].AddMember("end", Value(kObjectType), resp->GetAllocator());
  edit["range"]["end"].AddMember("line", SIZE_MAX, resp->GetAllocator());
  edit["range"]["end"].AddMember("character", SIZE_MAX, resp->GetAllocator());
  edit.AddMember(
      "newText",
      Value(formatted.c_str(), formatted.size(), resp->GetAllocator()).Move(),
      resp->GetAllocator());

  resp->PushBack(edit, resp->GetAllocator());

  ///==========================================================

  return;
}
