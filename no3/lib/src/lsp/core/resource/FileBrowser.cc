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

using namespace ncc;
using namespace no3::lsp::core;

class FileBrowser::PImpl {
public:
  std::mutex m_mutex;
  std::unordered_map<FlyString, std::shared_ptr<ConstFile>> m_files;
};

FileBrowser::FileBrowser(protocol::TextDocumentSyncKind) : m_impl(std::make_unique<PImpl>()) {}

FileBrowser::~FileBrowser() = default;

auto FileBrowser::DidOpen(FlyString file_uri, FileRevision revision, FlyString raw) -> bool {
  qcore_assert(m_impl != nullptr);
  std::lock_guard lock(m_impl->m_mutex);

  Log << Trace << "FileBrowser::DidOpen(" << file_uri << ", " << revision << ", " << raw->size() << " bytes)";

  const auto it = m_impl->m_files.find(file_uri);
  if (it != m_impl->m_files.end()) [[unlikely]] {
    Log << "FileBrowser::DidOpen: File already open: " << file_uri;
    return false;
  }

  Log << Trace << "FileBrowser::DidOpen: File not already open, opening: " << file_uri;

  m_impl->m_files[std::move(file_uri)] = std::make_shared<ConstFile>(file_uri, revision, raw);

  Log << Trace << "FileBrowser::DidOpen: File opened: " << file_uri;

  return true;
}

auto FileBrowser::DidChange(FlyString file_uri, FileRevision new_revision, FlyString raw) -> bool {
  qcore_assert(m_impl != nullptr);
  std::lock_guard lock(m_impl->m_mutex);

  Log << Trace << "FileBrowser::DidChange(" << file_uri << ", " << new_revision << ", " << raw->size() << " bytes)";

  const auto it = m_impl->m_files.find(file_uri);
  if (it == m_impl->m_files.end()) [[unlikely]] {
    Log << "FileBrowser::DidChange: File not found: " << file_uri;
    return false;
  }

  Log << Trace << "FileBrowser::DidChange: Found file: " << file_uri;

  const auto old_revision = it->second->GetRevision();
  it->second = std::make_shared<ConstFile>(file_uri, new_revision, raw);

  Log << Trace << "FileBrowser::DidChange: " << file_uri << " changed from revision " << old_revision << " to "
      << new_revision;

  return true;
}

static auto FromLCToOffset(std::string_view raw, uint64_t line, uint64_t column) -> std::optional<uint64_t> {
  uint64_t target_offset = 0;

  {  // Skip until the target line, else return std::nullopt if EOF
    uint64_t current_line = 0;

    while (true) {
      if (current_line == line) [[unlikely]] {
        break;
      } else if (target_offset >= raw.size()) [[unlikely]] {
        Log << "FromLCToOffset: Offset is out of bounds";
        return std::nullopt;
      }

      if (raw[target_offset++] == '\n') {
        ++current_line;
      }
    }

    // target_offset: is pointing to the first byte after the line break
    qcore_assert(current_line == line);
  }

  uint64_t line_length = 0;
  for (auto i = target_offset; i < raw.size() && raw[i] != '\n'; ++i) {
    ++line_length;
  }

  // The LSP protocol said we have to
  if (column > line_length) [[unlikely]] {
    Log << Trace << "FromLCToOffset: Clipping column (" << column << ") to line length (" << line_length << ")";
    column = line_length;
  }

  target_offset += column;

  if (target_offset > raw.size()) [[unlikely]] {
    Log << "FromLCToOffset: Offset is out of bounds";
    return std::nullopt;
  }

  /// TODO: Verify this is correct

  return target_offset;
}

auto FileBrowser::DidChanges(FlyString file_uri, FileRevision new_revision, IncrementalChanges changes) -> bool {
  qcore_assert(m_impl != nullptr);
  std::lock_guard lock(m_impl->m_mutex);

  Log << Trace << "FileBrowser::DidChange(" << file_uri << ", " << new_revision << ", " << changes.size()
      << " changes)";

  const auto it = m_impl->m_files.find(file_uri);
  if (it == m_impl->m_files.end()) [[unlikely]] {
    Log << "FileBrowser::DidChange: File not found: " << file_uri;
    return false;
  }
  Log << Trace << "FileBrowser::DidChange: Found file: " << file_uri;

  std::string state = it->second->ReadAll();

  for (size_t i = 0; i < changes.size(); ++i) {
    const auto& [range, new_content] = changes[i];
    auto [start_line, start_character] = range.m_start_inclusive;
    auto [end_line_ex, end_character_ex] = range.m_end_exclusive;

    start_character /= 2;   // LSP uses UTF-16, so divide by 2 to get byte offset
    end_character_ex /= 2;  // LSP uses UTF-16, so divide by 2 to get byte offset

    const auto start_offset = FromLCToOffset(state, start_line, start_character);
    if (!start_offset) {
      Log << "FileBrowser::DidChange: Failed to convert start line/column to offset";
      return false;
    }

    const auto end_offset_plus_one = FromLCToOffset(state, end_line_ex, end_character_ex);
    if (!end_offset_plus_one) {
      Log << "FileBrowser::DidChange: Failed to convert end line/column to offset";
      return false;
    }

    Log << Trace << "FileBrowser::DidChange: Change #" << i << ", Range: (l:" << start_line << ", c:" << start_character
        << ", o:" << *start_offset << ") - (l:" << end_line_ex << ", c:" << end_character_ex
        << ", o:" << *end_offset_plus_one << ")";

    const auto n = *end_offset_plus_one - *start_offset;

    if (*start_offset >= state.size()) {
      Log << "FileBrowser::DidChange: Start offset is out of bounds";
      return false;
    }

    if (n > state.size()) {
      Log << "FileBrowser::DidChange: End offset is out of bounds";
      return false;
    }

    state.replace(*start_offset, n, *new_content);
    Log << Trace << "FileBrowser::DidChange: Change #" << i << " applied to temporary state";
  }

  Log << Trace << "FileBrowser::DidChange: Flushing " << changes.size() << " changes to file: " << file_uri;
  it->second = std::make_shared<ConstFile>(file_uri, new_revision, FlyString(state));
  Log << Trace << "FileBrowser::DidChange: File changed: " << file_uri << " to revision " << new_revision;

  return true;
}

auto FileBrowser::DidSave(const FlyString& file_uri) -> bool {
  qcore_assert(m_impl != nullptr);
  std::lock_guard lock(m_impl->m_mutex);

  Log << Trace << "FileBrowser::DidSave(" << file_uri << ")";
  // Nothing to do

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

  Log << Trace << "FileBrowser::DidClose: Found file: " << file_uri;
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
