#include <lsp/core/LSPContext.hh>
#include <nitrate-core/Logger.hh>

using namespace ncc;
using namespace no3::lsp;

static auto VerifyTextDocumentDidSave(const nlohmann::json& j) -> bool {
  if (!j.is_object()) {
    return false;
  }

  if (!j.contains("textDocument") || !j["textDocument"].is_object()) {
    return false;
  }

  const auto& text_document = j["textDocument"];

  return text_document.contains("uri") && text_document["uri"].is_string();
}

void core::LSPContext::NotifyTextDocumentDidSave(const message::NotifyMessage& notif) {
  const auto& j = *notif;
  if (!VerifyTextDocumentDidSave(j)) {
    Log << "Invalid textDocument/didSave notification";
    return;
  }

  const auto& uri = j["textDocument"]["uri"].get<std::string>();
  if (!m_fs.DidSave(FlyString(uri))) {
    Log << "Failed to save text document: " << uri;
    return;
  }

  Log << Info << "Saved text document: " << uri;
}
