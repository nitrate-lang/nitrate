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

#ifndef __QUIX_QXIR_REPORT_H__
#define __QUIX_QXIR_REPORT_H__

#include <quix-lexer/Token.h>
#include <quix-qxir/QXIR.h>

#include <cstdarg>
#include <functional>
#include <memory>
#include <queue>
#include <span>
#include <string_view>

namespace qxir::diag {
  class SyntaxError : public std::runtime_error {
  public:
    SyntaxError() : std::runtime_error("") {}
  };

  enum class MessageType {
    BadTree,
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
    qlex_loc_t start, end;
    MessageType type;
  };

  class DiagnosticManager {
    qxir_t *m_qxir;
    std::vector<DiagMessage> m_msgs;

    std::string mint_clang16_message(const DiagMessage &msg) const;
    std::string mint_plain_message(const DiagMessage &msg) const;
    std::string mint_clang_truecolor_message(const DiagMessage &msg) const;

  public:
    void push(DiagMessage &&msg);
    size_t render(DiagnosticMessageHandler handler, FormatStyle style) const;

    void set_ctx(qxir_t *qxir) { m_qxir = qxir; }
  };

  /* Set reference to the current qxir */
  void install_reference(qxir_t *qxir);

  /**
   * @brief Report a syntax error
   */
  void badtree(const qparse::Node *node, std::string_view fmt, ...);
};  // namespace qxir::diag

#endif  // __QUIX_QXIR_REPORT_H__
