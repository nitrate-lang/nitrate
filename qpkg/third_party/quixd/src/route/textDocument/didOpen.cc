#include <rapidjson/document.h>

#include <core/SyncFS.hh>
#include <core/server.hh>
#include <route/RoutesList.hh>
#include <string>

using namespace rapidjson;

void do_didOpen(const lsp::NotificationMessage& notif) {
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

  if (!text_document.HasMember("languageId")) {
    LOG(ERROR) << "Missing languageId member";
    return;
  }

  if (!text_document["languageId"].IsString()) {
    LOG(ERROR) << "languageId member is not a string";
    return;
  }

  if (!text_document.HasMember("version")) {
    LOG(ERROR) << "Missing version member";
    return;
  }

  if (!text_document["version"].IsInt64()) {
    LOG(ERROR) << "version member is not an integer";
    return;
  }

  if (!text_document.HasMember("text")) {
    LOG(ERROR) << "Missing text member";
    return;
  }

  if (!text_document["text"].IsString()) {
    LOG(ERROR) << "text member is not a string";
    return;
  }

  std::string uri = text_document["uri"].GetString();
  std::string language_id = text_document["languageId"].GetString();
  std::string text = text_document["text"].GetString();

  SyncFS::the().select_uri(uri);

  auto open_code = SyncFS::the().open(language_id);
  switch (open_code) {
    case SyncFS::OpenCode::OK: {
      break;
    }
    case SyncFS::OpenCode::NOT_FOUND: {
      LOG(ERROR) << "File not found: " << uri;
      return;
    }
    case SyncFS::OpenCode::ALREADY_OPEN: {
      LOG(ERROR) << "File already open: " << uri;
      return;
    }
    case SyncFS::OpenCode::OPEN_FAILED: {
      LOG(ERROR) << "Failed to open file: " << uri;
      return;
    }
  }

  auto replace_code = SyncFS::the().replace(0, -1, text);
  switch (replace_code) {
    case SyncFS::ReplaceCode::OK: {
      break;
    }
    case SyncFS::ReplaceCode::NOT_OPEN: {
      LOG(ERROR) << "File not open: " << uri;
      return;
    }
    case SyncFS::ReplaceCode::REPLACE_FAILED: {
      LOG(ERROR) << "Failed to replace file contents: " << uri;
      return;
    }
  }

  LOG(INFO) << "Opened file: " << uri;
}
