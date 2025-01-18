#include <lsp/core/SyncFS.hh>
#include <lsp/route/RoutesList.hh>

using namespace no3::lsp;

void srv::DoDidClose(const NotificationMessage& notif) {
  if (!notif.GetJSON().contains("textDocument")) {
    LOG(ERROR) << "Missing textDocument member";
    return;
  }

  if (!notif.GetJSON()["textDocument"].is_object()) {
    LOG(ERROR) << "textDocument is not an object";
    return;
  }

  auto text_document = notif.GetJSON()["textDocument"];

  if (!text_document.contains("uri")) {
    LOG(ERROR) << "Missing uri member";
    return;
  }

  if (!text_document["uri"].is_string()) {
    LOG(ERROR) << "uri member is not a string";
    return;
  }

  auto uri = text_document["uri"].get<std::string>();

  SyncFS::The().Close(uri);
}
