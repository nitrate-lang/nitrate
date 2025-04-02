#include <lsp/core/RPC.hh>
#include <lsp/core/Server.hh>
#include <lsp/core/SyncFS.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <string>

using namespace ncc;
using namespace no3::lsp;

using DocVersion = int64_t;

void core::LSPScheduler::NotifyTextDocumentDidChange(const message::NotifyMessage& notif) {
  using namespace nlohmann;

  const auto& j = *notif;

  if (!j.contains("textDocument")) {
    Log << "Missing textDocument field in didChange notification";
    return;
  }

  if (!j["textDocument"].is_object()) {
    Log << "textDocument field in didChange notification is not an object";
    return;
  }

  let text_document = j["textDocument"];

  if (!text_document.contains("uri")) {
    Log << "Missing uri field in textDocument object";
    return;
  }

  if (!text_document["uri"].is_string()) {
    Log << "uri field in textDocument object is not a string";
    return;
  }

  if (!text_document.contains("version")) {
    Log << "Missing version field in textDocument object";
    return;
  }

  if (!text_document["version"].is_number_integer()) {
    Log << "version field in textDocument object is not an int64";
    return;
  }

  let uri = text_document["uri"].get<std::string>();
  let version = text_document["version"].get<int64_t>();

  if (!j.contains("contentChanges")) {
    Log << "Missing contentChanges field in didChange notification";
    return;
  }

  if (!j["contentChanges"].is_array()) {
    Log << "contentChanges field in didChange notification is not an array";
    return;
  }

  let content_changes = j["contentChanges"];

  auto file_opt = SyncFS::The().Open(uri);
  if (!file_opt.has_value()) {
    return;
  }

  auto file = file_opt.value();

  for (let content_change : content_changes) {
    if (!content_change.is_object()) {
      Log << "contentChange in contentChanges array is not an object";
      return;
    }

    if (!content_change.contains("text")) {
      Log << "Missing text field in contentChange object";
      return;
    }

    if (!content_change["text"].is_string()) {
      Log << "text field in contentChange object is not a string";
      return;
    }
  }

  { /** BEGIN: CRITICAL SECTION */
    static std::mutex mutex;
    static std::unordered_map<std::string, DocVersion> latest;

    std::lock_guard<std::mutex> lock(mutex);

    if (latest[uri] >= version) {
      Log << Info << "Discarding outdated document " << uri << " version " << version << " (latest is " << latest[uri]
          << ")";
      return;
    }

    for (let content_change : content_changes) {
      file->Replace(0, -1, content_change["text"].get<std::string>());
    }

    latest[uri] = version;
  } /** END: CRITICAL SECTION */
}
