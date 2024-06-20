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

#ifndef __QPKG_BENCH_HH__
#define __QPKG_BENCH_HH__

#include <chrono>
#include <cstdint>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <string_view>

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

  COLOR_MASK = FG_BLACK | FG_RED | FG_GREEN | FG_YELLOW | FG_BLUE | FG_PURPLE |
               FG_CYAN | FG_WHITE | FG_DEFAULT,
  ATTRIBUTE_MASK = BOLD | UNDERLINE | ILTALIC | STRIKE,
  BG_COLOR_MASK = BG_BLACK | BG_RED | BG_GREEN | BG_YELLOW | BG_BLUE |
                  BG_PURPLE | BG_CYAN | BG_WHITE | BG_DEFAULT
};

static inline Style operator|(Style a, Style b) {
  return static_cast<Style>(static_cast<uint32_t>(a) |
                            static_cast<uint32_t>(b));
}

static inline Style operator&(Style a, Style b) {
  return static_cast<Style>(static_cast<uint32_t>(a) &
                            static_cast<uint32_t>(b));
}

static inline bool operator==(Style a, uint32_t b) {
  return static_cast<uint32_t>(a) == b;
}

class AnsiCout final {
  Style style;

 public:
  AnsiCout();

  AnsiCout &operator<<(const std::string &str);

  template <class T>
  AnsiCout &write(const T &msg) {
    std::stringstream ss;
    ss << msg;
    return operator<<(ss.str());
  }

  AnsiCout newline();

  AnsiCout &set_style(Style style) {
    this->style = style;
    return *this;
  }
};

template <class T>
AnsiCout &operator<<(AnsiCout &out, const T &msg) {
  return out.write(msg);
}

static inline void operator<<(AnsiCout &out,
                              std::ostream &(*var)(std::ostream &)) {
  if (var == static_cast<std::ostream &(*)(std::ostream &)>(std::endl)) {
    out.newline();
  }
}

static inline void operator|=(AnsiCout &out, Style style) {
  out.set_style(style);
}

extern thread_local AnsiCout acout;
}  // namespace ansi

namespace bench {
int run_benchmark_lexer();
int run_benchmark_parser();
int run_benchmark_quix_ir();
int run_benchmark_delta_ir();
int run_benchmark_llvm_ir();
int run_benchmark_llvm_codegen();
int run_benchmark_c11_codegen();
int run_benchmark_pipeline();

extern const std::string_view test_source_code;

class Progress {
  std::string m_title;
  std::string m_last_msg;
  std::queue<std::pair<std::string, double>> m_queue;
  std::chrono::high_resolution_clock::time_point m_last_print;
  constexpr static std::chrono::milliseconds m_print_interval =
      std::chrono::milliseconds(1000);

  void print(double percent, const std::string &msg);

 public:
  Progress(const std::string &title);

  void update(double percent, const std::string &msg = "");
  void done(const std::string &outfile_name = "");

  enum class Result {
    THROUGHPUT,
    MEMORY,
    TIME,
  };

  void begin_result(Result type);
  void result(const std::string &msg);
  void end_result();
};
}  // namespace bench

}  // namespace qpkg

#endif  // __QPKG_BENCH_HH__