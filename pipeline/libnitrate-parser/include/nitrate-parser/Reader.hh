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

#ifndef __NITRATE_PARSER_READER_H__
#define __NITRATE_PARSER_READER_H__

#include <istream>
#include <nitrate-parser/Vistor.hh>
#include <optional>

namespace npar {
  class AST_JsonReader final {
    std::istream& m_is;
    npar_node_t* m_root;
    bool m_okay;

    void parse();

  public:
    constexpr AST_JsonReader(std::istream& is)
        : m_is(is), m_root(nullptr), m_okay(false) {
      parse();
    }

    constexpr bool okay() const { return m_okay; }

    constexpr std::optional<npar_node_t*> get() const {
      if (!m_okay || !m_root) {
        return std::nullopt;
      }

      return m_root;
    }
  };

  class AST_MsgPackReader final {
    std::istream& m_is;
    npar_node_t* m_root;
    bool m_okay;

    void parse();

  public:
    constexpr AST_MsgPackReader(std::istream& is)
        : m_is(is), m_root(nullptr), m_okay(false) {
      parse();
    }

    constexpr bool okay() const { return m_okay; }

    constexpr std::optional<npar_node_t*> get() const {
      if (!m_okay || !m_root) {
        return std::nullopt;
      }

      return m_root;
    }
  };

  class AST_Reader final {
    std::istream& m_is;
    npar_node_t* m_root;
    bool m_okay;

    enum class InputFormat {
      JSON,
      MSGPACK,
    };

    InputFormat determine_format();

    void parse_json();
    void parse_msgpack();

  public:
    constexpr AST_Reader(std::istream& is)
        : m_is(is), m_root(nullptr), m_okay(false) {
      switch (determine_format()) {
        case InputFormat::JSON:
          parse_json();
          break;
        case InputFormat::MSGPACK:
          parse_msgpack();
          break;
      }
    }

    constexpr bool okay() const { return m_okay; }

    constexpr std::optional<npar_node_t*> get() const {
      if (!m_okay || !m_root) {
        return std::nullopt;
      }

      return m_root;
    }
  };
}  // namespace npar

#endif
