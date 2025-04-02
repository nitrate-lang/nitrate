#include <lsp/core/LSPContext.hh>
#include <nitrate-core/Assert.hh>
#include <nitrate-core/Logger.hh>

using namespace ncc;
using namespace no3::lsp;

static auto VerifyTextDocumentDidChange(const nlohmann::json& j) -> bool {
  if (!j.is_object()) {
    return false;
  }

  if (!j.contains("textDocument") || !j["textDocument"].is_object()) {
    return false;
  }

  const auto& text_document = j["textDocument"];

  if (!text_document.contains("uri") || !text_document["uri"].is_string()) {
    return false;
  }

  if (!text_document.contains("version") || !text_document["version"].is_number_integer()) {
    return false;
  }

  if (!j.contains("contentChanges") || !j["contentChanges"].is_array()) {
    return false;
  }

  for (const auto& content_change : j["contentChanges"]) {
    if (!content_change.is_object()) {
      return false;
    }

    if (!content_change.contains("range") || !content_change["range"].is_object()) {
      return false;
    }

    const auto& range = content_change["range"];
    if (!range.contains("start") || !range["start"].is_object()) {
      return false;
    }
    if (!range.contains("end") || !range["end"].is_object()) {
      return false;
    }

    const auto& start = range["start"];
    if (!start.contains("line") || !start["line"].is_number_integer() || !start.contains("character") ||
        !start["character"].is_number_integer()) {
      return false;
    }

    const auto& end = range["end"];
    if (!end.contains("line") || !end["line"].is_number_integer() || !end.contains("character") ||
        !end["character"].is_number_integer()) {
      return false;
    }

    if (!content_change.contains("text") || !content_change["text"].is_string()) {
      return false;
    }
  }

  return true;
}

void core::LSPContext::NotifyTextDocumentDidChange(const message::NotifyMessage& notif) {
  const auto& j = *notif;
  if (!VerifyTextDocumentDidChange(j)) {
    Log << "Invalid textDocument/didChange notification";
    return;
  }

  const auto& uri = j["textDocument"]["uri"].get<std::string>();
  const auto& version = j["textDocument"]["version"].get<int64_t>();
  const auto& content_changes = j["contentChanges"];

  std::vector<protocol::TextDocumentContentChangeEvent> content_changes_vec;
  for (const auto& content_change : content_changes) {
    protocol::TextDocumentContentChangeEvent change;
    change.m_range.m_start_inclusive.m_line = content_change["range"]["start"]["line"].get<int64_t>();
    change.m_range.m_start_inclusive.m_character = content_change["range"]["start"]["character"].get<int64_t>();
    change.m_range.m_end_exclusive.m_line = content_change["range"]["end"]["line"].get<int64_t>();
    change.m_range.m_end_exclusive.m_character = content_change["range"]["end"]["character"].get<int64_t>();
    change.m_text = content_change["text"].get<std::string>();
    content_changes_vec.push_back(change);
  }

  auto status = m_fs.DidChanges(FlyString(uri), version, content_changes_vec);
  if (!status) {
    Log << "Failed to apply changes to text document: " << uri;
    return;
  }

  Log << Info << "Applied changes to text document: " << uri;

  // {  /// TODO: Remove this part
  //   auto raw_content = *m_fs.GetFile(FlyString(uri)).value()->ReadAll();

  //   std::fstream debug_output("/tmp/nitrate_lsp_debug.txt", std::ios::out | std::ios::trunc | std::ios::binary);
  //   if (!debug_output) {
  //     qcore_panic("Failed to open debug output file");
  //   }

  //   debug_output << raw_content;
  // }
}
