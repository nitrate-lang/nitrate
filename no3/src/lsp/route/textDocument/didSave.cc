#include <lsp/core/SyncFS.hh>
#include <lsp/route/RoutesList.hh>

using namespace no3::lsp;

void srv::DoDidSave(const NotificationMessage& notif) {
  if (!notif.GetJSON().HasMember("textDocument")) {
    LOG(ERROR) << "Missing textDocument member";
    return;
  }

  if (!notif.GetJSON()["textDocument"].IsObject()) {
    LOG(ERROR) << "textDocument is not an object";
    return;
  }

  const auto& text_document = notif.GetJSON()["textDocument"];

  if (!text_document.HasMember("uri")) {
    LOG(ERROR) << "Missing uri member";
    return;
  }

  if (!text_document["uri"].IsString()) {
    LOG(ERROR) << "uri member is not a string";
    return;
  }

  std::string uri = text_document["uri"].GetString();

  LOG(INFO) << "Saving file: " << uri;
}
