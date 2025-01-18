#include <lsp/core/Server.hh>
#include <lsp/core/SyncFS.hh>
#include <lsp/route/RoutesList.hh>
#include <nitrate-core/Macro.hh>
#include <string>

using namespace no3::lsp;

using DocVersion = int64_t;

void srv::DoDidChange(const NotificationMessage& notif) {
  using namespace nlohmann;

  if (!notif.GetJSON().contains("textDocument")) {
    LOG(ERROR) << "Missing textDocument field in didChange notification";
    return;
  }

  if (!notif.GetJSON()["textDocument"].is_object()) {
    LOG(ERROR)
        << "textDocument field in didChange notification is not an object";
    return;
  }

  let text_document = notif.GetJSON()["textDocument"];

  if (!text_document.contains("uri")) {
    LOG(ERROR) << "Missing uri field in textDocument object";
    return;
  }

  if (!text_document["uri"].is_string()) {
    LOG(ERROR) << "uri field in textDocument object is not a string";
    return;
  }

  if (!text_document.contains("version")) {
    LOG(ERROR) << "Missing version field in textDocument object";
    return;
  }

  if (!text_document["version"].is_number_integer()) {
    LOG(ERROR) << "version field in textDocument object is not an int64";
    return;
  }

  let uri = text_document["uri"].get<std::string>();
  let version = text_document["version"].get<int64_t>();

  if (!notif.GetJSON().contains("contentChanges")) {
    LOG(ERROR) << "Missing contentChanges field in didChange notification";
    return;
  }

  if (!notif.GetJSON()["contentChanges"].is_array()) {
    LOG(ERROR)
        << "contentChanges field in didChange notification is not an array";
    return;
  }

  let content_changes = notif.GetJSON()["contentChanges"];

  auto file_opt = SyncFS::The().Open(uri);
  if (!file_opt.has_value()) {
    return;
  }

  auto file = file_opt.value();

  for (let content_change : content_changes) {
    if (!content_change.is_object()) {
      LOG(ERROR) << "contentChange in contentChanges array is not an object";
      return;
    }

    if (!content_change.contains("text")) {
      LOG(ERROR) << "Missing text field in contentChange object";
      return;
    }

    if (!content_change["text"].is_string()) {
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
      file->Replace(0, -1, content_change["text"].get<std::string>());
    }

    latest[uri] = version;
  } /** END: CRITICAL SECTION */
}
