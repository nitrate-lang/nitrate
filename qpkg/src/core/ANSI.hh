////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///           ░▒▓██████▓▒░░▒▓███████▓▒░░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░            ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░           ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░                  ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓███████▓▒░░▒▓███████▓▒░░▒▓█▓▒▒▓███▓▒░           ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░           ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░           ///
///           ░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░            ///
///             ░▒▓█▓▒░                                                      ///
///              ░▒▓██▓▒░                                                    ///
///                                                                          ///
///   * QUIX PACKAGE MANAGER - The official tool for the Quix language.      ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The QUIX Compiler Suite is free software; you can redistribute it or   ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The QUIX Compiler Suite is distributed in the hope that it will be     ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the QUIX Compiler Suite; if not, see                ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#ifndef __QPKG_CORE_ANSI_HH__
#define __QPKG_CORE_ANSI_HH__

#include <cstdint>
#include <sstream>
#include <string>

namespace qpkg {
  namespace ansi {
    enum class Style {
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

      COLOR_MASK = FG_BLACK | FG_RED | FG_GREEN | FG_YELLOW | FG_BLUE | FG_PURPLE | FG_CYAN |
                   FG_WHITE | FG_DEFAULT,
      ATTRIBUTE_MASK = BOLD | UNDERLINE | ILTALIC | STRIKE,
      BG_COLOR_MASK = BG_BLACK | BG_RED | BG_GREEN | BG_YELLOW | BG_BLUE | BG_PURPLE | BG_CYAN |
                      BG_WHITE | BG_DEFAULT
    };

    static inline Style operator|(Style a, Style b) {
      return static_cast<Style>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

    static inline Style operator&(Style a, Style b) {
      return static_cast<Style>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
    }

    static inline bool operator==(Style a, uint32_t b) { return static_cast<uint32_t>(a) == b; }

    class AnsiCerr final {
      Style style;

    public:
      AnsiCerr();

      AnsiCerr &operator<<(const std::string &str);

      template <class T>
      AnsiCerr &write(const T &msg) {
        std::stringstream ss;
        ss << msg;
        return operator<<(ss.str());
      }

      AnsiCerr newline();

      AnsiCerr &set_style(Style style) {
        this->style = style;
        return *this;
      }
    };

    template <class T>
    AnsiCerr &operator<<(AnsiCerr &out, const T &msg) {
      return out.write(msg);
    }

    static inline void operator<<(AnsiCerr &out, std::ostream &(*var)(std::ostream &)) {
      if (var == static_cast<std::ostream &(*)(std::ostream &)>(std::endl)) {
        out.newline();
      }
    }

    static inline void operator|=(AnsiCerr &out, Style style) { out.set_style(style); }

    extern thread_local AnsiCerr acout;
  }  // namespace ansi

}  // namespace qpkg

#endif  // __QPKG_CORE_ANSI_HH__