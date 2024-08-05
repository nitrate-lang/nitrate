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

#define __QPARSE_IMPL__

#include <QXIRImpl.h>
#include <QXIRReport.h>
#include <quix-core/Error.h>
#include <quix-qxir/QXIR.h>

#include <sstream>

using namespace qxir::diag;

thread_local qxir_t *g_qxir_inst;

///============================================================================///

std::string DiagnosticManager::mint_plain_message(const DiagMessage &msg) const {
  /// TODO: Implement plain text diagnostic message formatting

  throw std::runtime_error("Not implemented");

  // std::stringstream ss;
  // ss << qlex_filename(m_qxir->lexer) << ":";
  // uint32_t line = qlex_line(m_qxir->lexer, msg.tok.loc);
  // uint32_t col = qlex_col(m_qxir->lexer, msg.tok.loc);

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
  //   case MessageType::Syntax:
  //     ss << "SyntaxError";
  //     break;
  //   case MessageType::FatalError:
  //     ss << "FatalError";
  //     break;
  // }

  // ss << "]\n";

  // uint32_t offset;
  // char *snippet = qlex_snippet(m_qxir->lexer, msg.tok, &offset);
  // if (!snippet) {
  //   return ss.str();
  // }

  // ss << snippet << "\n";
  // for (uint32_t i = 0; i < offset; i++) {
  //   ss << " ";
  // }
  // ss << "^\n";
  // free(snippet);

  // return ss.str();
}

std::string DiagnosticManager::mint_clang16_message(const DiagMessage &msg) const {
  /// TODO: Implement plain text diagnostic message formatting

  throw std::runtime_error("Not implemented");

  // std::stringstream ss;
  // ss << "\x1b[37;1m" << qlex_filename(m_qxir->lexer) << ":";
  // uint32_t line = qlex_line(m_qxir->lexer, msg.tok.loc);
  // uint32_t col = qlex_col(m_qxir->lexer, msg.tok.loc);

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
  //   case MessageType::Syntax:
  //     ss << "SyntaxError";
  //     break;
  //   case MessageType::FatalError:
  //     ss << "FatalError";
  //     break;
  // }

  // ss << "]\x1b[0m\n";

  // uint32_t offset;
  // char *snippet = qlex_snippet(m_qxir->lexer, msg.tok, &offset);
  // if (!snippet) {
  //   return ss.str();
  // }

  // ss << snippet << "\n";
  // for (uint32_t i = 0; i < offset; i++) {
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
  void install_reference(qxir_t *qxir) { g_qxir_inst = qxir; }

  void syntax_impl(const qlex_tok_t &tok, std::string_view fmt, va_list args) {
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
    diag.tok = tok;
    diag.type = MessageType::Syntax;

    g_qxir_inst->impl->diag.push(std::move(diag));
    g_qxir_inst->failed = true;

    if (g_qxir_inst->conf->has(QQV_FASTERROR, QQV_ON)) {
      throw SyntaxError();
    }
  }

  void syntax(const qlex_tok_t &tok, std::string_view fmt, ...) {
    va_list args;
    va_start(args, fmt);
    syntax_impl(tok, fmt, args);
    va_end(args);
  }
}  // namespace qxir::diag
