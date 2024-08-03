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

#include <Impl.h>
#include <Report.h>
#include <quix-core/Error.h>
#include <quix-parser/Parser.h>

#include <sstream>

using namespace qparse::diag;

thread_local qparse_t *g_parser_inst;

/*
/app/libquix-parser/src/parser/InlineAsmParser.cc:120:73: warning: unused parameter 'node'
[-Wunused-parameter] bool qparse::parser::parse_inline_asm(qparse_t &job, qlex_t *rd, Stmt **node) {
                                                                        ^
*/

///============================================================================///

std::string DiagnosticManager::mint_plain_message(const DiagMessage &msg) const {
  std::stringstream ss;
  ss << qlex_filename(m_parser->lexer) << ":";
  ss << qlex_line(m_parser->lexer, msg.tok.loc) << ":";
  ss << qlex_col(m_parser->lexer, msg.tok.loc) << ": ";
  ss << "error: " << msg.msg << " [SyntaxError]\n";

  uint32_t offset;
  char *snippet = qlex_snippet(m_parser->lexer, msg.tok, &offset);
  if (!snippet) {
    return ss.str();
  }

  ss << snippet << "\n";
  for (uint32_t i = 0; i < offset; i++) {
    ss << " ";
  }
  ss << "^\n";
  free(snippet);

  return ss.str();
}

std::string DiagnosticManager::mint_clang16_message(const DiagMessage &msg) const {
  std::stringstream ss;
  ss << "\x1b[37;1m" << qlex_filename(m_parser->lexer) << ":";
  ss << qlex_line(m_parser->lexer, msg.tok.loc) << ":";
  ss << qlex_col(m_parser->lexer, msg.tok.loc) << ":\x1b[0m ";
  ss << "\x1b[31;1merror:\x1b[0m \x1b[37;1m" << msg.msg << " [SyntaxError]\x1b[0m\n";

  uint32_t offset;
  char *snippet = qlex_snippet(m_parser->lexer, msg.tok, &offset);
  if (!snippet) {
    return ss.str();
  }

  ss << snippet << "\n";
  for (uint32_t i = 0; i < offset; i++) {
    ss << " ";
  }
  ss << "\x1b[32;1m^\x1b[0m\n";
  free(snippet);

  return ss.str();
}

std::string DiagnosticManager::mint_clang_truecolor_message(const DiagMessage &msg) const {
  return mint_clang16_message(msg); /* For now this will do okay */
}

///============================================================================///

using namespace qparse::diag;

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

namespace qparse::diag {
  void install_reference(qparse_t *parser) { g_parser_inst = parser; }

  void syntax_impl(const qlex_tok_t &tok, std::string_view fmt, va_list args) {
    std::string msg;

    {  // Format the message
      char *c_msg = nullptr;
      vasprintf(&c_msg, fmt.data(), args);
      msg = c_msg;
      free(c_msg);
    }

    DiagMessage diag;
    diag.msg = msg;
    diag.tok = tok;
    diag.type = MessageType::Syntax;

    g_parser_inst->impl->diag.push(std::move(diag));
    g_parser_inst->failed = true;
  }

  void syntax(const qlex_tok_t &tok, std::string_view fmt, ...) {
    va_list args;
    va_start(args, fmt);
    syntax_impl(tok, fmt, args);
    va_end(args);
  }
}  // namespace qparse::diag
