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

#ifndef __NITRATE_LEXER_LOCATION_HH__
#define __NITRATE_LEXER_LOCATION_HH__

#include <cstdint>
#include <nitrate-core/String.hh>
#include <nitrate-lexer/ScannerFwd.hh>

namespace ncc::lex {
  constexpr size_t kLexEof = UINT32_MAX;

  class Location {
    uint32_t m_offset = kLexEof, m_line = kLexEof, m_column = kLexEof;
    string m_filename;

  public:
    constexpr Location() = default;
    constexpr Location(uint32_t offset, uint32_t line, uint32_t column, string filename)
        : m_offset(offset), m_line(line), m_column(column), m_filename(filename) {}

    static constexpr auto EndOfFile() { return Location(kLexEof, kLexEof, kLexEof, ""); }

    [[nodiscard]] constexpr auto GetOffset() const { return m_offset; }
    [[nodiscard]] constexpr auto GetRow() const { return m_line; }
    [[nodiscard]] constexpr auto GetCol() const { return m_column; }
    [[nodiscard]] constexpr auto GetFilename() const -> string { return m_filename; }

    bool operator==(const Location &rhs) const {
      return m_offset == rhs.m_offset && m_line == rhs.m_line && m_column == rhs.m_column &&
             m_filename == rhs.m_filename;
    }
  } __attribute__((packed));

  class LocationID {
  public:
    using Counter = uint32_t;

    constexpr explicit LocationID(Counter id = 0) : m_id(id) {}

    auto Get(IScanner &l) const -> Location;
    [[nodiscard]] constexpr auto GetId() const -> Counter { return m_id; }

    [[nodiscard]] constexpr auto operator==(const LocationID &rhs) const -> bool { return m_id == rhs.m_id; }
    [[nodiscard]] constexpr auto operator<(const LocationID &rhs) const -> bool { return m_id < rhs.m_id; }
    [[nodiscard]] constexpr auto HasValue() const -> bool { return m_id != 0; }

  private:
    Counter m_id;
  } __attribute__((packed));

  using LocationRange = std::pair<LocationID, LocationID>;
}  // namespace ncc::lex

#endif
