#include <rapidjson/document.h>

#include <core/SyncFS.hh>
#include <core/server.hh>
#include <string>

using namespace rapidjson;

static void do_didSave(const lsp::NotificationMessage& notif) {
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
  if (!uri.starts_with("file://")) {
    LOG(ERROR) << "uri is not a file";
    return;
  }
  uri = uri.substr(7);

  LOG(INFO) << "Saving file: " << uri;
}

ADD_NOTIFICATION_HANDLER("textDocument/didSave", do_didSave);
