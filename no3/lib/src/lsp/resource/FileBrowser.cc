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

#include <lsp/resource/FileBrowser.hh>
#include <memory>
#include <nitrate-core/Assert.hh>
#include <nitrate-core/Logger.hh>
#include <string>

using namespace ncc;
using namespace no3::lsp::core;

class FileBrowser::PImpl {
public:
  std::mutex m_mutex;
  std::unordered_map<FlyString, std::shared_ptr<ConstFile>> m_files;
};

FileBrowser::FileBrowser(protocol::TextDocumentSyncKind) : m_impl(std::make_unique<PImpl>()) {}

FileBrowser::~FileBrowser() = default;

static auto TransformUTF8ToLF(const FlyByteString& raw) -> FlyByteString {
  const auto& utf8_bytes = *raw;

  std::basic_string<uint8_t> result;
  result.reserve(utf8_bytes.size());

  for (uint64_t i = 0; i < utf8_bytes.size(); ++i) {
    const auto ch = utf8_bytes[i];

    if (ch == '\r') {
      if (i + 1 < utf8_bytes.size() && utf8_bytes[i + 1] == '\n') {
        ++i;
      }
      result.push_back('\n');
    } else {
      result.push_back(ch);
    }
  }

  return FlyByteString(std::move(result));
}

auto FileBrowser::DidOpen(const FlyString& file_uri, FileVersion version, FlyByteString raw) -> bool {
  qcore_assert(m_impl != nullptr);
  std::lock_guard lock(m_impl->m_mutex);

  Log << Trace << "FileBrowser::DidOpen(" << file_uri << ", " << version << ", " << raw->size() << " bytes)";

  const auto it = m_impl->m_files.find(file_uri);
  if (it != m_impl->m_files.end()) [[unlikely]] {
    Log << "FileBrowser::DidOpen: File already open: " << file_uri;
    return false;
  }

  Log << Trace << "FileBrowser::DidOpen: File not already open, opening: " << file_uri;

  m_impl->m_files[file_uri] = std::make_shared<ConstFile>(file_uri, version, TransformUTF8ToLF(raw));

  Log << Trace << "FileBrowser::DidOpen: File opened: " << file_uri;

  return true;
}

auto FileBrowser::DidChange(const FlyString& file_uri, FileVersion version, FlyByteString raw) -> bool {
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

auto FileBrowser::DidChanges(const FlyString& file_uri, FileVersion version, IncrementalChanges changes) -> bool {
  qcore_assert(m_impl != nullptr);
  std::lock_guard lock(m_impl->m_mutex);

  Log << Trace << "FileBrowser::DidChange(" << file_uri << ", " << version << ", " << changes.size() << " changes)";

  const auto it = m_impl->m_files.find(file_uri);
  if (it == m_impl->m_files.end()) [[unlikely]] {
    Log << "FileBrowser::DidChange: File not found: " << file_uri;
    return false;
  }

  std::basic_string<uint8_t> state = it->second->ReadAll();

  for (size_t i = 0; i < changes.size(); ++i) {
    const auto& [range, new_content] = changes[i];
    auto [start_line, start_character] = range.m_start;
    auto [end_line_ex, end_character_ex] = range.m_end;

    const auto start_offset = ConstFile::GetOffset(state, start_line, start_character);
    if (!start_offset) {
      Log << "FileBrowser::DidChange: Failed to convert start line/column to offset";
      return false;
    }

    const auto end_offset_plus_one = ConstFile::GetOffset(state, end_line_ex, end_character_ex);
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
  it->second = std::make_shared<ConstFile>(file_uri, version, FlyByteString(state));
  Log << Trace << "FileBrowser::DidChange: File changed: " << file_uri << " to version " << version;

  return true;
}

auto FileBrowser::DidSave(const FlyString& file_uri, std::optional<FlyByteString> full_content) -> bool {
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
