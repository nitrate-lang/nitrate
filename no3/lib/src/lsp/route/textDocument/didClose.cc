#include <lsp/core/SyncFS.hh>
#include <lsp/route/RoutesList.hh>

using namespace no3::lsp;

void rpc::NotifyTextDocumentDidClose(const NotifyMessage& notif) {
  const auto& j = *notif;

  if (!j.contains("textDocument")) {
    Log << "Missing textDocument member";
    return;
  }

  if (!j["textDocument"].is_object()) {
    Log << "textDocument is not an object";
    return;
  }

  auto text_document = j["textDocument"];

  if (!text_document.contains("uri")) {
    Log << "Missing uri member";
    return;
  }

  if (!text_document["uri"].is_string()) {
    Log << "uri member is not a string";
    return;
  }

  auto uri = text_document["uri"].get<std::string>();

  SyncFS::The().Close(uri);
}
