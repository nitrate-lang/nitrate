////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///  ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
///  ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
///    ░▒▓█▓▒░                                                               ///
///     ░▒▓██▓▒░                                                             ///
///                                                                          ///
///   * QUIX LANG COMPILER - The official compiler for the Quix language.    ///
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

#include <core/LibMacro.h>
#include <quix-core/Error.h>
#include <quix-lexer/Lexer.h>
#include <quix-parser/Node.h>

#include <boost/bimap.hpp>
#include <core/Config.hh>
#include <cstdint>
#include <iomanip>
#include <quix-qxir/IRGraph.hh>
#include <quix-qxir/Module.hh>
#include <quix-qxir/Report.hh>
#include <sstream>

using namespace qxir::diag;

template <typename L, typename R>
boost::bimap<L, R> make_bimap(std::initializer_list<typename boost::bimap<L, R>::value_type> list) {
  return boost::bimap<L, R>(list.begin(), list.end());
}

struct IssueInfo {
  std::string_view flagname;
  std::string overview;
  std::vector<std::string_view> hints;

  bool operator<(const IssueInfo &rhs) const { return flagname < rhs.flagname; }
};

/// FIXME: Write correct stuff here

static const boost::bimap<IssueCode, IssueInfo> details = make_bimap<IssueCode, IssueInfo>({
    {IssueCode::CompilerError, {"Compiler Error", "An error occurred during compilation.", {}}},
    {IssueCode::PTreeInvalid,
     {"ptree-invalid", /* FIXME: Summarize */
      "Parse tree is not okay.",
      {}}},
    {IssueCode::SignalReceived,
     {"signal-recv", "The compiler received an unrecoverable process signal.", {}}},
    {IssueCode::DSPolyCyclicRef,
     {"ds-cyclic-ref",
      "Cyclic polymorphic node reference detected in internal module IR data structure.",
      {"This is an (INTERNAL) compiler error. Please report this issue."}}},
    {IssueCode::DSNullPtr,
     {"ds-nullptr",
      "Nullptr detected in internal module IR data structure.",
      {"This is an (INTERNAL) compiler error. Please report this issue."}}},
    {IssueCode::DSBadType,
     {"ds-bad-type",
      "Internal module IR data structure contains a bad type.",
      {"This is an (INTERNAL) compiler error. Please report this issue."}}},
    {IssueCode::DSMissingMod,
     {"ds-missing-mod",
      "Internal module IR data structure contains a node with a missing module pointer.",
      {"This is an (INTERNAL) compiler error. Please report this issue."}}},
    {IssueCode::DSBadTmpNode,
     {"ds-bad-tmp-node",
      "Internal module IR data structure contains an unexpected temporary node.",
      {"This is an (INTERNAL) compiler error. Please report this issue."}}},
    {IssueCode::Redefinition,
     {"redefinition", /* FIXME: Summarize */
      "write me",
      {}}},
    {IssueCode::UnknownFunction,
     {"unknown-function", /* FIXME: Summarize */
      "write me",
      {}}},
    {IssueCode::TooManyArguments,
     {"too-many-arguments", /* FIXME: Summarize */
      "write me",
      {}}},
    {IssueCode::UnknownArgument,
     {"unknown-argument", /* FIXME: Summarize */
      "write me",
      {}}},

    {IssueCode::UnknownType,
     {"unknown-type", /* FIXME: Summarize */
      "write me",
      {}}},
    {IssueCode::UnresolvedIdentifier,
     {"unresolved-identifier", /* FIXME: Summarize */
      "404 - Identifier '%s' not found.",
      {"Make sure the identifier is defined in the current scope.", "Check for typos.",
       "Check for visibility."}}},

    {IssueCode::MissingReturn,
     {"missing-return",
      "Function '%s' is missing a return statement.",
      {"Make sure all code paths return a value.",
       "Check for missing return statements in conditional branches.",
       "If your code is complicated, consider using an unreachable assertion."}}},
});

///============================================================================///

static void print_qsizeloc(std::stringstream &ss, qlex_size num) {
  if (num == UINT32_MAX) {
    ss << "?";
  } else {
    ss << num;
  }
}

static std::vector<std::string_view> word_break(std::string_view text, size_t max_width) {
  /** TODO: Unit testing */

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

static std::string format_overview(std::string_view overview, std::string_view param) {
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

static void confine_rect_bounds(int64_t &x_0, int64_t &y_0, int64_t &x_1, int64_t &y_1,
                                size_t win_width) {
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

std::string DiagnosticManager::mint_modern_message(const DiagMessage &msg) const {
  constexpr size_t WIDTH = 70;

  std::stringstream ss;
  qlex_t *lx = m_qxir->getLexer();
  qlex_size sl, sc, el, ec;

  { /* Print filename and source row:column start and end */
    ss << "\x1b[37;1m" << qlex_filename(lx) << ":";

    sl = qlex_line(lx, msg.m_start);
    sc = qlex_col(lx, msg.m_start);
    el = qlex_line(lx, msg.m_end);
    ec = qlex_col(lx, msg.m_end);

    if (sl != UINT32_MAX || sc != UINT32_MAX || el != UINT32_MAX || ec != UINT32_MAX) {
      print_qsizeloc(ss, sl);
      ss << ":";
      print_qsizeloc(ss, sc);

      ss << "-";
      print_qsizeloc(ss, el);
      ss << ":";
      print_qsizeloc(ss, ec);

      ss << ":\x1b[0m";
    }

    ss << " ";
  }

  { /* Print message flagname */
    switch (msg.m_type) {
      case IssueClass::Debug:
        ss << "\x1b[1mdebug:\x1b[0m \x1b[1m" << details.left.at(msg.m_code).flagname << "\x1b[0m\n";
        break;
      case IssueClass::Info:
        ss << "\x1b[37;1minfo:\x1b[0m \x1b[37;1m" << details.left.at(msg.m_code).flagname
           << "\x1b[0m\n";
        break;
      case IssueClass::Warn:
        ss << "\x1b[35;1mwarning:\x1b[0m \x1b[35;1m" << details.left.at(msg.m_code).flagname
           << "\x1b[0m\n";
        break;
      case IssueClass::Error:
        ss << "\x1b[31;1merror:\x1b[0m \x1b[31;1m" << details.left.at(msg.m_code).flagname
           << "\x1b[0m\n";
        break;
      case IssueClass::FatalError:
        ss << "\x1b[31;1;4mfatal error:\x1b[0m \x1b[31;1;4m" << details.left.at(msg.m_code).flagname
           << "\x1b[0m\n";
        break;
    }
  }

  std::string ind;
  size_t ind_sz;

  if (sl != UINT32_MAX) {
    ind_sz = std::ceil(std::log10(sl));
    ind = std::string(ind_sz, ' ');
  } else {
    ind_sz = 0;
  }

  { /* Print message overview */
    auto data = format_overview(details.left.at(msg.m_code).overview, msg.m_msg);
    auto lines = word_break(data, WIDTH);

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
    auto hints = details.left.at(msg.m_code).hints;

    if (!hints.empty()) {
      ss << ind << "\x1b[33m╔═\x1b[0m \x1b[32;1mCode Intelligence:\x1b[0m\n";
      for (auto hint : hints) {
        auto lines = word_break(hint, WIDTH - 2);

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

  if (sl != UINT32_MAX && sc != UINT32_MAX && el != UINT32_MAX &&
      ec != UINT32_MAX) { /* Source window */
    constexpr size_t WINDOW_WIDTH = 60;

    /// TODO: Use lexer to add color coating

    int64_t x_0 = sc, y_0 = sl, x_1 = ec, y_1 = el;
    confine_rect_bounds(x_0, y_0, x_1, y_1, WINDOW_WIDTH);

    size_t width = x_1 - x_0;
    size_t height = y_1 - y_0;
    size_t buf_size = width * height + 1;
    std::unique_ptr<char[]> out(new char[buf_size]);

    qlex_rect(lx, x_0, y_0, x_1, y_1, out.get(), buf_size, ' ');

    std::vector<std::string_view> source_lines;

    for (size_t i = 0; i < height; i++) {
      source_lines.push_back(std::string_view(&out[i * width], width));
    }

    /*= {
        R"(pub "c" fn main(args: [string]): i32 {                      )",
        R"(  print(20); // Hello world                                 )",
        R"(}                                                           )"};
        */

    { /* Render code view */
      std::string sep;
      for (size_t i = 0; i < WINDOW_WIDTH + 2; i++) {
        sep += "━";
      }

      ss << ind << "  \x1b[32m┏" << sep << "┓\x1b[0m\n";
      for (size_t i = 0; i < source_lines.size(); i++) {
        auto lines = word_break(source_lines[i], WINDOW_WIDTH);

        for (const auto &line : lines) {
          if (sl != UINT32_MAX) {
            ss << std::setw(ind_sz + 1) << (sl - (source_lines.size() / 2)) + i + 1;
          } else {
            ss << std::setw(ind_sz + 1) << "?";
          }

          ss << " \x1b[32m┃\x1b[0m " << line;
          if (line.size() < WINDOW_WIDTH + 2) {
            ss << std::string(WINDOW_WIDTH - line.size(), ' ');
          }
          ss << " \x1b[32m┃\x1b[0m\n";
        }
      }

      ss << ind << "  \x1b[32m┗" << sep << "┛\x1b[0m\n\n";
    }
  }

  return ss.str();
}
