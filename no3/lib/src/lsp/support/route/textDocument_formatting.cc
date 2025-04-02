// #include <cctype>
// #include <cstdint>
// #include <format/tree/Visitor.hh>
// #include <lsp/core/Server.hh>
// #include <lsp/core/SyncFS.hh>
// #include <lsp/core/LSPContext.hh>
// #include <memory>
// #include <nitrate-core/Environment.hh>
// #include <nitrate-core/Logger.hh>
// #include <nitrate-lexer/Scanner.hh>
// #include <nitrate-parser/AST.hh>
// #include <nitrate-parser/ASTExpr.hh>
// #include <nitrate-parser/Context.hh>
// #include <nitrate-seq/Sequencer.hh>
// #include <sstream>
// #include <string>

// using namespace nlohmann;
// using namespace ncc::lex;
// using namespace ncc::seq;
// using namespace no3::lsp;

// void rpc::DoFormatting(const RequestMessage& req, ResponseMessage& resp) {
//   struct Position {
//     size_t m_line = 0;
//     size_t m_character = 0;
//   };

//   struct Range {
//     Position m_start;
//     Position m_end;
//   };

//   struct FormattingOptions {
//     size_t m_tabSize = 0;
//     bool m_insertSpaces = false;
//   };

//   if (!req.contains("textDocument")) {
//     resp.SetStatusCode(StatusCode::InvalidParams);
//     return;
//   }

//   if (!req["textDocument"].is_object()) {
//     resp.SetStatusCode(StatusCode::InvalidParams);
//     return;
//   }

//   if (!req["textDocument"].contains("uri")) {
//     resp.SetStatusCode(StatusCode::InvalidParams);
//     return;
//   }

//   if (!req["textDocument"]["uri"].is_string()) {
//     resp.SetStatusCode(StatusCode::InvalidParams);
//     return;
//   }

//   if (!req.contains("options")) {
//     resp.SetStatusCode(StatusCode::InvalidParams);
//     return;
//   }

//   if (!req["options"].is_object()) {
//     resp.SetStatusCode(StatusCode::InvalidParams);
//     return;
//   }

//   if (!req["options"].contains("tabSize")) {
//     resp.SetStatusCode(StatusCode::InvalidParams);
//     return;
//   }

//   if (!req["options"]["tabSize"].is_number_unsigned()) {
//     resp.SetStatusCode(StatusCode::InvalidParams);
//     return;
//   }

//   if (req["options"]["tabSize"].get<size_t>() == 0) {
//     resp.SetStatusCode(StatusCode::InvalidParams);
//     return;
//   }

//   if (!req["options"].contains("insertSpaces")) {
//     resp.SetStatusCode(StatusCode::InvalidParams);
//     return;
//   }

//   if (!req["options"]["insertSpaces"].get<bool>()) {
//     resp.SetStatusCode(StatusCode::InvalidParams);
//     return;
//   }

//   FormattingOptions options;
//   options.m_tabSize = req["options"]["tabSize"].get<size_t>();
//   options.m_insertSpaces = req["options"]["insertSpaces"].get<bool>();

//   auto uri = req["textDocument"]["uri"].get<std::string>();
//   auto file_opt = SyncFS::The().Open(uri);
//   if (!file_opt.has_value()) {
//     resp.SetStatusCode(StatusCode::InternalError);
//     return;
//   }
//   auto file = file_opt.value();

//   std::stringstream ss(*file->Content());

//   auto env = std::make_shared<ncc::Environment>();
//   auto l = Sequencer(ss, env);
//   auto pool = ncc::DynamicArena();

//   /// FIXME: Get the import profile
//   auto parser = ncc::parse::GeneralParser(l, env, pool);
//   auto ast = parser.Parse();

//   if (l.HasError() || !ast.Check()) {
//     return;
//   }

//   std::stringstream formatted_ss;
//   bool has_errors = false;
//   auto formatter = no3::format::QuasiCanonicalFormatterFactory::Create(formatted_ss, has_errors);
//   ast.Get()->Accept(*formatter);

//   if (has_errors) {
//     resp.SetStatusCode(StatusCode::InternalError);
//     return;
//   }

//   auto formatted = formatted_ss.str();
//   file->Replace(0, -1, formatted);

//   ///==========================================================
//   /// Send the whole new file contents

//   resp = json::array();

//   auto edit = json::object();

//   edit["range"]["start"]["line"] = 0;
//   edit["range"]["start"]["character"] = 0;
//   edit["range"]["end"]["line"] = SIZE_MAX;
//   edit["range"]["end"]["character"] = SIZE_MAX;
//   edit["newText"] = formatted;

//   resp.push_back(edit);
// }
