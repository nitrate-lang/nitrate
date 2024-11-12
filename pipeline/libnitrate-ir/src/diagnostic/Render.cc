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

using namespace nr::diag;

///============================================================================///

static void print_qsizeloc(std::stringstream &ss, qlex_size num) {
  if (num == UINT32_MAX) {
    ss << "?";
  } else {
    ss << num;
  }
}

std::string DiagnosticManager::mint_plain_message(const DiagMessage &msg) const {
  std::stringstream ss;
  qlex_t *lx = m_nr->getLexer();
  qlex_size sl, sc, el, ec;

  {  // Print the filename and location information
    ss << qlex_filename(lx) << ":";

    sl = qlex_line(lx, msg.m_start);
    sc = qlex_col(lx, msg.m_start);
    el = qlex_line(lx, msg.m_end);
    ec = qlex_col(lx, msg.m_end);

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

  qlex_size res = qlex_spanx(
      lx, msg.m_start, msg.m_end,
      [](const char *str, qlex_size len, uintptr_t x) {
        if (len > 100) {
          len = 100;
        }

        std::stringstream &ss = *reinterpret_cast<std::stringstream *>(x);
        ss << '\n' << std::string_view(str, len);
      },
      reinterpret_cast<uintptr_t>(&ss));
  if (res == UINT32_MAX) {
    ss << "\n# [failed to extract source code snippet]\n";
  }

  return ss.str();
}

std::string DiagnosticManager::mint_clang16_message(const DiagMessage &msg) const {
  std::stringstream ss;
  qlex_t *lx = m_nr->getLexer();
  qlex_size sl, sc, el, ec;

  {  // Print the filename and location information
    ss << "\x1b[39;1m" << qlex_filename(lx) << ":";

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

  qlex_size res = qlex_spanx(
      lx, msg.m_start, msg.m_end,
      [](const char *str, qlex_size len, uintptr_t x) {
        if (len > 100) {
          len = 100;
        }

        std::stringstream &ss = *reinterpret_cast<std::stringstream *>(x);
        ss << '\n' << std::string_view(str, len) << '\n';
      },
      reinterpret_cast<uintptr_t>(&ss));
  if (res == UINT32_MAX) {
    ss << "\n# [\x1b[35;1mfailed to extract source code snippet\x1b[0m]\n";
  }

  return ss.str();
}

///============================================================================///

using namespace nr::diag;

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
  bp.m_start = m_start.tag;
  bp.m_end_trunc = m_end.tag;

  return std::bit_cast<uint64_t>(bp);
}

void DiagnosticManager::push(nr_audit_ticket_t ticket, DiagMessage &&msg) {
  auto &chan = m_msgs[ticket];

  switch (msg.m_type) {
    case IssueClass::Debug:
      break;
    case IssueClass::Info:
      break;
    case IssueClass::Warn:
      break;
    case IssueClass::Error:
      current->setFailbit(true);
      break;
    case IssueClass::FatalError:
      current->setFailbit(true);
      break;
  }

  { /* Prevent duplicates and maintain order of messages */
    auto hash = msg.hash();
    if (chan.visited.contains(hash)) {
      return;
    }
    chan.visited.insert(hash);
  }

  chan.vec.push_back(std::move(msg));
}

size_t DiagnosticManager::dump_diagnostic_vector(std::vector<DiagMessage> &vec,
                                                 DiagnosticMessageHandler handler,
                                                 nr_diag_format_t style) {
  for (auto &msg : vec) {
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
        ss << qlex_line(m_nr->getLexer(), msg.m_start) << ":";
        ss << qlex_col(m_nr->getLexer(), msg.m_start) << ":";
        ss << qlex_filename(m_nr->getLexer());

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

  return vec.size();
}

size_t DiagnosticManager::render(nr_audit_ticket_t ticket, DiagnosticMessageHandler handler,
                                 nr_diag_format_t style) {
  if (ticket == QXIR_AUDIT_ALL) {
    size_t n = 0;
    for (auto &[_, msgs] : m_msgs) {
      n += dump_diagnostic_vector(msgs.vec, handler, style);
    }
    return n;
  } else if (ticket == QXIR_AUDIT_LAST) {
    return dump_diagnostic_vector(m_msgs[m_last_ticket].vec, handler, style);
  } else {
    if (!m_msgs.contains(ticket)) {
      return 0;
    }

    return dump_diagnostic_vector(m_msgs[ticket].vec, handler, style);
  }
}

namespace nr::diag {
  void badtree_impl(qlex_loc_t start, qlex_loc_t end, std::string_view fmt, va_list args) {
    std::string msg;

    {  // Format the message
      char *c_msg = nullptr;
      int r = vasprintf(&c_msg, fmt.data(), args);
      if (r < 0) {
        qcore_panic("Failed to format diagnostic message");
      }
      msg = c_msg;
      free(c_msg);
    }

    report(IssueCode::PTreeInvalid, IssueClass::Error, fmt, start, end);

    if (current->getConf()->has(QQV_FASTERROR, QQV_ON)) {
      throw SyntaxError();
    }
  }

  void badtree(qparse::Node *node, std::string_view fmt, ...) {
    if (!node) {
      qcore_panic("badtree: node is NULL");
    }

    current->setFailbit(true);

    if (!current->isDiagnosticsEnabled()) {
      return;
    }

    va_list args;
    va_start(args, fmt);
    badtree_impl(node->get_start_pos(), node->get_end_pos(), fmt, args);
    va_end(args);
  }
}  // namespace nr::diag

static const std::unordered_map<IssueClass, nr_level_t> issue_class_map = {
    {IssueClass::Debug, QXIR_LEVEL_DEBUG},      {IssueClass::Info, QXIR_LEVEL_INFO},
    {IssueClass::Warn, QXIR_LEVEL_WARN},        {IssueClass::Error, QXIR_LEVEL_ERROR},
    {IssueClass::FatalError, QXIR_LEVEL_FATAL},
};

LIB_EXPORT size_t nr_diag_read(qmodule_t *nr, nr_audit_ticket_t ticket, nr_diag_format_t format,
                               nr_report_cb cb, uintptr_t data) {
  if (!cb) {
    nr->getDiag().count(ticket);
  }

  auto res = nr->getDiag().render(
      ticket,
      [cb, data](std::string_view v, IssueClass lvl) {
        cb(reinterpret_cast<const uint8_t *>(v.data()), v.size(), issue_class_map.at(lvl), data);
      },
      format);

  return res;
}

LIB_EXPORT size_t nr_diag_clear(qmodule_t *nr, nr_audit_ticket_t ticket) {
  return nr->getDiag().clear(ticket);
}

bool nr::diag::report(diag::IssueCode code, diag::IssueClass type, qlex_loc_t loc_start,
                      qlex_loc_t loc_end, int channel) {
  current->getDiag().push(channel, diag::DiagMessage("", type, code, loc_start, loc_end));

  return true;
}

bool nr::diag::report(diag::IssueCode code, diag::IssueClass type, std::string_view subject,
                      qlex_loc_t loc_start, qlex_loc_t loc_end, int channel) {
  current->getDiag().push(channel, diag::DiagMessage(subject, type, code, loc_start, loc_end));

  return true;
}
