#include <rapidjson/document.h>

#include <lsp/core/SyncFS.hh>
#include <lsp/core/server.hh>
#include <lsp/route/RoutesList.hh>
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

  auto file_opt = SyncFS::the().open(uri);
  if (!file_opt.has_value()) {
    return;
  }

  auto file = file_opt.value();

  if (!file->replace(0, -1, text)) {
    LOG(ERROR) << "Failed to replace code";
    return;
  }

  LOG(INFO) << "File ready: " << uri;
}
