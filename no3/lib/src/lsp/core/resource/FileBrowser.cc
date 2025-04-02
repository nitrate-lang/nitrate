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

#include <lsp/core/resource/FileBrowser.hh>
#include <memory>
#include <nitrate-core/Assert.hh>
#include <nitrate-core/Logger.hh>

#include "lsp/core/protocol/Base.hh"

using namespace ncc;
using namespace no3::lsp::core;

class FileBrowser::PImpl {
public:
  std::mutex m_mutex;
  std::unordered_map<FlyString, std::shared_ptr<ConstFile>> m_files;
};

FileBrowser::FileBrowser(protocol::TextDocumentSyncKind) : m_impl(std::make_unique<PImpl>()) {}

FileBrowser::~FileBrowser() = default;

auto FileBrowser::DidOpen(const FlyString& file_uri, FileVersion version, FlyString raw) -> bool {
  qcore_assert(m_impl != nullptr);
  std::lock_guard lock(m_impl->m_mutex);

  Log << Trace << "FileBrowser::DidOpen(" << file_uri << ", " << version << ", " << raw->size() << " bytes)";

  const auto it = m_impl->m_files.find(file_uri);
  if (it != m_impl->m_files.end()) [[unlikely]] {
    Log << "FileBrowser::DidOpen: File already open: " << file_uri;
    return false;
  }

  Log << Trace << "FileBrowser::DidOpen: File not already open, opening: " << file_uri;

  m_impl->m_files[file_uri] = std::make_shared<ConstFile>(file_uri, version, raw);

  Log << Trace << "FileBrowser::DidOpen: File opened: " << file_uri;

  return true;
}

auto FileBrowser::DidChange(const FlyString& file_uri, FileVersion version, FlyString raw) -> bool {
  qcore_assert(m_impl != nullptr);
  std::lock_guard lock(m_impl->m_mutex);

  Log << Trace << "FileBrowser::DidChange(" << file_uri << ", " << version << ", " << raw->size() << " bytes)";

  const auto it = m_impl->m_files.find(file_uri);
  if (it == m_impl->m_files.end()) [[unlikely]] {
    Log << "FileBrowser::DidChange: File not found: " << file_uri;
    return false;
  }

  const auto old_version = it->second->GetVersion();
  it->second = std::make_shared<ConstFile>(file_uri, version, raw);

  Log << Trace << "FileBrowser::DidChange: " << file_uri << " changed from version " << old_version << " to "
      << version;

  return true;
}

static auto FromLCToOffsetUsingUTF16(const std::string_view utf8_bytes, uint64_t line,
                                     uint64_t column) -> std::optional<uint64_t> {
  uint64_t raw_offset = 0;
  const auto utf8_bytes_size = utf8_bytes.size();

  {  // Skip until the target line, else return std::nullopt if EOF
    uint64_t current_line = 0;

    while (true) {
      if (current_line == line) [[unlikely]] {
        break;
      } else if (raw_offset >= utf8_bytes_size) [[unlikely]] {
        Log << "FromLCToOffset: Offset is out of bounds";
        return std::nullopt;
      }

      /**
       * @ref https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocuments
       *
       * export const EOL: string[] = ['\n', '\r\n', '\r'];
       */

      auto ch = utf8_bytes[raw_offset++];
      bool is_newline = false;

      if (ch == '\r') {
        is_newline = true;
        if (raw_offset < utf8_bytes_size && utf8_bytes[raw_offset] == '\n') {
          ++raw_offset;
        }
      } else if (ch == '\n') {
        is_newline = true;
      }

      current_line += static_cast<uint64_t>(is_newline);
    }

    // raw_byte_offset: is pointing to the first byte after the line break
    qcore_assert(current_line == line);
  }

  uint64_t line_length = 0;
  for (auto i = raw_offset; i < utf8_bytes.size() && utf8_bytes[i] != '\n'; ++i) {
    ++line_length;
  }

  // The LSP protocol said we have to
  if (column > line_length) [[unlikely]] {
    Log << Trace << "FromLCToOffset: Clipping column (" << column << ") to line length (" << line_length << ")";
    column = line_length;
  }

  /// TODO: Handle UTF-16 offsets

  raw_offset += column;
  if (raw_offset > utf8_bytes.size()) [[unlikely]] {
    Log << "FromLCToOffset: Offset is out of bounds";
    return std::nullopt;
  }

  return raw_offset;
}

auto FileBrowser::DidChanges(const FlyString& file_uri, FileVersion version, IncrementalChanges changes) -> bool {
  qcore_assert(m_impl != nullptr);
  std::lock_guard lock(m_impl->m_mutex);

  Log << Trace << "FileBrowser::DidChange(" << file_uri << ", " << version << ", " << changes.size() << " changes)";

  const auto it = m_impl->m_files.find(file_uri);
  if (it == m_impl->m_files.end()) [[unlikely]] {
    Log << "FileBrowser::DidChange: File not found: " << file_uri;
    return false;
  }

  std::string state = it->second->ReadAll();

  for (size_t i = 0; i < changes.size(); ++i) {
    const auto& [range, new_content] = changes[i];
    auto [start_line, start_character] = range.m_start;
    auto [end_line_ex, end_character_ex] = range.m_end;

    const auto start_offset = FromLCToOffsetUsingUTF16(state, start_line, start_character);
    if (!start_offset) {
      Log << "FileBrowser::DidChange: Failed to convert start line/column to offset";
      return false;
    }

    const auto end_offset_plus_one = FromLCToOffsetUsingUTF16(state, end_line_ex, end_character_ex);
    if (!end_offset_plus_one) {
      Log << "FileBrowser::DidChange: Failed to convert end line/column to offset";
      return false;
    }

    Log << Trace << "FileBrowser::DidChange: Change #" << i << ", Range: (l:" << start_line << ", c:" << start_character
        << ", o:" << *start_offset << ") - (l:" << end_line_ex << ", c:" << end_character_ex
        << ", o:" << *end_offset_plus_one << ")";

    const auto n = *end_offset_plus_one - *start_offset;
    if (*start_offset > state.size()) {
      Log << "FileBrowser::DidChange: Start offset is out of bounds: " << *start_offset << " > " << state.size();
      return false;
    }

    if (n > state.size()) {
      Log << "FileBrowser::DidChange: End offset is out of bounds: " << n << " > " << state.size();
      return false;
    }

    state.replace(*start_offset, n, *new_content);
    Log << Trace << "FileBrowser::DidChange: Change #" << i << " applied to temporary state";
  }

  Log << Trace << "FileBrowser::DidChange: Flushing " << changes.size() << " changes to file: " << file_uri;
  it->second = std::make_shared<ConstFile>(file_uri, version, FlyString(state));
  Log << Trace << "FileBrowser::DidChange: File changed: " << file_uri << " to version " << version;

  return true;
}

auto FileBrowser::DidSave(const FlyString& file_uri, std::optional<FlyString> full_content) -> bool {
  qcore_assert(m_impl != nullptr);
  std::lock_guard lock(m_impl->m_mutex);

  Log << Trace << "FileBrowser::DidSave(" << file_uri << ")";

  const auto it = m_impl->m_files.find(file_uri);
  if (it == m_impl->m_files.end()) [[unlikely]] {
    Log << Warning << "FileBrowser::DidSave: File not open: " << file_uri;
    return true;
  }

  if (full_content) {
    Log << Trace << "FileBrowser::DidSave: Saving file: " << file_uri << ", size: " << full_content.value()->size()
        << " bytes";
    it->second = std::make_shared<ConstFile>(file_uri, it->second->GetVersion(), *full_content);
  }

  return true;
}

auto FileBrowser::DidClose(const FlyString& file_uri) -> bool {
  qcore_assert(m_impl != nullptr);
  std::lock_guard lock(m_impl->m_mutex);

  Log << Trace << "FileBrowser::DidClose(" << file_uri << ")";
  const auto it = m_impl->m_files.find(file_uri);
  if (it == m_impl->m_files.end()) [[unlikely]] {
    Log << "FileBrowser::DidClose: File not found: " << file_uri;
    return false;
  }

  m_impl->m_files.erase(it);
  Log << Trace << "FileBrowser::DidClose: File closed: " << file_uri;

  return true;
}

auto FileBrowser::GetFile(const FlyString& file_uri) const -> std::optional<ReadOnlyFile> {
  qcore_assert(m_impl != nullptr);
  std::lock_guard lock(m_impl->m_mutex);

  Log << Trace << "FileBrowser::GetFile(" << file_uri << ")";

  const auto it = m_impl->m_files.find(file_uri);
  if (it == m_impl->m_files.end()) [[unlikely]] {
    Log << "FileBrowser::GetFile: File not found: " << file_uri;
    return std::nullopt;
  }

  Log << Trace << "FileBrowser::GetFile: Got file: " << file_uri;

  return it->second;
}
