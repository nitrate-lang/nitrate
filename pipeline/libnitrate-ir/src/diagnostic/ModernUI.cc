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

#include <nitrate-core/Error.h>
#include <nitrate-core/Macro.h>
#include <nitrate-parser/Node.h>

#include <boost/bimap.hpp>
#include <core/Config.hh>
#include <core/Diagnostic.hh>
#include <cstdint>
#include <iomanip>
#include <nitrate-ir/IRGraph.hh>
#include <nitrate-ir/Module.hh>
#include <sstream>

#include "nitrate-ir/Report.hh"

using namespace nr;

template <typename L, typename R>
boost::bimap<L, R> make_bimap(
    std::initializer_list<typename boost::bimap<L, R>::value_type> list) {
  return boost::bimap<L, R>(list.begin(), list.end());
}

/// FIXME: Write correct stuff here

const boost::bimap<IssueCode, IssueInfo> nr::issue_info =
    make_bimap<IssueCode, IssueInfo>({
        {Info, {"info", "%s", {}}},
        {CompilerError,
         {"error", "An error occurred during compilation: %s", {}}},
        {PTreeInvalid, {"ptree-invalid", "%s", {}}},
        {SignalReceived,
         {"signal-recv",
          "The compiler received an unrecoverable process signal.",
          {}}},
        {DSPolyCyclicRef,
         {"ds-cyclic-ref",
          "Cyclic polymorphic node reference detected in internal module IR "
          "data structure.",
          {"This is an (INTERNAL) compiler error. Please report this issue."}}},
        {DSNullPtr,
         {"ds-nullptr",
          "Nullptr detected in internal module IR data structure.",
          {"This is an (INTERNAL) compiler error. Please report this issue."}}},
        {DSBadType,
         {"ds-bad-type",
          "Internal module IR data structure contains a bad type.",
          {"This is an (INTERNAL) compiler error. Please report this issue."}}},
        {DSBadTmpNode,
         {"ds-bad-tmp-node",
          "Internal module IR data structure contains an unexpected temporary "
          "node.",
          {"This is an (INTERNAL) compiler error. Please report this issue."}}},
        {NameConflict,
         {"name-conflict",
          "Naming conflict: %s",
          {{"Ensure that the name is unique."},
           {"Try wrapping your code in a named subsystem"}}}},
        {UnknownFunction, {"unknown-function", "write me", {}}},
        {VariadicNotEnoughArguments,
         {"variadic-not-enough-args",
          "Variadic function call '%s' has too few arguments.",
          {"Ensure that the number of arguments is correct."}}},
        {TwoManyArguments,
         {"too-many-args",
          "Function call '%s' has too many arguments.",
          {"Ensure that the number of arguments is correct."}}},
        {TwoFewArguments,
         {"too-few-args",
          "Function call '%s' has too few arguments.",
          {"Ensure that the number of arguments is correct."}}},
        {TypeInference, {"type-inference", "Type inference failed: %s", {}}},
        {NameManglingTypeInfer,
         {"nm-type-infer",
          "Failed to mangle the name of symbol named: '%s'.",
          {
              "Ensure that the symbol node is correctly typed.",
          }}},
        {UnexpectedUndefLiteral,
         {"bad-undef-keyword",
          "Unexpected 'undef' keyword",
          {"The 'undef' keyword is only permitted as default values for "
           "variable declarations."}}},

        {UnknownType, {"unknown-type", "write me", {}}},
        {UnresolvedIdentifier,
         {"unresolved-identifier",
          "404 - Identifier '%s' not found.",
          {"Make sure the identifier is defined in the current scope.",
           "Check for typos.", "Check for visibility."}}},
        {TypeRedefinition,
         {"type-redefinition",
          "Type '%s' is redefined.",
          {"Ensure that the one-defintion-rule (ODR) is obeyed.",
           "Check for typos.", "Check for visibility."}}},
        {BadCast,
         {"bad-cast",
          "%s",
          {
              "Ensure that the cast is valid.",
          }}},

        {MissingReturn,
         {"missing-return",
          "Function '%s' is missing a return statement.",
          {"Make sure all code paths return a value.",
           "Check for missing return statements in conditional branches.",
           "If your code is complicated, consider using an unreachable "
           "assertion."}}},
    });

///============================================================================///

static void print_qsizeloc(std::stringstream &ss, uint32_t num) {
  if (num == UINT32_MAX) {
    ss << "?";
  } else {
    ss << num;
  }
}

static std::vector<std::string_view> word_break(std::string_view text,
                                                size_t max_width) {
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

static std::string format_overview(std::string_view overview,
                                   std::string_view param) {
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

static void confine_rect_bounds(int64_t &x_0, int64_t &y_0, int64_t &x_1,
                                int64_t &y_1, size_t win_width) {
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

std::string nr::mint_modern_message(const IReport::ReportData &R,
                                    ISourceView *B) {
  constexpr size_t WIDTH = 70;

  std::stringstream ss;
  uint32_t sl, sc, el, ec;

  { /* Print filename and source row:column start and end */
    /// FIXME: Render filename
    ss << "\x1b[37;1m" << "??" << ":";

    auto default_if = std::pair<uint32_t, uint32_t>(UINT32_MAX, UINT32_MAX);
    auto beg = B->off2rc(R.start_offset).value_or(default_if);
    auto end = B->off2rc(R.end_offset).value_or(default_if);

    sl = beg.first;
    sc = beg.second;
    el = end.first;
    ec = end.second;

    if (sl != UINT32_MAX || sc != UINT32_MAX || el != UINT32_MAX ||
        ec != UINT32_MAX) {
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
    switch (R.level) {
      case IC::Debug:
        ss << "\x1b[1mdebug:\x1b[0m \x1b[1m"
           << issue_info.left.at(R.code).flagname << "\x1b[0m\n";
        break;
      case IC::Info:
        ss << "\x1b[37;1minfo:\x1b[0m \x1b[37;1m"
           << issue_info.left.at(R.code).flagname << "\x1b[0m\n";
        break;
      case IC::Warn:
        ss << "\x1b[35;1mwarning:\x1b[0m \x1b[35;1m"
           << issue_info.left.at(R.code).flagname << "\x1b[0m\n";
        break;
      case IC::Error:
        ss << "\x1b[31;1merror:\x1b[0m \x1b[31;1m"
           << issue_info.left.at(R.code).flagname << "\x1b[0m\n";
        break;
      case IC::FatalError:
        ss << "\x1b[31;1;4mfatal error:\x1b[0m \x1b[31;1;4m"
           << issue_info.left.at(R.code).flagname << "\x1b[0m\n";
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
    auto data = format_overview(issue_info.left.at(R.code).overview, R.param);
    auto lines = word_break(data, WIDTH);

    if (lines.size() == 0) {
    } else if (lines.size() == 1) {
      ss << ind << "\x1b[33m╠\x1b[0m \x1b[35;1m" << lines[0] << "\x1b[0m\n\n";
    } else {
      ss << ind << "\x1b[33m╔\x1b[0m \x1b[35;1m" << lines[0] << "\x1b[0m\n";
      for (size_t i = 1; i < lines.size() - 1; i++) {
        ss << ind << "\x1b[33m║\x1b[0m \x1b[35;1m" << lines[i] << "\x1b[0m\n";
      }
      ss << ind << "\x1b[33m╚\x1b[0m \x1b[35;1m" << lines[lines.size() - 1]
         << "\x1b[0m\n\n";
    }
  }

  { /* Print code intelligence */
    auto hints = issue_info.left.at(R.code).hints;

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
            ss << ind << "\x1b[33m║\x1b[0m   \x1b[37m" << lines[i]
               << "\x1b[0m\n";
          }
          ss << ind << "\x1b[33m║\x1b[0m   \x1b[37m" << lines[lines.size() - 1]
             << "\x1b[0m\n";
        }
      }

      ss << "\n";
    }
  }

  if (sl != UINT32_MAX && sc != UINT32_MAX && el != UINT32_MAX &&
      ec != UINT32_MAX) { /* Source window */
    constexpr size_t WINDOW_WIDTH = 60;

    int64_t x_0 = sc, y_0 = sl, x_1 = ec, y_1 = el;
    confine_rect_bounds(x_0, y_0, x_1, y_1, WINDOW_WIDTH);

    auto source_lines = B->rect(x_0, y_0, x_1, y_1);

    if (source_lines.has_value()) {
      std::string sep;
      for (size_t i = 0; i < WINDOW_WIDTH + 2; i++) {
        sep += "━";
      }

      ss << ind << "  \x1b[32m┏" << sep << "┓\x1b[0m\n";
      for (size_t i = 0; i < source_lines.value().size(); i++) {
        auto lines = word_break(source_lines.value()[i], WINDOW_WIDTH);

        for (const auto &line : lines) {
          if (sl != UINT32_MAX) {
            ss << std::setw(ind_sz + 1)
               << (sl - (source_lines.value().size() / 2)) + i + 1;
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
