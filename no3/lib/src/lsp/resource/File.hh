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

#pragma once

#include <boost/flyweight.hpp>
#include <istream>
#include <lsp/protocol/Base.hh>
#include <memory>
#include <string_view>

namespace no3::lsp::core {
  using FileVersion = long;

  class ConstFile {
    class PImpl;
    std::unique_ptr<PImpl> m_impl;

  public:
    ConstFile(FlyString file_uri, FileVersion version, FlyByteString raw);
    ConstFile(const ConstFile&) = delete;
    ConstFile(ConstFile&&) = default;
    ConstFile& operator=(const ConstFile&) = delete;
    ConstFile& operator=(ConstFile&&) = default;
    ~ConstFile();

    [[nodiscard]] auto GetVersion() const -> FileVersion;
    [[nodiscard]] auto GetURI() const -> FlyString;
    [[nodiscard]] auto GetFileSizeInBytes() const -> std::streamsize;
    [[nodiscard]] auto GetFileSizeInKiloBytes() const -> std::streamsize;
    [[nodiscard]] auto GetFileSizeInMegaBytes() const -> std::streamsize;
    [[nodiscard]] auto GetFileSizeInGigaBytes() const -> std::streamsize;

    [[nodiscard]] auto ReadAll() const -> FlyByteString;
    [[nodiscard]] auto GetReader() const -> std::unique_ptr<std::basic_istream<uint8_t>>;

    static auto GetOffset(std::basic_string_view<uint8_t> raw, uint64_t line,
                          uint64_t column) -> std::optional<uint64_t>;
    static auto GetLC(std::basic_string_view<uint8_t> raw,
                      uint64_t offset) -> std::optional<std::pair<uint64_t, uint64_t>>;

    auto GetOffset(uint64_t line, uint64_t column) -> std::optional<uint64_t>;
    auto GetLC(uint64_t offset) -> std::optional<std::pair<uint64_t, uint64_t>>;
  };
}  // namespace no3::lsp::core
