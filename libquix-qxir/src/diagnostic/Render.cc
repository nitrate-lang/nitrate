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

#define __QXIR_IMPL__
#define QPARSE_USE_CPP_API

#include <core/LibMacro.h>
#include <quix-core/Error.h>
#include <quix-qxir/Module.h>
#include <quix-qxir/Node.h>

#include <boost/bimap.hpp>
#include <core/Config.hh>
#include <diagnostic/Report.hh>
#include <sstream>

using namespace qxir::diag;

template <typename L, typename R>
boost::bimap<L, R> make_bimap(std::initializer_list<typename boost::bimap<L, R>::value_type> list) {
  return boost::bimap<L, R>(list.begin(), list.end());
}

static const boost::bimap<IssueCode, std::string_view> issue_code_bimap =
    make_bimap<IssueCode, std::string_view>({
        {IssueCode::Default, "-Wdefault"},
        {IssueCode::PTreeInvalid, "-Werror=ptree-invalid"},
        {IssueCode::SignalReceived, "-Werror=signal-recv"},
        {IssueCode::DSPolyCyclicRef, "-Werror=ds-cyclic-ref"},
        {IssueCode::DSNullPtr, "-Werror=ds-nullptr"},
        {IssueCode::DSBadType, "-Werror=ds-bad-type"},
        {IssueCode::DSMissingMod, "-Werror=ds-missing-mod"},
    });

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
  qlex_t *lx = m_qxir->getLexer();
  qlex_size sl, sc, el, ec;

  {  // Print the filename and location information
    ss << qlex_filename(lx) << ":";

    sl = qlex_line(lx, msg.start);
    sc = qlex_col(lx, msg.start);
    el = qlex_line(lx, msg.end);
    ec = qlex_col(lx, msg.end);

    print_qsizeloc(ss, sl);
    ss << ":";
    print_qsizeloc(ss, sc);
    ss << ": ";
  }

  (void)ec;
  (void)el;

  switch (msg.type) {
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

  ss << ": " << msg.msg << " [" << issue_code_bimap.left.at(msg.code) << "]";

  qlex_spanx(
      lx, msg.start, msg.end,
      [](const char *str, qlex_size len, uintptr_t x) {
        if (len > 100) {
          len = 100;
        }

        std::stringstream &ss = *reinterpret_cast<std::stringstream *>(x);
        ss << '\n' << str;
      },
      reinterpret_cast<uintptr_t>(&ss));

  return ss.str();
}

std::string DiagnosticManager::mint_clang16_message(const DiagMessage &msg) const {
  std::stringstream ss;
  qlex_t *lx = m_qxir->getLexer();
  qlex_size sl, sc, el, ec;

  {  // Print the filename and location information
    ss << "\x1b[39;1m" << qlex_filename(lx) << ":";

    sl = qlex_line(lx, msg.start);
    sc = qlex_col(lx, msg.start);
    el = qlex_line(lx, msg.end);
    ec = qlex_col(lx, msg.end);

    print_qsizeloc(ss, sl);
    ss << ":";
    print_qsizeloc(ss, sc);
    ss << ":\x1b[0m ";
  }

  (void)ec;
  (void)el;

  switch (msg.type) {
    case IssueClass::Debug:
      ss << "\x1b[1mdebug:\x1b[0m " << msg.msg << " \x1b[39;1m[\x1b[0m\x1b[1m"
         << issue_code_bimap.left.at(msg.code) << "\x1b[0m\x1b[39;1m]\x1b[0m";
      break;
    case IssueClass::Info:
      ss << "\x1b[37;1minfo:\x1b[0m " << msg.msg << " \x1b[39;1m[\x1b[0m\x1b[37;1m"
         << issue_code_bimap.left.at(msg.code) << "\x1b[0m\x1b[39;1m]\x1b[0m";
      break;
    case IssueClass::Warn:
      ss << "\x1b[35;1mwarning:\x1b[0m " << msg.msg << " \x1b[39;1m[\x1b[0m\x1b[35;1m"
         << issue_code_bimap.left.at(msg.code) << "\x1b[0m\x1b[39;1m]\x1b[0m";
      break;
    case IssueClass::Error:
      ss << "\x1b[31;1merror:\x1b[0m " << msg.msg << " \x1b[39;1m[\x1b[0m\x1b[31;1m"
         << issue_code_bimap.left.at(msg.code) << "\x1b[0m\x1b[39;1m]\x1b[0m";
      break;
    case IssueClass::FatalError:
      ss << "\x1b[31;1;4mfatal error:\x1b[0m " << msg.msg << " \x1b[39;1m[\x1b[0m\x1b[31;1;4m"
         << issue_code_bimap.left.at(msg.code) << "\x1b[0m\x1b[39;1m]\x1b[0m";
      break;
  }

  qlex_spanx(
      lx, msg.start, msg.end,
      [](const char *str, qlex_size len, uintptr_t x) {
        if (len > 100) {
          len = 100;
        }

        std::stringstream &ss = *reinterpret_cast<std::stringstream *>(x);
        ss << '\n' << str;
      },
      reinterpret_cast<uintptr_t>(&ss));

  return ss.str();
}

std::string DiagnosticManager::mint_clang_truecolor_message(const DiagMessage &msg) const {
  return mint_clang16_message(msg); /* For now this will do okay */
}

///============================================================================///

using namespace qxir::diag;

void DiagnosticManager::push(qxir_audit_ticket_t ticket, DiagMessage &&msg) {
  m_msgs[ticket].push_back(std::move(msg));
}

size_t DiagnosticManager::dump_diagnostic_vector(std::vector<DiagMessage> &vec,
                                                 DiagnosticMessageHandler handler,
                                                 qxir_diag_format_t style) {
  for (auto &msg : vec) {
    switch (style) {
      /**
       * @brief Code decimal serialization of the error code.
       * @example `801802`
       * @format <code>
       */
      case QXIR_DIAG_ASCII_ECODE: {
        handler(std::to_string(static_cast<uint64_t>(msg.code)));
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
        ss << std::to_string(static_cast<uint64_t>(msg.code)) << ":";
        ss << qlex_line(m_qxir->getLexer(), msg.start) << ":";
        ss << qlex_col(m_qxir->getLexer(), msg.start) << ":";
        ss << qlex_filename(m_qxir->getLexer());

        handler(ss.str());
        break;
      }

      /**
       * @brief Code decimal serialization of the error code and UTF-8 error message.
       * @example `801802:This is an UTF-8 error message.`
       * @format <code>:<utf8_message>
       */
      case QXIR_DIAG_UTF8_ECODE_ETEXT: {
        std::stringstream ss;
        ss << std::to_string(static_cast<uint64_t>(msg.code)) << ":";
        ss << msg.msg;

        handler(ss.str());
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
        handler(mint_plain_message(msg));
        break;
      }

      /**
       * @brief Unspecified format.
       * @note Similar to `QXIR_DIAG_NOSTD_TTY_UTF8`, but with undocumented differences.
       */
      case QXIR_DIAG_NONSTD_ANSI16_UTF8_FULL: {
        handler(mint_clang16_message(msg));
        break;
      }

      /**
       * @brief Unspecified format.
       * @note Similar to `QXIR_DIAG_NOSTD_TTY_UTF8`, but with undocumented differences.
       */
      case QXIR_DIAG_NONSTD_ANSI256_UTF8_FULL: {
        handler(mint_clang16_message(msg));
        break;
      }

      /**
       * @brief Unspecified format.
       * @note Similar to `QXIR_DIAG_NOSTD_TTY_UTF8`, but with undocumented differences.
       */
      case QXIR_DIAG_NONSTD_ANSIRGB_UTF8_FULL: {
        handler(mint_clang_truecolor_message(msg));
        break;
      }

      default: {
        handler(mint_plain_message(msg));
        break;
      }
    }
  }

  return vec.size();
}

size_t DiagnosticManager::render(qxir_audit_ticket_t ticket, DiagnosticMessageHandler handler,
                                 qxir_diag_format_t style) {
  if (ticket == QXIR_AUDIT_ALL) {
    size_t n = 0;
    for (auto &[_, msgs] : m_msgs) {
      n += dump_diagnostic_vector(msgs, handler, style);
    }
    return n;
  } else if (ticket == QXIR_AUDIT_LAST) {
    return dump_diagnostic_vector(m_msgs[m_last_ticket], handler, style);
  } else {
    if (!m_msgs.contains(ticket)) {
      return 0;
    }

    return dump_diagnostic_vector(m_msgs[ticket], handler, style);
  }
}

namespace qxir::diag {
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

    DiagMessage diag;
    diag.msg = std::move(msg);
    diag.start = start;
    diag.end = end;
    diag.type = IssueClass::Error;
    diag.code = IssueCode::PTreeInvalid;

    current->getDiag().push(QXIR_AUDIT_CONV, std::move(diag));

    if (current->getConf()->has(QQV_FASTERROR, QQV_ON)) {
      throw SyntaxError();
    }
  }

  void badtree(const qparse::Node *node, std::string_view fmt, ...) {
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
}  // namespace qxir::diag

LIB_EXPORT size_t qxir_diag_read(qmodule_t *qxir, qxir_audit_ticket_t ticket,
                                 qxir_diag_format_t format, qxir_report_cb cb, uintptr_t data) {
  if (!cb) {
    qxir->getDiag().count(ticket);
  }

  auto res = qxir->getDiag().render(
      ticket,
      [cb, data](std::string_view v) {
        cb(reinterpret_cast<const uint8_t *>(v.data()), v.size(), data);
      },
      format);

  return res;
}

LIB_EXPORT size_t qxir_diag_clear(qmodule_t *qxir, qxir_audit_ticket_t ticket) {
  return qxir->getDiag().clear(ticket);
}
