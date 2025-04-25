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

#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <istream>
#include <lsp/protocol/Base.hh>
#include <lsp/resource/File.hh>
#include <memory>
#include <nitrate-core/Assert.hh>
#include <nitrate-core/Logger.hh>
#include <string_view>

using namespace ncc;
using namespace no3::lsp::core;

class ConstFile::PImpl {
public:
  FlyString m_file_uri;
  FlyByteString m_raw;
  FileVersion m_version;

  PImpl(FlyString file_uri, FileVersion version, FlyByteString raw)
      : m_file_uri(std::move(file_uri)), m_raw(std::move(raw)), m_version(version) {}
  PImpl(const PImpl &) = delete;
};

ConstFile::ConstFile(FlyString file_uri, FileVersion version, FlyByteString raw)
    : m_impl(std::make_unique<PImpl>(std::move(file_uri), version, std::move(raw))) {}

ConstFile::~ConstFile() = default;

auto ConstFile::GetVersion() const -> FileVersion {
  qcore_assert(m_impl != nullptr);
  return m_impl->m_version;
}

auto ConstFile::GetURI() const -> FlyString {
  qcore_assert(m_impl != nullptr);
  return m_impl->m_file_uri;
}

auto ConstFile::GetFileSizeInBytes() const -> std::streamsize {
  qcore_assert(m_impl != nullptr);
  return m_impl->m_raw->size();
}

auto ConstFile::GetFileSizeInKiloBytes() const -> std::streamsize { return GetFileSizeInBytes() / 1000; }
auto ConstFile::GetFileSizeInMegaBytes() const -> std::streamsize { return GetFileSizeInKiloBytes() / 1000; }
auto ConstFile::GetFileSizeInGigaBytes() const -> std::streamsize { return GetFileSizeInMegaBytes() / 1000; }

auto ConstFile::ReadAll() const -> FlyByteString {
  qcore_assert(m_impl != nullptr);
  return m_impl->m_raw;
}

auto ConstFile::GetReader() const -> std::unique_ptr<std::basic_istream<uint8_t>> {
  qcore_assert(m_impl != nullptr);
  const auto &data = m_impl->m_raw;

  return std::make_unique<boost::iostreams::stream<boost::iostreams::basic_array_source<uint8_t>>>(data->data(),
                                                                                                   data->size());
}

struct UnicodeResult {
  uint8_t m_count;
  uint8_t m_size;
};

static auto UTF8ToUTF16CharacterCount(std::basic_string_view<uint8_t> utf8_bytes) -> std::optional<UnicodeResult> {
  std::array<uint8_t, 4> utf8_buf;
  uint32_t codepoint = 0;
  uint8_t codepoint_size = 0;

  utf8_buf.fill(0);
  std::memcpy(utf8_buf.data(), utf8_bytes.data(), utf8_bytes.size() < 4 ? utf8_bytes.size() : 4);

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

auto ConstFile::GetOffset(std::basic_string_view<uint8_t> raw, uint64_t line,
                          uint64_t column) -> std::optional<uint64_t> {
  uint64_t raw_offset = 0;
  const auto utf8_bytes_size = raw.size();

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

      const auto ch = raw[raw_offset++];
      if (ch == '\r') {
        if (raw_offset < utf8_bytes_size && raw[raw_offset] == '\n') {
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
  for (auto &i = raw_offset; i < raw.size() && (raw[i] != '\n' && raw[i] != '\r'); ++i) {
    if (column == utf16_line_pos) {
      return raw_offset;
    }

    if (auto res = UTF8ToUTF16CharacterCount(raw.substr(i))) {
      utf16_line_pos += res->m_count;
      i += res->m_size - 1;
    }
  }

  if (raw_offset == raw.size() && column != utf16_line_pos) {
    Log << Trace << "ConvertUTF16LCToOffset: Clipping UTF-16 column (" << column << ") to line length ("
        << utf16_line_pos << ")";
  }

  qcore_assert(raw_offset <= raw.size());

  return raw_offset;
}

auto ConstFile::GetLC(std::basic_string_view<uint8_t> raw,
                      uint64_t offset) -> std::optional<std::pair<uint64_t, uint64_t>> {
  uint64_t line = 0;
  uint64_t column = 0;
  uint64_t i = 0;

  for (; i < offset && i < raw.size(); ++i) {
    const auto ch = raw[i];
    if (ch == '\r') {
      if (i + 1 < raw.size() && raw[i + 1] == '\n') {
        ++i;
      }

      ++line;
      column = 0;
    } else if (ch == '\n') {
      ++line;
      column = 0;
    } else {
      ++column;
    }
  }

  if (i != offset) [[unlikely]] {
    Log << Trace << "ConvertUTF16OffsetToLC: Offset is out of bounds";
    return std::nullopt;
  }

  return std::make_pair(line, column);
}

auto ConstFile::GetOffset(uint64_t line, uint64_t column) -> std::optional<uint64_t> {
  qcore_assert(m_impl != nullptr);

  const auto &raw = m_impl->m_raw;
  const auto utf8_bytes = std::basic_string_view<uint8_t>(raw->data(), raw->size());

  return GetOffset(utf8_bytes, line, column);
}

auto ConstFile::GetLC(uint64_t offset) -> std::optional<std::pair<uint64_t, uint64_t>> {
  qcore_assert(m_impl != nullptr);

  const auto &raw = m_impl->m_raw;
  const auto utf8_bytes = std::basic_string_view<uint8_t>(raw->data(), raw->size());

  return GetLC(utf8_bytes, offset);
}
