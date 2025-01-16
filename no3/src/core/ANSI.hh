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

#ifndef __NO3_CORE_ANSI_HH__
#define __NO3_CORE_ANSI_HH__

#include <cstdint>
#include <sstream>
#include <string>

namespace no3::ansi {
  enum Style {
    /*==== Text Color ====*/
    FG_BLACK = 1 << 0,
    FG_RED = 1 << 1,
    FG_GREEN = 1 << 2,
    FG_YELLOW = 1 << 3,
    FG_BLUE = 1 << 4,
    FG_PURPLE = 1 << 5,
    FG_CYAN = 1 << 6,
    FG_WHITE = 1 << 7,
    FG_DEFAULT = 1 << 8,

    /*==== Background Color ====*/
    BG_BLACK = 1 << 9,
    BG_RED = 1 << 10,
    BG_GREEN = 1 << 11,
    BG_YELLOW = 1 << 12,
    BG_BLUE = 1 << 13,
    BG_PURPLE = 1 << 14,
    BG_CYAN = 1 << 15,
    BG_WHITE = 1 << 16,
    BG_DEFAULT = 1 << 17,

    /*==== Text Attribute ====*/
    BOLD = 1 << 18,
    UNDERLINE = 1 << 19,
    ILTALIC = 1 << 20,
    STRIKE = 1 << 21,

    RESET = FG_DEFAULT | BG_DEFAULT,

    COLOR_MASK = FG_BLACK | FG_RED | FG_GREEN | FG_YELLOW | FG_BLUE |
                 FG_PURPLE | FG_CYAN | FG_WHITE | FG_DEFAULT,
    ATTRIBUTE_MASK = BOLD | UNDERLINE | ILTALIC | STRIKE,
    BG_COLOR_MASK = BG_BLACK | BG_RED | BG_GREEN | BG_YELLOW | BG_BLUE |
                    BG_PURPLE | BG_CYAN | BG_WHITE | BG_DEFAULT
  };

  class AnsiOut final {
    std::ostream &m_out;
    uint32_t m_style{};

  public:
    AnsiOut(std::ostream &out) : m_out(out){};

    AnsiOut &operator<<(const std::string &str);

    template <class T>
    AnsiOut &Write(const T &msg) {
      std::stringstream ss;
      ss << msg;
      return operator<<(ss.str());
    }

    AnsiOut Newline();

    AnsiOut &SetStyle(uint32_t style) {
      this->m_style = style;
      return *this;
    }
  };

  template <class T>
  AnsiOut &operator<<(AnsiOut &out, const T &msg) {
    return out.Write(msg);
  }

  static inline void operator<<(AnsiOut &out,
                                std::ostream &(*var)(std::ostream &)) {
    if (var == static_cast<std::ostream &(*)(std::ostream &)>(std::endl)) {
      out.Newline();
    }
  }

  static inline void operator|=(AnsiOut &out, uint32_t style) {
    out.SetStyle(style);
  }

  bool IsUsingColors();
}  // namespace no3::ansi

#endif  // __NO3_CORE_ANSI_HH__