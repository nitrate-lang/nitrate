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
#include <string>
#include <string_view>

using namespace ncc;
using namespace no3::lsp::core;

struct UnicodeResult {
  uint8_t m_count;
  uint8_t m_size;
};

static auto UTF8ToUTF16CharacterCount(const uint8_t* utf8_bytes,
                                      size_t utf8_bytes_size) -> std::optional<UnicodeResult> {
  std::array<uint8_t, 4> utf8_buf;
  uint32_t codepoint = 0;
  uint8_t codepoint_size = 0;

  utf8_buf.fill(0);
  std::memcpy(utf8_buf.data(), utf8_bytes, utf8_bytes_size < 4 ? utf8_bytes_size : 4);

  if ((utf8_buf[0] & 0x80) == 0) [[likely]] {
    codepoint = utf8_buf[0];
    codepoint_size = 1;
  } else if ((utf8_buf[0] & 0xE0) == 0xC0) {
    codepoint = (utf8_buf[0] & 0x1F) << 6 | (utf8_buf[1] & 0x3F);
    codepoint_size = 2;
  } else if ((utf8_buf[0] & 0xF0) == 0xE0) {
    codepoint = (utf8_buf[0] & 0x0F) << 12 | (utf8_buf[1] & 0x3F) << 6 | (utf8_buf[2] & 0x3F);
    codepoint_size = 3;
  } else if ((utf8_buf[0] & 0xF8) == 0xF0) {
    codepoint =
        (utf8_buf[0] & 0x07) << 18 | (utf8_buf[1] & 0x3F) << 12 | (utf8_buf[2] & 0x3F) << 6 | (utf8_buf[3] & 0x3F);
    codepoint_size = 4;
  } else {
    return std::nullopt;
  }

  // Can be stored in a single UTF-16 code unit
  if (codepoint < 0xD800 || (codepoint > 0xDFFF && codepoint < 0x10000)) {
    return {{1, codepoint_size}};
  }

  // Surrogate pair
  if (codepoint >= 0x10000 && codepoint <= 0x10FFFF) {
    return {{2, codepoint_size}};
  }

  return std::nullopt;
}

static auto ConvertUTF16LCToOffset(const std::basic_string_view<uint8_t> utf8_bytes, const uint64_t line,
                                   const uint64_t utf16_column) -> std::optional<uint64_t> {
  uint64_t raw_offset = 0;
  const auto utf8_bytes_size = utf8_bytes.size();

  {  // Skip until the target line, else return std::nullopt if EOF
    uint64_t current_line = 0;

    while (true) {
      if (current_line == line) {
        break;
      }

      if (raw_offset >= utf8_bytes_size) [[unlikely]] {
        Log << "ConvertUTF16LCToOffset: Offset is out of bounds";
        return std::nullopt;
      }

      /**
       * @ref https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocuments
       *
       * export const EOL: string[] = ['\n', '\r\n', '\r'];
       */

      const auto ch = utf8_bytes[raw_offset++];
      if (ch == '\r') {
        if (raw_offset < utf8_bytes_size && utf8_bytes[raw_offset] == '\n') {
          ++raw_offset;
        }

        ++current_line;
      } else if (ch == '\n') {
        ++current_line;
      }
    }

    // raw_byte_offset: is pointing to the first byte after the line break
    qcore_assert(current_line == line);
  }

  uint64_t utf16_line_pos = 0;
  for (auto& i = raw_offset; i < utf8_bytes.size() && (utf8_bytes[i] != '\n' && utf8_bytes[i] != '\r'); ++i) {
    if (utf16_column == utf16_line_pos) {
      return raw_offset;
    }

    const auto substr = utf8_bytes.substr(i);
    if (auto res = UTF8ToUTF16CharacterCount(substr.data(), substr.size())) {
      utf16_line_pos += res->m_count;
      i += res->m_size - 1;
    }
  }

  Log << Trace << "ConvertUTF16LCToOffset: Clipping UTF-16 column (" << utf16_column << ") to line length ("
      << utf16_line_pos << ")";

  qcore_assert(raw_offset <= utf8_bytes.size());

  return raw_offset;
}

class FileBrowser::PImpl {
public:
  std::mutex m_mutex;
  std::unordered_map<FlyString, std::shared_ptr<ConstFile>> m_files;
};

FileBrowser::FileBrowser(protocol::TextDocumentSyncKind) : m_impl(std::make_unique<PImpl>()) {}

FileBrowser::~FileBrowser() = default;

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

  m_impl->m_files[file_uri] = std::make_shared<ConstFile>(file_uri, version, raw);

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

    const auto start_offset = ConvertUTF16LCToOffset(state, start_line, start_character);
    if (!start_offset) {
      Log << "FileBrowser::DidChange: Failed to convert start line/column to offset";
      return false;
    }

    const auto end_offset_plus_one = ConvertUTF16LCToOffset(state, end_line_ex, end_character_ex);
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
