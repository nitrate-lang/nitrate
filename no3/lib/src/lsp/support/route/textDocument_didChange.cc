////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///     .-----------------.    .----------------.     .----------------.     ///
///    | .--------------. |   | .--------------. |   | .--------------. |    ///
///    | | ____  _____  | |   | |     ____     | |   | |    ______    | |    ///
///    | ||_   _|_   _| | |   | |   .'    `.   | |   | |   / ____ `.  | |    ///
///    | |  |   \ | |   | |   | |  /  .--.  \  | |   | |   `'  __) |  | |    ///
///    | |  | |\ \| |   | |   | |  | |    | |  | |   | |   _  |__ '.  | |    ///
///    | | _| |_\   |_  | |   | |  \  `--'  /  | |   | |  | \____) |  | |    ///
///    | ||_____|\____| | |   | |   `.____.'   | |   | |   \______.'  | |    ///
///    | |              | |   | |              | |   | |              | |    ///
///    | '--------------' |   | '--------------' |   | '--------------' |    ///
///     '----------------'     '----------------'     '----------------'     ///
///                                                                          ///
///   * NITRATE TOOLCHAIN - The official toolchain for the Nitrate language. ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The Nitrate Toolchain is free software; you can redistribute it or     ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The Nitrate Toolcain is distributed in the hope that it will be        ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the Nitrate Toolchain; if not, see                  ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

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

  const auto content_changes_valid =
      std::all_of(j["contentChanges"].begin(), j["contentChanges"].end(), [](const auto& content_change) {
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

        return true;
      });

  return content_changes_valid;
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

  std::vector<protocol::TextDocumentContentChangeEvent> changes;

  {  // Prepare content changes
    changes.reserve(content_changes.size());

    for (const auto& content_change : content_changes) {
      protocol::TextDocumentContentChangeEvent change;
      change.m_range.m_start.m_line = content_change["range"]["start"]["line"].get<int64_t>();
      change.m_range.m_start.m_character = content_change["range"]["start"]["character"].get<int64_t>();
      change.m_range.m_end.m_line = content_change["range"]["end"]["line"].get<int64_t>();
      change.m_range.m_end.m_character = content_change["range"]["end"]["character"].get<int64_t>();
      change.m_text = content_change["text"].get<std::string>();
      changes.push_back(change);
    }
  }

  if (!m_fs.DidChanges(FlyString(uri), version, changes)) {
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
