#include <rapidjson/document.h>

#include <core/SyncFS.hh>
#include <core/server.hh>
#include <route/RoutesList.hh>
#include <string>

using namespace rapidjson;

void do_didSave(const lsp::NotificationMessage& notif) {
  if (!notif.params().HasMember("textDocument")) {
    LOG(ERROR) << "Missing textDocument member";
    return;
  }

  if (!notif.params()["textDocument"].IsObject()) {
    LOG(ERROR) << "textDocument is not an object";
    return;
  }

  const auto& text_document = notif.params()["textDocument"];

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
