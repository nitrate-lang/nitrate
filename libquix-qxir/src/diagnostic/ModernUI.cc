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

#define __QUIX_IMPL__

#include <core/LibMacro.h>
#include <quix-core/Error.h>
#include <quix-parser/Node.h>
#include <quix-qxir/Module.h>
#include <quix-qxir/Node.h>

#include <boost/bimap.hpp>
#include <core/Config.hh>
#include <cstdint>
#include <iomanip>
#include <quix-qxir/Report.hh>
#include <sstream>

using namespace qxir::diag;

template <typename L, typename R>
boost::bimap<L, R> make_bimap(std::initializer_list<typename boost::bimap<L, R>::value_type> list) {
  return boost::bimap<L, R>(list.begin(), list.end());
}

struct IssueInfo {
  std::string_view flagname;
  std::string_view overview;
  std::vector<std::string_view> hints;

  bool operator<(const IssueInfo &rhs) const { return flagname < rhs.flagname; }
};

/// FIXME: Write correct stuff here

static const boost::bimap<IssueCode, IssueInfo> details = make_bimap<IssueCode, IssueInfo>({
    {IssueCode::Default,
     {"unspecified", /* TODO: Summarize */
      "write me",
      {}}},
    {IssueCode::PTreeInvalid,
     {"ptree-invalid", /* TODO: Summarize */
      "This function is expected to return in all possible branches. Why is your function failing "
      "to do so?",
      {"Make sure you have a return statement when you need one.",
       "If you are using a loop and avoiding a return, ensure that it is knowable that the loop "
       "will always terminate.",
       "If you are optimizing, make sure it is knowable that that a particular branch is "
       "impossible to avoid this error."}}},
    {IssueCode::SignalReceived,
     {"signal-recv", /* TODO: Summarize */
      "write me",
      {}}},
    {IssueCode::DSPolyCyclicRef,
     {"ds-cyclic-ref", /* TODO: Summarize */
      "write me",
      {}}},
    {IssueCode::DSNullPtr,
     {"ds-nullptr", /* TODO: Summarize */
      "write me",
      {}}},
    {IssueCode::DSBadType,
     {"ds-bad-type", /* TODO: Summarize */
      "write me",
      {}}},
    {IssueCode::DSMissingMod,
     {"ds-missing-mod", /* TODO: Summarize */
      "write me",
      {}}},

    {IssueCode::Redefinition,
     {"redefinition", /* TODO: Summarize */
      "write me",
      {}}},
    {IssueCode::UnknownFunction,
     {"unknown-function", /* TODO: Summarize */
      "write me",
      {}}},
    {IssueCode::TooManyArguments,
     {"too-many-arguments", /* TODO: Summarize */
      "write me",
      {}}},
    {IssueCode::UnknownArgument,
     {"unknown-argument", /* TODO: Summarize */
      "write me",
      {}}},

    {IssueCode::UnknownType,
     {"unknown-type", /* TODO: Summarize */
      "write me",
      {}}},
    {IssueCode::UnresolvedIdentifier,
     {"unresolved-identifier", /* TODO: Summarize */
      "write me",
      {}}},
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

std::string DiagnosticManager::mint_modern_message(const DiagMessage &msg) const {
  constexpr size_t WIDTH = 70;

  std::stringstream ss;
  qlex_t *lx = m_qxir->getLexer();
  qlex_size sl, sc, el, ec;

  { /* Print filename and source row:column start and end */
    ss << "\x1b[37;1m" << qlex_filename(lx) << ":";

    sl = qlex_line(lx, msg.start);
    sc = qlex_col(lx, msg.start);
    el = qlex_line(lx, msg.end);
    ec = qlex_col(lx, msg.end);

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
    switch (msg.type) {
      case IssueClass::Debug:
        ss << "\x1b[1mdebug:\x1b[0m \x1b[1m" << details.left.at(msg.code).flagname << "\x1b[0m\n";
        break;
      case IssueClass::Info:
        ss << "\x1b[37;1minfo:\x1b[0m \x1b[37;1m" << details.left.at(msg.code).flagname
           << "\x1b[0m\n";
        break;
      case IssueClass::Warn:
        ss << "\x1b[35;1mwarning:\x1b[0m \x1b[35;1m" << details.left.at(msg.code).flagname
           << "\x1b[0m\n";
        break;
      case IssueClass::Error:
        ss << "\x1b[31;1merror:\x1b[0m \x1b[31;1m" << details.left.at(msg.code).flagname
           << "\x1b[0m\n";
        break;
      case IssueClass::FatalError:
        ss << "\x1b[31;1;4mfatal error:\x1b[0m \x1b[31;1;4m" << details.left.at(msg.code).flagname
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
    auto lines = word_break(details.left.at(msg.code).overview, WIDTH);

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
    auto hints = details.left.at(msg.code).hints;

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

  if (sl != UINT32_MAX && sc != UINT32_MAX) { /* Source window */
    constexpr size_t WINDOW_WIDTH = 60;

    /// TODO: Get from the lexer
    /// TODO: Use lexer to add color coating

    std::vector<std::string_view> source_lines = {"pub \"c\" fn main(args: [string]): i32 {",
                                                  "  print(20); // Hello world", "}"};

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

  return ss.str();
}
