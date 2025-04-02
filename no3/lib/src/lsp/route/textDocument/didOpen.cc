#include <lsp/core/RPC.hh>
#include <nitrate-core/Logger.hh>

using namespace ncc;
using namespace no3::lsp;

static auto VerifyTextDocumentDidOpen(const nlohmann::json& j) -> bool {
  if (!j.is_object()) {
    return false;
  }

  if (!j.contains("textDocument") || !j["textDocument"].is_object()) {
    return false;
  }

  const auto& text_document = j["textDocument"];

  if (!text_document.contains("uri") || !text_document["uri"].is_string()) {
    return false;
  }

  if (!text_document.contains("version") || !text_document["version"].is_number_integer()) {
    return false;
  }

  if (!text_document.contains("text") || !text_document["text"].is_string()) {
    return false;
  }

  return true;
}

void core::LSPScheduler::NotifyTextDocumentDidOpen(const message::NotifyMessage& notif) {
  const auto& j = *notif;
  if (!VerifyTextDocumentDidOpen(j)) {
    Log << "Invalid textDocument/didOpen notification";
    return;
  }

  const auto uri = j["textDocument"]["uri"].get<std::string>();
  const auto version = j["textDocument"]["version"].get<int64_t>();
  const auto text = j["textDocument"]["text"].get<std::string>();
  const auto status = m_fs.DidOpen(FlyString(uri), version, FlyString(text));

  if (!status) {
    Log << "Failed to open text document: " << uri;
    return;
  }

  Log << Info << "Opened text document: " << uri;
}
