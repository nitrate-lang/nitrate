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

#include <boost/bimap.hpp>
#include <cstdint>
#include <iomanip>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-ir/IR/Nodes.hh>
#include <nitrate-ir/Module.hh>
#include <nitrate-ir/diagnostic/EC.hh>
#include <nitrate-ir/diagnostic/Report.hh>
#include <nitrate-parser/AST.hh>
#include <sstream>

using namespace ncc;
using namespace ncc::ir;
using namespace ncc::lex;

static std::vector<std::string_view> WordBreak(std::string_view text, size_t max_width) {
  std::vector<std::string_view> lines;
  size_t word_beg = 0, cur_beg = 0, cur_len = 0;

  enum {
    Main,
    Word,
  } state = Main;

  size_t i;

  for (i = 0; i < text.size(); i++) {
    char ch = text[i];

    switch (state) {
      case Main: {
        if (std::isspace(ch)) {
          state = Word;
        }
        cur_len++;
        break;
      }
      case Word: {
        while (std::isspace(ch) && i < text.size()) {
          ch = text[i++];
          cur_len++;

          if (cur_len >= max_width) {
            std::string_view line = text.substr(cur_beg, cur_len);
            while (line.size() > max_width) {
              lines.push_back(line.substr(0, max_width));
              line = line.substr(max_width);
            }
            if (!line.empty()) {
              lines.push_back(line);
            }
            cur_beg += cur_len;
            cur_len = 0;
          }
        }

        if (i == text.size()) {
          std::string_view last = text.substr(cur_beg, cur_len);
          while (last.size() > max_width) {
            lines.push_back(last.substr(0, max_width));
            last = last.substr(max_width);
          }

          if (!last.empty()) {
            lines.push_back(last);
          }
          return lines;
        }

        assert(!std::isspace(ch));

        word_beg = i;

        while (!std::isspace(ch) && i < text.size()) {
          ch = text[i++];
          cur_len++;

          if (cur_len > max_width) {
            lines.push_back(text.substr(cur_beg, word_beg - cur_beg));
            cur_beg = word_beg;
            cur_len = (i - word_beg);
          }
        }

        cur_len++;
        state = Main;
        break;
      }
    }
  }

  std::string_view last = text.substr(cur_beg, cur_len);
  while (last.size() > max_width) {
    lines.push_back(last.substr(0, max_width));
    last = last.substr(max_width);
  }

  if (!last.empty()) {
    lines.push_back(last);
  }

  return lines;
}

static std::string FormatOverview(std::string_view overview, std::string_view param) {
  std::string formatted;
  size_t i = 0;

  while (i < overview.size()) {
    if (overview[i] == '%') {
      if (i + 1 < overview.size()) {
        if (overview[i + 1] == 's') {
          formatted += param;
          i += 2;
          continue;
        }
      }
    }

    formatted += overview[i];
    i++;
  }

  return formatted;
}

static void ConfineRectBounds(int64_t &x_0, int64_t &y_0, int64_t &x_1, int64_t &y_1, size_t win_width) {
  if (x_1 < x_0) {
    x_1 = x_0;
  }

  if (y_1 < y_0) {  // Should never happen, but who knows
    y_1 = y_0;
  }

  size_t width = x_1 - x_0;

  if (width > win_width) {
    x_1 = x_0 + win_width;
    width = win_width;
  }

  int64_t ledge = x_0 - (win_width - width) / 2;
  if (width < win_width && ledge >= 0) {
    x_0 -= (win_width - width) / 2;
    x_1 = x_0 + win_width;
  } else if (width < win_width) {
    x_0 = 0;
    x_1 = win_width;
  }

  if (y_1 - y_0 < 3) {
    y_1 = y_0 + 3;
  } else if (y_1 - y_0 > 5) {
    y_1 = y_0 + 5;
  }

  if (x_0 < 0) x_0 = 0;
  if (y_0 < 0) y_0 = 0;
  if (x_0 < 0) x_0 = 0;
  if (y_1 < 0) y_1 = 0;
}

NCC_EXPORT std::string ec::Formatter(std::string_view msg, Sev sev) {
  constexpr size_t kWidth = 70;

  std::string_view flagname = "" /* TODO: Get flagname */;
  std::string_view overview = "" /* TODO: Get overview */;
  std::vector<std::string> hints = {} /* TODO: Get hints */;

  std::stringstream ss;
  uint32_t sl = 0, sc = 0, el = 0, ec = 0;

  { /* Print filename and source row:column start and end */
    /// FIXME: Get source location
    ss << "\x1b[0m\x1b[37;1m" << "??" << ":";

    auto default_if = std::pair<uint32_t, uint32_t>(kLexEof, kLexEof);
    auto beg = default_if;
    auto end = default_if;

    sl = beg.first;
    sc = beg.second;
    el = end.first;
    ec = end.second;

    if (sl != kLexEof || sc != kLexEof || el != kLexEof || ec != kLexEof) {
      ss << (sl == kLexEof ? "?" : std::to_string(sl));
      ss << ":";
      ss << (sc == kLexEof ? "?" : std::to_string(sc));

      ss << "-";
      ss << (el == kLexEof ? "?" : std::to_string(el));
      ss << ":";
      ss << (ec == kLexEof ? "?" : std::to_string(ec));

      ss << ":\x1b[0m";
    }

    ss << " ";
  }

  { /* Print message flagname */
    switch (sev) {
      case Sev::Trace:
      case Sev::Debug:
      case Sev::Raw: {
        ss << "\x1b[1mdebug:\x1b[0m \x1b[1m" << flagname << "\x1b[0m\n";
        break;
      }

      case Sev::Notice:
      case Sev::Info: {
        ss << "\x1b[37;1minfo:\x1b[0m \x1b[37;1m" << flagname << "\x1b[0m\n";
        break;
      }

      case Sev::Warning: {
        ss << "\x1b[35;1mwarning:\x1b[0m \x1b[35;1m" << flagname << "\x1b[0m\n";
        break;
      }

      case Sev::Error:
      case Sev::Alert:
      case Sev::Critical:
      case Sev::Emergency: {
        ss << "\x1b[31;1merror:\x1b[0m \x1b[31;1m" << flagname << "\x1b[0m\n";
        break;
      }
    }
  }

  std::string ind;
  size_t ind_sz;

  if (sl != kLexEof) {
    ind_sz = std::ceil(std::log10(sl));
    ind = std::string(ind_sz, ' ');
  } else {
    ind_sz = 0;
  }

  { /* Print message overview */

    auto data = FormatOverview(overview, msg);
    auto lines = WordBreak(data, kWidth);

    if (lines.size() == 0) {
    } else if (lines.size() == 1) {
      ss << ind << "\x1b[33m╠\x1b[0m \x1b[35;1m" << lines[0] << "\x1b[0m\n\n";
    } else {
      ss << ind << "\x1b[33m╔\x1b[0m \x1b[35;1m" << lines[0] << "\x1b[0m\n";
      for (size_t i = 1; i < lines.size() - 1; i++) {
        ss << ind << "\x1b[33m║\x1b[0m \x1b[35;1m" << lines[i] << "\x1b[0m\n";
      }
      ss << ind << "\x1b[33m╚\x1b[0m \x1b[35;1m" << lines[lines.size() - 1] << "\x1b[0m\n\n";
    }
  }

  { /* Print code intelligence */
    if (!hints.empty()) {
      ss << ind
         << "\x1b[33m╔═\x1b[0m \x1b[32;1mCode "
            "Intelligence:\x1b[0m\n";
      for (auto hint : hints) {
        auto lines = WordBreak(hint, kWidth - 2);

        if (lines.size() == 0) {
        } else if (lines.size() == 1) {
          ss << ind << "\x1b[33m╠═\x1b[0m \x1b[37m" << lines[0] << "\x1b[0m\n";
        } else {
          ss << ind << "\x1b[33m╠═\x1b[0m \x1b[37m" << lines[0] << "\x1b[0m\n";
          for (size_t i = 1; i < lines.size() - 1; i++) {
            ss << ind << "\x1b[33m║\x1b[0m   \x1b[37m" << lines[i] << "\x1b[0m\n";
          }
          ss << ind << "\x1b[33m║\x1b[0m   \x1b[37m" << lines[lines.size() - 1] << "\x1b[0m\n";
        }
      }

      ss << "\n";
    }
  }

  if (sl != kLexEof && sc != kLexEof && el != kLexEof && ec != kLexEof) { /* Source window */
    constexpr size_t kWindowWidth = 60;

    int64_t x_0 = sc, y_0 = sl, x_1 = ec, y_1 = el;
    ConfineRectBounds(x_0, y_0, x_1, y_1, kWindowWidth);

    /// TODO: Get source code
    auto source_lines = std::optional<std::vector<std::string_view>>();

    if (source_lines.has_value()) {
      std::string sep;
      for (size_t i = 0; i < kWindowWidth + 2; i++) {
        sep += "━";
      }

      ss << ind << "  \x1b[32m┏" << sep << "┓\x1b[0m\n";
      for (size_t i = 0; i < source_lines.value().size(); i++) {
        auto lines = WordBreak(source_lines.value()[i], kWindowWidth);

        for (const auto &line : lines) {
          if (sl != kLexEof) {
            ss << std::setw(ind_sz + 1) << (sl - (source_lines.value().size() / 2)) + i + 1;
          } else {
            ss << std::setw(ind_sz + 1) << "?";
          }

          ss << " \x1b[32m┃\x1b[0m " << line;
          if (line.size() < kWindowWidth + 2) {
            ss << std::string(kWindowWidth - line.size(), ' ');
          }
          ss << " \x1b[32m┃\x1b[0m\n";
        }
      }

      ss << ind << "  \x1b[32m┗" << sep << "┛\x1b[0m\n\n";
    }
  }

  return ss.str();
}
