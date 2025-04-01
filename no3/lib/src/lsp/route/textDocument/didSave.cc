#include <lsp/core/SyncFS.hh>
#include <lsp/route/RoutesList.hh>

using namespace no3::lsp;

void rpc::DoDidSave(const NotifyMessage& notif) {
  if (!notif.contains("textDocument")) {
    Log << "Missing textDocument member";
    return;
  }

  if (!notif["textDocument"].is_object()) {
    Log << "textDocument is not an object";
    return;
  }

  const auto& text_document = notif["textDocument"];

  if (!text_document.contains("uri")) {
    Log << "Missing uri member";
    return;
  }

  if (!text_document["uri"].is_string()) {
    Log << "uri member is not a string";
    return;
  }
}
