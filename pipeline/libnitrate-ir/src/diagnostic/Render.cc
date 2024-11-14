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

#include <core/LibMacro.h>
#include <nitrate-core/Error.h>
#include <nitrate-parser/Node.h>

#include <core/Config.hh>
#include <cstddef>
#include <cstdint>
#include <nitrate-ir/IRGraph.hh>
#include <nitrate-ir/Module.hh>
#include <nitrate-ir/Report.hh>
#include <sstream>

using namespace nr;

///============================================================================///

static void print_qsizeloc(std::stringstream &ss, uint32_t num) {
  if (num == UINT32_MAX) {
    ss << "?";
  } else {
    ss << num;
  }
}

std::string DiagnosticManager::mint_plain_message(const DiagMessage &msg) const {
  std::stringstream ss;
  uint32_t sl, sc, el, ec;

  {  // Print the filename and location information
    /// FIXME: Render filename
    ss << "??" << ":";

    auto default_if = std::pair<uint32_t, uint32_t>({UINT32_MAX, UINT32_MAX});
    auto beg = m_resolver->resolve(msg.m_start).value_or(default_if);
    auto end = m_resolver->resolve(msg.m_end).value_or(default_if);

    sl = beg.first;
    sc = beg.second;
    el = end.first;
    ec = end.second;

    if (sl != UINT32_MAX || sc != UINT32_MAX) {
      print_qsizeloc(ss, sl);
      ss << ":";
      print_qsizeloc(ss, sc);
      ss << ":";
    }

    ss << " ";
  }

  (void)ec;
  (void)el;

  switch (msg.m_type) {
    case IssueClass::Debug:
      ss << "debug";
      break;
    case IssueClass::Info:
      ss << "info";
      break;
    case IssueClass::Warn:
      ss << "warning";
      break;
    case IssueClass::Error:
      ss << "error";
      break;
    case IssueClass::FatalError:
      ss << "fatal error";
      break;
  }

  ss << ": " << msg.m_msg;

  if (msg.m_code != IssueCode::Info) {
    ss << " [-Werror=" << issue_info.left.at(msg.m_code).flagname << "]";
  }

  uint32_t res = UINT32_MAX; /*qlex_spanx(
       lx, msg.m_start, msg.m_end,
       [](const char *str, uint32_t len, uintptr_t x) {
         if (len > 100) {
           len = 100;
         }

         std::stringstream &ss = *reinterpret_cast<std::stringstream *>(x);
         ss << '\n' << std::string_view(str, len);
       },
       reinterpret_cast<uintptr_t>(&ss));*/
  if (res == UINT32_MAX) {
    ss << "\n# [failed to extract source code snippet]\n";
  }

  return ss.str();
}

std::string DiagnosticManager::mint_clang16_message(const DiagMessage &msg) const {
  std::stringstream ss;
  uint32_t sl, sc, el, ec;

  {  // Print the filename and location information
    ss << "\x1b[39;1m" << "??" << ":";

    auto default_if = std::pair<uint32_t, uint32_t>({UINT32_MAX, UINT32_MAX});
    auto beg = m_resolver->resolve(msg.m_start).value_or(default_if);
    auto end = m_resolver->resolve(msg.m_end).value_or(default_if);

    sl = beg.first;
    sc = beg.second;
    el = end.first;
    ec = end.second;

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

  switch (msg.m_type) {
    case IssueClass::Debug:
      ss << "\x1b[1mdebug:\x1b[0m " << msg.m_msg;
      if (msg.m_code != IssueCode::Info) {
        ss << " \x1b[39;1m[\x1b[0m\x1b[1m-Werror=" << issue_info.left.at(msg.m_code).flagname
           << "\x1b[0m\x1b[39;1m]\x1b[0m";
      }
      break;
    case IssueClass::Info:
      ss << "\x1b[37;1minfo:\x1b[0m " << msg.m_msg;
      if (msg.m_code != IssueCode::Info) {
        ss << " \x1b[39;1m[\x1b[0m\x1b[37;1m-Werror=" << issue_info.left.at(msg.m_code).flagname
           << "\x1b[0m\x1b[39;1m]\x1b[0m";
      }
      break;
    case IssueClass::Warn:
      ss << "\x1b[35;1mwarning:\x1b[0m " << msg.m_msg;
      if (msg.m_code != IssueCode::Info) {
        ss << " \x1b[39;1m[\x1b[0m\x1b[35;1m-Werror=" << issue_info.left.at(msg.m_code).flagname
           << "\x1b[0m\x1b[39;1m]\x1b[0m";
      }
      break;
    case IssueClass::Error:
      ss << "\x1b[31;1merror:\x1b[0m " << msg.m_msg;
      if (msg.m_code != IssueCode::Info) {
        ss << " \x1b[39;1m[\x1b[0m\x1b[31;1m-Werror=" << issue_info.left.at(msg.m_code).flagname
           << "\x1b[0m\x1b[39;1m]\x1b[0m";
      }
      break;
    case IssueClass::FatalError:
      ss << "\x1b[31;1;4mfatal error:\x1b[0m " << msg.m_msg;
      if (msg.m_code != IssueCode::Info) {
        ss << " \x1b[39;1m[\x1b[0m\x1b[31;1;4m-Werror=" << issue_info.left.at(msg.m_code).flagname
           << "\x1b[0m\x1b[39;1m]\x1b[0m";
      }
      break;
  }

  uint32_t res = UINT32_MAX; /* qlex_spanx(
       lx, msg.m_start, msg.m_end,
       [](const char *str, uint32_t len, uintptr_t x) {
         if (len > 100) {
           len = 100;
         }

         std::stringstream &ss = *reinterpret_cast<std::stringstream *>(x);
         ss << '\n' << std::string_view(str, len) << '\n';
       },
       reinterpret_cast<uintptr_t>(&ss)); */
  if (res == UINT32_MAX) {
    ss << "\n# [\x1b[35;1mfailed to extract source code snippet\x1b[0m]\n";
  }

  return ss.str();
}

///============================================================================///

using namespace nr;

uint64_t DiagMessage::hash() const {
  /* Not quite a PHF, but it's pretty close as long as there are not two many subject strings */
  /* In the worst case messages will be discarded, but that can be fixed by passing a parameter
     to disable deduplication */

  struct BitPack {
    IssueClass m_type : 3;
    IssueCode m_code : 10;
    uint64_t m_msg_trunc : 7;
    uint64_t m_end_trunc : 20;
    uint64_t m_start : 24;
  } __attribute__((packed)) bp;

  bp.m_type = m_type;
  bp.m_code = m_code;
  bp.m_msg_trunc = std::hash<std::string_view>{}(m_msg);
  bp.m_start = m_start;
  bp.m_end_trunc = m_end;

  return std::bit_cast<uint64_t>(bp);
}

void DiagnosticManager::report(IssueCode code, IssueClass level, std::span<std::string_view> params,
                               std::string_view filename, uint32_t start_offset,
                               uint32_t end_offset) {
  std::string message;
  for (auto p : params) {
    message += std::string(p) + "; ";
  }

  DiagMessage msg(message, level, code, start_offset, end_offset);

  { /* Prevent duplicates and maintain order of messages */
    auto hash = msg.hash();
    if (m_visited.contains(hash)) {
      return;
    }
    m_visited.insert(hash);
  }

  m_vec.push_back(std::move(msg));
}

size_t DiagnosticManager::render(DiagnosticMessageHandler handler, nr_diag_format_t style) {
  for (auto &msg : m_vec) {
    switch (style) {
      /**
       * @brief Code decimal serialization of the error code.
       * @example `801802`
       * @format <code>
       */
      case QXIR_DIAG_ASCII_ECODE: {
        handler(std::to_string(static_cast<uint64_t>(msg.m_code)), msg.m_type);
        break;
      }

      /**
       * @brief Code decimal serialization of the error code and source location.
       * @example `801802:1:1:/path/to/filename.q`
       * @format <code>:<line>:<col>:<path>
       *
       * @note UTF-8 characters in the path are preserved.
       */
      case QXIR_DIAG_UTF8_ECODE_LOC: {
        std::stringstream ss;
        ss << std::to_string(static_cast<uint64_t>(msg.m_code)) << ":";
        auto beg = m_resolver->resolve(msg.m_start);

        ss << beg->first << ":";
        ss << beg->second << ":";
        ss << "??";

        handler(ss.str(), msg.m_type);
        break;
      }

      /**
       * @brief Code decimal serialization of the error code and UTF-8 error message.
       * @example `801802:This is an UTF-8 error message.`
       * @format <code>:<utf8_message>
       */
      case QXIR_DIAG_UTF8_ECODE_ETEXT: {
        std::stringstream ss;
        ss << std::to_string(static_cast<uint64_t>(msg.m_code)) << ":";
        ss << msg.m_msg;

        handler(ss.str(), msg.m_type);
        break;
      }

      /**
       * @brief Unspecified format.
       * @note No-ANSI colors are included
       * @note Includes source location information as well as source code snippets (if
       * available).
       * @note Includes error messages and suggestions.
       * @note Basically, everything you expect from a mainstream compiler (except without
       * color).
       */
      case QXIR_DIAG_NOSTD_TTY_UTF8: {
        handler(mint_plain_message(msg), msg.m_type);
        break;
      }

      /**
       * @brief Unspecified format.
       * @note Similar to `QXIR_DIAG_NOSTD_TTY_UTF8`, but with undocumented differences.
       */
      case QXIR_DIAG_NONSTD_ANSI16_UTF8_FULL: {
        handler(mint_clang16_message(msg), msg.m_type);
        break;
      }

      /**
       * @brief Unspecified format.
       * @note Similar to `QXIR_DIAG_NOSTD_TTY_UTF8`, but with undocumented differences.
       */
      case QXIR_DIAG_NONSTD_ANSI256_UTF8_FULL: {
        handler(mint_clang16_message(msg), msg.m_type);
        break;
      }

      /**
       * @brief Unspecified format.
       * @note Similar to `QXIR_DIAG_NOSTD_TTY_UTF8`, but with undocumented differences.
       */
      case QXIR_DIAG_NONSTD_ANSIRGB_UTF8_FULL: {
        handler(mint_modern_message(msg), msg.m_type);
        break;
      }
    }
  }

  return m_vec.size();
}

static const std::unordered_map<IssueClass, nr_level_t> issue_class_map = {
    {IssueClass::Debug, QXIR_LEVEL_DEBUG},      {IssueClass::Info, QXIR_LEVEL_INFO},
    {IssueClass::Warn, QXIR_LEVEL_WARN},        {IssueClass::Error, QXIR_LEVEL_ERROR},
    {IssueClass::FatalError, QXIR_LEVEL_FATAL},
};

LIB_EXPORT size_t nr_diag_read(qmodule_t *nr, nr_diag_format_t format, nr_report_cb cb,
                               uintptr_t data) {
  if (!cb) {
    return nr->getDiag().size();
  }

  auto res = nr->getDiag().render(
      [cb, data](std::string_view v, IssueClass lvl) {
        cb(reinterpret_cast<const uint8_t *>(v.data()), v.size(), issue_class_map.at(lvl), data);
      },
      format);

  return res;
}

LIB_EXPORT size_t nr_diag_clear(qmodule_t *nr) {
  size_t n = nr->getDiag().size();
  nr->getDiag().clear();
  return n;
}

bool nr::report(IssueCode code, IssueClass type, uint32_t loc_start, uint32_t loc_end) {
  qcore_implement();
  // current->getDiag().push(channel, DiagMessage("", type, code, loc_start, loc_end));

  return true;
}

bool nr::report(IssueCode code, IssueClass type, std::string_view subject, uint32_t loc_start,
                uint32_t loc_end) {
  qcore_implement();
  // current->getDiag().push(channel, DiagMessage(subject, type, code, loc_start, loc_end));

  return true;
}
