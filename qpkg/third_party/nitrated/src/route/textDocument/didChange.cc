#include <rapidjson/allocators.h>
#include <rapidjson/document.h>

#include <core/SyncFS.hh>
#include <core/server.hh>
#include <route/RoutesList.hh>
#include <string>

using namespace rapidjson;

typedef int64_t DocVersion;

struct Change {
  std::string text;
};

void do_didChange(const lsp::NotificationMessage& notif) {
  if (!notif.params().HasMember("textDocument")) {
    LOG(ERROR) << "Missing textDocument field in didChange notification";
    return;
  }

  if (!notif.params()["textDocument"].IsObject()) {
    LOG(ERROR) << "textDocument field in didChange notification is not an object";
    return;
  }

  const auto& text_document = notif.params()["textDocument"];

  if (!text_document.HasMember("uri")) {
    LOG(ERROR) << "Missing uri field in textDocument object";
    return;
  }

  if (!text_document["uri"].IsString()) {
    LOG(ERROR) << "uri field in textDocument object is not a string";
    return;
  }

  if (!text_document.HasMember("version")) {
    LOG(ERROR) << "Missing version field in textDocument object";
    return;
  }

  if (!text_document["version"].IsInt64()) {
    LOG(ERROR) << "version field in textDocument object is not an int64";
    return;
  }

  std::string uri = text_document["uri"].GetString();
  DocVersion version = text_document["version"].GetInt64();

  if (!notif.params().HasMember("contentChanges")) {
    LOG(ERROR) << "Missing contentChanges field in didChange notification";
    return;
  }

  if (!notif.params()["contentChanges"].IsArray()) {
    LOG(ERROR) << "contentChanges field in didChange notification is not an array";
    return;
  }

  const auto& content_changes = notif.params()["contentChanges"].GetArray();

  static std::mutex m;
  std::lock_guard lock(m);
  static std::unordered_map<std::string, DocVersion> latest;

  if (latest[uri] > version) {
    return;
  }

  SyncFS::the().select_uri(uri);

  for (const auto& content_change : content_changes) {
    if (!content_change.IsObject()) {
      LOG(ERROR) << "contentChange in contentChanges array is not an object";
      return;
    }

    if (!content_change.HasMember("text")) {
      LOG(ERROR) << "Missing text field in contentChange object";
      return;
    }

    if (!content_change["text"].IsString()) {
      LOG(ERROR) << "text field in contentChange object is not a string";
      return;
    }

    std::string_view text(content_change["text"].GetString(),
                          content_change["text"].GetStringLength());

    SyncFS::the().replace(0, SyncFS::the().size().value(), text);
  }

  latest[uri] = version;
}
