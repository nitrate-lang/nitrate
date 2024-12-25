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

#include <core/Diagnostic.hh>
#include <cstdint>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-ir/IRGraph.hh>
#include <nitrate-ir/Module.hh>
#include <nitrate-ir/Report.hh>
#include <nitrate-parser/AST.hh>
#include <sstream>

using namespace ncc;
using namespace ncc::ir;
using namespace ncc::lex;

///============================================================================///

static void print_qsizeloc(std::stringstream &ss, uint32_t num) {
  if (num == QLEX_EOFF) {
    ss << "?";
  } else {
    ss << num;
  }
}

std::string ir::mint_plain_message(const IReport::ReportData &R,
                                   ISourceView *B) {
  std::stringstream ss;
  uint32_t sl, sc, el, ec;

  {  // Print the filename and location information
    /// FIXME: Render filename
    ss << "??" << ":";

    auto default_if = std::pair<uint32_t, uint32_t>({QLEX_EOFF, QLEX_EOFF});
    auto beg = B->off2rc(R.start_offset).value_or(default_if);
    auto end = default_if;

    sl = beg.first;
    sc = beg.second;
    el = end.first;
    ec = end.second;

    if (sl != QLEX_EOFF || sc != QLEX_EOFF) {
      print_qsizeloc(ss, sl);
      ss << ":";
      print_qsizeloc(ss, sc);
      ss << ":";
    }

    ss << " ";
  }

  (void)ec;
  (void)el;

  switch (R.level) {
    case IC::Debug:
      ss << "debug";
      break;
    case IC::Info:
      ss << "info";
      break;
    case IC::Warn:
      ss << "warning";
      break;
    case IC::Error:
      ss << "error";
      break;
    case IC::FatalError:
      ss << "fatal error";
      break;
  }

  ss << ": " << R.param;

  if (R.code != Info) {
    ss << " [-Werror=" << issue_info.left.at(R.code).flagname << "]";
  }

  uint32_t res = QLEX_EOFF;
  /// TODO: Implement item
  if (res == QLEX_EOFF) {
    ss << "\n# [failed to extract source code snippet]\n";
  }

  return ss.str();
}

std::string ir::mint_clang16_message(const IReport::ReportData &R,
                                     ISourceView *B) {
  std::stringstream ss;
  uint32_t sl, sc, el, ec;

  {  // Print the filename and location information
    ss << "\x1b[39;1m" << "??" << ":";

    auto default_if = std::pair<uint32_t, uint32_t>({QLEX_EOFF, QLEX_EOFF});
    auto beg = B->off2rc(R.start_offset).value_or(default_if);
    auto end = default_if;

    sl = beg.first;
    sc = beg.second;
    el = end.first;
    ec = end.second;

    if (sl != QLEX_EOFF || sc != QLEX_EOFF || el != QLEX_EOFF ||
        ec != QLEX_EOFF) {
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

  switch (R.level) {
    case IC::Debug:
      ss << "\x1b[1mdebug:\x1b[0m " << R.param;
      if (R.code != Info) {
        ss << " \x1b[39;1m[\x1b[0m\x1b[1m-Werror="
           << issue_info.left.at(R.code).flagname
           << "\x1b[0m\x1b[39;1m]\x1b[0m";
      }
      break;
    case IC::Info:
      ss << "\x1b[37;1minfo:\x1b[0m " << R.param;
      if (R.code != Info) {
        ss << " \x1b[39;1m[\x1b[0m\x1b[37;1m-Werror="
           << issue_info.left.at(R.code).flagname
           << "\x1b[0m\x1b[39;1m]\x1b[0m";
      }
      break;
    case IC::Warn:
      ss << "\x1b[35;1mwarning:\x1b[0m " << R.param;
      if (R.code != Info) {
        ss << " \x1b[39;1m[\x1b[0m\x1b[35;1m-Werror="
           << issue_info.left.at(R.code).flagname
           << "\x1b[0m\x1b[39;1m]\x1b[0m";
      }
      break;
    case IC::Error:
      ss << "\x1b[31;1merror:\x1b[0m " << R.param;
      if (R.code != Info) {
        ss << " \x1b[39;1m[\x1b[0m\x1b[31;1m-Werror="
           << issue_info.left.at(R.code).flagname
           << "\x1b[0m\x1b[39;1m]\x1b[0m";
      }
      break;
    case IC::FatalError:
      ss << "\x1b[31;1;4mfatal error:\x1b[0m " << R.param;
      if (R.code != Info) {
        ss << " \x1b[39;1m[\x1b[0m\x1b[31;1;4m-Werror="
           << issue_info.left.at(R.code).flagname
           << "\x1b[0m\x1b[39;1m]\x1b[0m";
      }
      break;
  }

  uint32_t res = QLEX_EOFF; /* qlex_spanx(
       lx, R.start_offset, R.end_offset,
       [](const char *str, uint32_t len, uintptr_t x) {
         if (len > 100) {
           len = 100;
         }

         std::stringstream &ss = *reinterpret_cast<std::stringstream *>(x);
         ss << '\n' << std::string_view(str, len) << '\n';
       },
       reinterpret_cast<uintptr_t>(&ss)); */
  if (res == QLEX_EOFF) {
    ss << "\n# [\x1b[35;1mfailed to extract source code snippet\x1b[0m]\n";
  }

  return ss.str();
}

///============================================================================///

using namespace ncc::ir;

uint64_t DiagDatum::hash() const {
  /* Not quite a PHF, but it's pretty close as long as there are not two many
   * subject strings */
  /* In the worst case messages will be discarded, but that can be fixed by
     passing a parameter to disable deduplication */

  struct BitPack {
    IC level : 3;
    IssueCode code : 10;
    uint64_t param_trunc : 7;
    uint32_t m_fileid : 20;
    uint64_t m_start : 24;
  } __attribute__((packed)) bp;

  bp.level = level;
  bp.code = code;
  bp.param_trunc = std::hash<std::string_view>{}(param);
  bp.m_start = start_offset;
  bp.m_fileid = fileid;

  return std::bit_cast<uint64_t>(bp);
}

void DiagnosticManager::report(IssueCode code, IC level,
                               std::vector<std::string_view> params,
                               std::tuple<uint32_t, uint32_t> loc) {
  std::string message;
  for (auto p : params) {
    message += std::string(p);
  }

  DiagDatum R(code, level, message, std::get<0>(loc), std::get<1>(loc));

  { /* Prevent duplicates and maintain order of messages */
    auto hash = R.hash();
    if (m_visited.contains(hash)) {
      return;
    }
    m_visited.insert(hash);
  }

  m_vec.emplace_back(std::move(R));
}

void DiagnosticManager::stream_reports(
    std::function<void(const ReportData &)> cb) {
  for (auto &item : m_vec) {
    ReportData datum;
    datum.code = item.code;
    datum.level = item.level;
    datum.param = item.param;
    datum.start_offset = item.start_offset;
    datum.fileid = item.fileid;

    cb(datum);
  }
}

static const std::unordered_map<IC, nr_level_t> issue_class_map = {
    {IC::Debug, NR_LEVEL_DEBUG},      {IC::Info, NR_LEVEL_INFO},
    {IC::Warn, NR_LEVEL_WARN},        {IC::Error, NR_LEVEL_ERROR},
    {IC::FatalError, NR_LEVEL_FATAL},
};

CPP_EXPORT void ir::nr_diag_read(IRModule *nr, nr_diag_format_t format,
                                 nr_report_cb cb, uintptr_t data) {
  if (!cb) {
    return;
  }

  ISourceView *B = nr->getOffsetResolver().get();

  nr->getDiag()->stream_reports([&](IReport::ReportData R) {
    std::stringstream ss;

    switch (format) {
      /**
       * @brief Code decimal serialization of the error code.
       * @example `801802`
       * @format <code>
       */
      case NR_DIAG_ASCII_ECODE: {
        ss << std::to_string(static_cast<uint64_t>(R.code));
        break;
      }

      /**
       * @brief Code decimal serialization of the error code and source
       * location.
       * @example `801802:1:1:/path/to/filename.q`
       * @format <code>:<line>:<col>:<path>
       *
       * @note UTF-8 characters in the path are preserved.
       */
      case NR_DIAG_UTF8_ECODE_LOC: {
        ss << std::to_string(static_cast<uint64_t>(R.code)) << ":";
        auto beg = B->off2rc(R.start_offset);

        ss << beg->first << ":";
        ss << beg->second << ":";
        ss << "??";

        break;
      }

      /**
       * @brief Code decimal serialization of the error code and UTF-8 error
       * message.
       * @example `801802:This is an UTF-8 error message.`
       * @format <code>:<utf8_message>
       */
      case NR_DIAG_UTF8_ECODE_ETEXT: {
        ss << std::to_string(static_cast<uint64_t>(R.code)) << ":";
        ss << R.param;

        break;
      }

      /**
       * @brief Unspecified format.
       * @note No-ANSI colors are included
       * @note Includes source location information as well as source code
       * snippets (if available).
       * @note Includes error messages and suggestions.
       * @note Basically, everything you expect from a mainstream compiler
       * (except without color).
       */
      case NR_DIAG_NOSTD_TTY_UTF8: {
        ss << ir::mint_plain_message(R, B);
        break;
      }

      /**
       * @brief Unspecified format.
       * @note Similar to `NR_DIAG_NOSTD_TTY_UTF8`, but with undocumented
       * differences.
       */
      case NR_DIAG_NONSTD_ANSI16_UTF8_FULL: {
        ss << ir::mint_clang16_message(R, B);
        break;
      }

      /**
       * @brief Unspecified format.
       * @note Similar to `NR_DIAG_NOSTD_TTY_UTF8`, but with undocumented
       * differences.
       */
      case NR_DIAG_NONSTD_ANSI256_UTF8_FULL: {
        ss << ir::mint_clang16_message(R, B);
        break;
      }

      /**
       * @brief Unspecified format.
       * @note Similar to `NR_DIAG_NOSTD_TTY_UTF8`, but with undocumented
       * differences.
       */
      case NR_DIAG_NONSTD_ANSIRGB_UTF8_FULL: {
        ss << ir::mint_modern_message(R, B);
        break;
      }
    }

    std::string message = ss.str();
    const uint8_t *ptr = (const uint8_t *)message.c_str();
    auto lvl = issue_class_map.at(R.level);

    cb(ptr, message.size(), lvl, data);
  });
}

CPP_EXPORT void ir::nr_diag_clear(IRModule *nr) {
  nr->getDiag()->erase_reports();
}
