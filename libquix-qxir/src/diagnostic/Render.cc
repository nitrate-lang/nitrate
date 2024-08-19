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

#include <core/Config.hh>
#include <diagnostic/Report.hh>
#include <sstream>

using namespace qxir::diag;

thread_local qmodule_t *g_qxir_inst;

///============================================================================///

std::string DiagnosticManager::mint_plain_message(const DiagMessage &msg) const {
  /// TODO: Implement this
  std::stringstream ss;
  ss << msg.msg;
  return ss.str();

  // std::stringstream ss;
  // ss << qlex_filename(m_qxir->lexer) << ":";
  // std::vector<std::pair<qlex_size, qlex_size>> range = {
  //   {qlex_line(m_qxir->lexer, msg.start), qlex_col(m_qxir->lexer, msg.start)},
  //   {qlex_line(m_qxir->lexer, msg.end),   qlex_col(m_qxir->lexer, msg.end)},
  // };

  // if (line != UINT32_MAX) {
  //   ss << line << ":";
  // } else {
  //   ss << "?:";
  // }

  // if (col != UINT32_MAX) {
  //   ss << col << ": ";
  // } else {
  //   ss << "?: ";
  // }
  // ss << "error: " << msg.msg << " [";

  // switch (msg.type) {
  //   case MessageType::BadTree:
  //     ss << "BadTree";
  //     break;
  //   case MessageType::FatalError:
  //     ss << "FatalError";
  //     break;
  // }

  // ss << "]\n";

  // qlex_size offset;
  // char *snippet = qlex_snippet(m_qxir->lexer, msg.tok, &offset);
  // if (!snippet) {
  //   return ss.str();
  // }

  // ss << snippet << "\n";
  // for (qlex_size i = 0; i < offset; i++) {
  //   ss << " ";
  // }
  // ss << "^\n";
  // free(snippet);

  // return ss.str();
}

std::string DiagnosticManager::mint_clang16_message(const DiagMessage &msg) const {
  /// TODO: Implement this
  std::stringstream ss;
  ss << msg.msg;
  return ss.str();

  // std::stringstream ss;
  // ss << "\x1b[37;1m" << qlex_filename(m_qxir->lexer) << ":";
  // qlex_size line = qlex_line(m_qxir->lexer, qlex_begin(msg.tok));
  // qlex_size col = qlex_col(m_qxir->lexer, qlex_begin(msg.tok));

  // if (line != UINT32_MAX) {
  //   ss << line << ":";
  // } else {
  //   ss << "?:";
  // }

  // if (col != UINT32_MAX) {
  //   ss << col << ":\x1b[0m ";
  // } else {
  //   ss << "?:\x1b[0m ";
  // }

  // ss << "\x1b[31;1merror:\x1b[0m \x1b[37;1m" << msg.msg << " [";

  // switch (msg.type) {
  //   case MessageType::BadTree:
  //     ss << "BadTree";
  //     break;
  //   case MessageType::FatalError:
  //     ss << "FatalError";
  //     break;
  // }

  // ss << "]\x1b[0m\n";

  // qlex_size offset;
  // char *snippet = qlex_snippet(m_qxir->lexer, msg.tok, &offset);
  // if (!snippet) {
  //   return ss.str();
  // }

  // ss << snippet << "\n";
  // for (qlex_size i = 0; i < offset; i++) {
  //   ss << " ";
  // }
  // ss << "\x1b[32;1m^\x1b[0m\n";
  // free(snippet);

  // return ss.str();
}

std::string DiagnosticManager::mint_clang_truecolor_message(const DiagMessage &msg) const {
  return mint_clang16_message(msg); /* For now this will do okay */
}

///============================================================================///

using namespace qxir::diag;

void DiagnosticManager::push(DiagMessage &&msg) { m_msgs.push_back(std::move(msg)); }

size_t DiagnosticManager::render(DiagnosticMessageHandler handler, FormatStyle style) const {
  switch (style) {
    case FormatStyle::ClangPlain:
      for (const auto &msg : m_msgs) {
        handler(mint_plain_message(msg).c_str());
      }
      break;
    case FormatStyle::Clang16Color:
      for (const auto &msg : m_msgs) {
        handler(mint_clang16_message(msg).c_str());
      }
      break;
    case FormatStyle::ClangTrueColor:
      for (const auto &msg : m_msgs) {
        handler(mint_clang_truecolor_message(msg).c_str());
      }
      break;
    default:
      qcore_panicf("Unsupported diagnostic format style: %d", static_cast<int>(style));
  }

  return m_msgs.size();
}

namespace qxir::diag {
  void install_reference(qmodule_t *qxir) noexcept { g_qxir_inst = qxir; }

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
    diag.msg = msg;
    diag.start = start;
    diag.end = end;
    diag.type = MessageType::BadTree;

    g_qxir_inst->getDiag().push(std::move(diag));

    if (g_qxir_inst->getConf()->has(QQV_FASTERROR, QQV_ON)) {
      throw SyntaxError();
    }
  }

  void badtree(const qparse::Node *node, std::string_view fmt, ...) {
    if (!node) {
      qcore_panic("badtree: node is NULL");
    }

    va_list args;
    va_start(args, fmt);
    badtree_impl(node->get_start_pos(), node->get_end_pos(), fmt, args);
    va_end(args);
  }
}  // namespace qxir::diag

LIB_EXPORT size_t qxir_diag_read(qmodule_t *qxir, qxir_audit_ticket_t ticket,
                                 qxir_diag_format_t format, qxir_report_cb cb, uintptr_t data) {
  /// TODO:
  qcore_implement(__func__);
}

LIB_EXPORT size_t qxir_diag_clear(qmodule_t *qxir, qxir_audit_ticket_t ticket) {
  /// TODO:
  qcore_implement(__func__);
}