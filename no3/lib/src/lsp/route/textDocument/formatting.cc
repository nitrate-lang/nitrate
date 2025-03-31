#include <cctype>
#include <cstdint>
#include <format/tree/Visitor.hh>
#include <lsp/core/Server.hh>
#include <lsp/core/SyncFS.hh>
#include <lsp/route/RoutesList.hh>
#include <memory>
#include <nitrate-core/Environment.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-lexer/Scanner.hh>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/ASTExpr.hh>
#include <nitrate-parser/Context.hh>
#include <nitrate-seq/Sequencer.hh>
#include <sstream>
#include <string>

using namespace nlohmann;
using namespace ncc::lex;
using namespace ncc::seq;
using namespace no3::lsp;

void message::DoFormatting(const RequestMessage& req, ResponseMessage& resp) {
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

  if (!req.GetJSON().contains("textDocument")) {
    resp.Error(LSPStatus::InvalidParams, "Missing textDocument");
    return;
  }

  if (!req.GetJSON()["textDocument"].is_object()) {
    resp.Error(LSPStatus::InvalidParams, "textDocument is not an object");
    return;
  }

  if (!req.GetJSON()["textDocument"].contains("uri")) {
    resp.Error(LSPStatus::InvalidParams, "Missing textDocument.uri");
    return;
  }

  if (!req.GetJSON()["textDocument"]["uri"].is_string()) {
    resp.Error(LSPStatus::InvalidParams, "textDocument.uri is not a string");
    return;
  }

  if (!req.GetJSON().contains("options")) {
    resp.Error(LSPStatus::InvalidParams, "Missing options");
    return;
  }

  if (!req.GetJSON()["options"].is_object()) {
    resp.Error(LSPStatus::InvalidParams, "options is not an object");
    return;
  }

  if (!req.GetJSON()["options"].contains("tabSize")) {
    resp.Error(LSPStatus::InvalidParams, "Missing options.tabSize");
    return;
  }

  if (!req.GetJSON()["options"]["tabSize"].is_number_unsigned()) {
    resp.Error(LSPStatus::InvalidParams, "options.tabSize is not an integer");
    return;
  }

  if (req.GetJSON()["options"]["tabSize"].get<size_t>() == 0) {
    resp.Error(LSPStatus::InvalidParams, "options.tabSize is 0");
    return;
  }

  if (!req.GetJSON()["options"].contains("insertSpaces")) {
    resp.Error(LSPStatus::InvalidParams, "Missing options.insertSpaces");
    return;
  }

  if (!req.GetJSON()["options"]["insertSpaces"].get<bool>()) {
    resp.Error(LSPStatus::InvalidParams, "options.insertSpaces is not a boolean");
    return;
  }

  FormattingOptions options;
  options.m_tabSize = req.GetJSON()["options"]["tabSize"].get<size_t>();
  options.m_insertSpaces = req.GetJSON()["options"]["insertSpaces"].get<bool>();

  auto uri = req.GetJSON()["textDocument"]["uri"].get<std::string>();
  auto file_opt = SyncFS::The().Open(uri);
  if (!file_opt.has_value()) {
    resp.Error(LSPStatus::InternalError, "Failed to open file");
    return;
  }
  auto file = file_opt.value();

  std::stringstream ss(*file->Content());

  auto env = std::make_shared<ncc::Environment>();
  auto l = Sequencer(ss, env);
  auto pool = ncc::DynamicArena();

  /// FIXME: Get the import profile
  auto parser = ncc::parse::GeneralParser(l, env, pool);
  auto ast = parser.Parse();

  if (l.HasError() || !ast.Check()) {
    return;
  }

  std::stringstream formatted_ss;
  bool has_errors = false;
  auto formatter = no3::format::QuasiCanonicalFormatterFactory::Create(formatted_ss, has_errors);
  ast.Get()->Accept(*formatter);

  if (has_errors) {
    resp.Error(LSPStatus::InternalError, "Failed to format document");
    return;
  }

  auto formatted = formatted_ss.str();
  file->Replace(0, -1, formatted);

  ///==========================================================
  /// Send the whole new file contents

  resp.GetJSON() = json::array();

  auto edit = json::object();

  edit["range"]["start"]["line"] = 0;
  edit["range"]["start"]["character"] = 0;
  edit["range"]["end"]["line"] = SIZE_MAX;
  edit["range"]["end"]["character"] = SIZE_MAX;
  edit["newText"] = formatted;

  resp.GetJSON().push_back(edit);
}
