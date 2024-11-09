#include <rapidjson/document.h>

#include <core/SyncFS.hh>
#include <core/server.hh>

using namespace rapidjson;

static void do_didClose(const lsp::NotificationMessage& notif) {
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
  SyncFS::the().select_uri(uri);
  auto close_code = SyncFS::the().close();

  switch (close_code) {
    case SyncFS::CloseCode::OK:
      break;
    case SyncFS::CloseCode::NOT_OPEN:
      LOG(ERROR) << "File is not open";
      break;
    case SyncFS::CloseCode::CLOSE_FAILED:
      LOG(ERROR) << "Failed to close file";
      break;
  }

  LOG(INFO) << "Closed file: " << uri;
}

ADD_NOTIFICATION_HANDLER("textDocument/didClose", do_didClose);
