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

#ifndef __QUIX_PARSER_REPORT_H__
#define __QUIX_PARSER_REPORT_H__

#include <nitrate-lexer/Token.h>
#include <nitrate-parser/Parser.h>

#include <cstdarg>
#include <functional>
#include <memory>
#include <queue>
#include <string_view>

namespace qparse::diag {
  class SyntaxError : public std::runtime_error {
  public:
    SyntaxError() : std::runtime_error("") {}
  };

  enum class MessageType {
    Syntax,
    FatalError,
  };

  enum ControlFlow {
    cont, /* Continue parsing */
    stop, /* Stop parsing (throw an error) */
  };

  enum class FormatStyle {
    Clang16Color,   /* Clang-like 16 color diagnostic format */
    ClangPlain,     /* Clang-like plain text diagnostic format */
    ClangTrueColor, /* Clang-like RGB TrueColor diagnostic format */
    Default = Clang16Color,
  };

  typedef std::function<void(const char *)> DiagnosticMessageHandler;

  struct DiagMessage {
    std::string msg;
    qlex_tok_t tok;
    MessageType type;
  };

  class DiagnosticManager {
    qparse_t *m_parser;
    std::vector<DiagMessage> m_msgs;

    std::string mint_clang16_message(const DiagMessage &msg) const;
    std::string mint_plain_message(const DiagMessage &msg) const;
    std::string mint_clang_truecolor_message(const DiagMessage &msg) const;

  public:
    void push(DiagMessage &&msg);
    size_t render(DiagnosticMessageHandler handler, FormatStyle style) const;

    void set_ctx(qparse_t *parser) { m_parser = parser; }
  };

  /* Set reference to the current parser */
  void install_reference(qparse_t *parser);

  /**
   * @brief Report a syntax error
   */
  void syntax(const qlex_tok_t &tok, std::string_view fmt, ...);
};  // namespace qparse::diag

#endif  // __QUIX_PARSER_REPORT_H__
