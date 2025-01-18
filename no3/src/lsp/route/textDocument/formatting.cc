#include <rapidjson/document.h>

#include <cctype>
#include <cstdint>
#include <lsp/core/SyncFS.hh>
#include <lsp/core/server.hh>
#include <lsp/lang/Format.hh>
#include <lsp/route/RoutesList.hh>
#include <memory>
#include <nitrate-core/Environment.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-lexer/Scanner.hh>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/Context.hh>
#include <nitrate-seq/Sequencer.hh>
#include <sstream>
#include <string>

using namespace rapidjson;
using namespace ncc::lex;
using namespace ncc::seq;
using namespace no3::lsp;

void srv::DoFormatting(const RequestMessage& req, ResponseMessage& resp) {
  struct Position {
    size_t m_line = 0;
    size_t m_character = 0;
  };

  struct Range {
    Position m_start;
    Position m_end;
  };

  struct FormattingOptions {
    size_t m_tabSize = 0;
    bool m_insertSpaces = false;
  };

  if (!req.GetJSON().HasMember("textDocument")) {
    resp.Error(ErrorCodes::InvalidParams, "Missing textDocument");
    return;
  }

  if (!req.GetJSON()["textDocument"].IsObject()) {
    resp.Error(ErrorCodes::InvalidParams, "textDocument is not an object");
    return;
  }

  if (!req.GetJSON()["textDocument"].HasMember("uri")) {
    resp.Error(ErrorCodes::InvalidParams, "Missing textDocument.uri");
    return;
  }

  if (!req.GetJSON()["textDocument"]["uri"].IsString()) {
    resp.Error(ErrorCodes::InvalidParams, "textDocument.uri is not a string");
    return;
  }

  if (!req.GetJSON().HasMember("options")) {
    resp.Error(ErrorCodes::InvalidParams, "Missing options");
    return;
  }

  if (!req.GetJSON()["options"].IsObject()) {
    resp.Error(ErrorCodes::InvalidParams, "options is not an object");
    return;
  }

  if (!req.GetJSON()["options"].HasMember("tabSize")) {
    resp.Error(ErrorCodes::InvalidParams, "Missing options.tabSize");
    return;
  }

  if (!req.GetJSON()["options"]["tabSize"].IsInt()) {
    resp.Error(ErrorCodes::InvalidParams, "options.tabSize is not an integer");
    return;
  }

  if (req.GetJSON()["options"]["tabSize"].GetUint() == 0) {
    resp.Error(ErrorCodes::InvalidParams, "options.tabSize is 0");
    return;
  }

  if (!req.GetJSON()["options"].HasMember("insertSpaces")) {
    resp.Error(ErrorCodes::InvalidParams, "Missing options.insertSpaces");
    return;
  }

  if (!req.GetJSON()["options"]["insertSpaces"].IsBool()) {
    resp.Error(ErrorCodes::InvalidParams,
               "options.insertSpaces is not a boolean");
    return;
  }

  FormattingOptions options;
  options.m_tabSize = req.GetJSON()["options"]["tabSize"].GetInt();
  options.m_insertSpaces = req.GetJSON()["options"]["insertSpaces"].GetBool();

  std::string uri = req.GetJSON()["textDocument"]["uri"].GetString();
  auto file_opt = SyncFS::The().Open(uri);
  if (!file_opt.has_value()) {
    resp.Error(ErrorCodes::InternalError, "Failed to open file");
    return;
  }
  auto file = file_opt.value();

  std::stringstream ss(*file->Content());

  auto env = std::make_shared<ncc::Environment>();
  auto l = Sequencer(ss, env);
  auto parser = ncc::parse::Parser::Create(l, env);
  auto ast = parser->Parse();

  if (l.HasError() || !ast.Check()) {
    return;
  }

  LOG(INFO) << "Requested document format";

  std::stringstream formatted_ss;
  if (!lsp::fmt::FormatterFactory::Create(lsp::fmt::Styleguide::Cambrian,
                                          formatted_ss)
           ->Format(ast.Get())) {
    resp.Error(ErrorCodes::InternalError, "Failed to format document");
    return;
  }

  auto formatted = formatted_ss.str();

  file->Replace(0, -1, formatted);

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
}
