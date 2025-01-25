#include <lsp/core/SyncFS.hh>
#include <lsp/route/RoutesList.hh>

using namespace no3::lsp;

void srv::DoDidSave(const NotificationMessage& notif) {
  if (!notif.GetJSON().contains("textDocument")) {
    LOG(ERROR) << "Missing textDocument member";
    return;
  }

  if (!notif.GetJSON()["textDocument"].is_object()) {
    LOG(ERROR) << "textDocument is not an object";
    return;
  }

  const auto& text_document = notif.GetJSON()["textDocument"];

  if (!text_document.contains("uri")) {
    LOG(ERROR) << "Missing uri member";
    return;
  }

  if (!text_document["uri"].is_string()) {
    LOG(ERROR) << "uri member is not a string";
    return;
  }
}
