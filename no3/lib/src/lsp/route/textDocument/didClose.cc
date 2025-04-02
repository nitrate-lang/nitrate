#include <lsp/core/RPC.hh>
#include <nitrate-core/Logger.hh>

using namespace ncc;
using namespace no3::lsp;

static auto VerifyTextDocumentDidClose(const nlohmann::json& j) -> bool {
  if (!j.is_object()) {
    return false;
  }

  if (!j.contains("textDocument") || !j["textDocument"].is_object()) {
    return false;
  }

  const auto& text_document = j["textDocument"];

  return text_document.contains("uri") && text_document["uri"].is_string();
}

void core::LSPScheduler::NotifyTextDocumentDidClose(const message::NotifyMessage& notif) {
  const auto& j = *notif;
  if (!VerifyTextDocumentDidClose(j)) {
    Log << "Invalid textDocument/didClose notification";
    return;
  }

  const auto& uri = j["textDocument"]["uri"].get<std::string>();
  if (!m_fs.DidClose(FlyString(uri))) {
    Log << "Failed to close text document: " << uri;
    return;
  }

  Log << Info << "Closed text document: " << uri;
}
