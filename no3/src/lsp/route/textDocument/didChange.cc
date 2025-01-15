#include <rapidjson/allocators.h>
#include <rapidjson/document.h>

#include <lsp/core/SyncFS.hh>
#include <lsp/core/server.hh>
#include <lsp/route/RoutesList.hh>
#include <nitrate-core/Macro.hh>
#include <string>

typedef int64_t DocVersion;

void DoDidChange(const lsp::NotificationMessage& notif) {
  using namespace rapidjson;

  if (!notif.Params().HasMember("textDocument")) {
    LOG(ERROR) << "Missing textDocument field in didChange notification";
    return;
  }

  if (!notif.Params()["textDocument"].IsObject()) {
    LOG(ERROR)
        << "textDocument field in didChange notification is not an object";
    return;
  }

  let text_document = notif.Params()["textDocument"];

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

  let uri = text_document["uri"].GetString();
  let version = text_document["version"].GetInt64();

  if (!notif.Params().HasMember("contentChanges")) {
    LOG(ERROR) << "Missing contentChanges field in didChange notification";
    return;
  }

  if (!notif.Params()["contentChanges"].IsArray()) {
    LOG(ERROR)
        << "contentChanges field in didChange notification is not an array";
    return;
  }

  let content_changes = notif.Params()["contentChanges"].GetArray();

  auto file_opt = SyncFS::The().Open(uri);
  if (!file_opt.has_value()) {
    return;
  }

  auto file = file_opt.value();

  for (let content_change : content_changes) {
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
  }

  { /** BEGIN: CRITICAL SECTION */
    static std::mutex mutex;
    static std::unordered_map<std::string, DocVersion> latest;

    std::lock_guard<std::mutex> lock(mutex);

    if (latest[uri] >= version) {
      LOG(INFO) << "Discarding outdated document " << uri << " version "
                << version << " (latest is " << latest[uri] << ")";
      return;
    }

    for (let content_change : content_changes) {
      std::string_view text(content_change["text"].GetString(),
                            content_change["text"].GetStringLength());

      file->Replace(0, -1, text);
    }

    latest[uri] = version;

    LOG(INFO) << "Document " << uri << " updated to version " << version;
  } /** END: CRITICAL SECTION */
}