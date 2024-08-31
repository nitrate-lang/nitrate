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

#define QPARSE_USE_CPP_API

#include <quix-lexer/Token.h>
#include <quix-parser/Node.h>
#include <quix-qxir/IR.h>

#include <cstdarg>
#include <functional>
#include <stdexcept>
#include <string_view>

namespace qxir::diag {
  class SyntaxError : public std::runtime_error {
  public:
    SyntaxError() : std::runtime_error("") {}
  };

  enum class IssueClass {
    Debug = 0,
    Info,
    Warn,
    Error,
    FatalError,
  };

  enum class IssueCode {
    Default = 0,

    SignalReceived,
    PTreeInvalid,
    DSPolyCyclicRef,
    DSNullPtr,
    DSBadType,
    DSMissingMod,
  };

  typedef std::function<void(std::string_view)> DiagnosticMessageHandler;

  struct DiagMessage {
    std::string msg;
    qlex_loc_t start, end;
    IssueClass type;
    IssueCode code;

    DiagMessage(std::string_view msg = "", IssueClass type = IssueClass::Debug,
                IssueCode code = IssueCode::Default, qlex_loc_t start = {0}, qlex_loc_t end = {0})
        : msg(msg), start(start), end(end), type(type), code(code) {}
  };

  class DiagnosticManager {
    qmodule_t *m_qxir;
    std::unordered_map<qxir_audit_ticket_t, std::vector<DiagMessage>> m_msgs;
    qxir_audit_ticket_t m_last_ticket;

    std::string mint_clang16_message(const DiagMessage &msg) const;
    std::string mint_plain_message(const DiagMessage &msg) const;
    std::string mint_clang_truecolor_message(const DiagMessage &msg) const;
    size_t dump_diagnostic_vector(std::vector<DiagMessage> &vec, DiagnosticMessageHandler handler,
                                  qxir_diag_format_t style);

  public:
    DiagnosticManager() {
      m_qxir = nullptr;
      m_msgs[QXIR_AUDIT_CONV] = {};
      m_last_ticket = QXIR_AUDIT_CONV;
    }

    void push(qxir_audit_ticket_t ticket, DiagMessage &&msg);
    size_t render(qxir_audit_ticket_t ticket, DiagnosticMessageHandler handler,
                  qxir_diag_format_t style);

    void set_ctx(qmodule_t *qxir) { m_qxir = qxir; }

    size_t clear(qxir_audit_ticket_t t) {
      if (t == QXIR_AUDIT_ALL) {
        size_t n = 0;
        for (auto &[_, msgs] : m_msgs) {
          n += msgs.size();
          msgs.clear();
        }
        return n;
      } else if (t == QXIR_AUDIT_LAST) {
        size_t n = m_msgs[m_last_ticket].size();
        m_msgs[m_last_ticket].clear();
        return n;
      } else {
        if (!m_msgs.contains(t)) {
          return 0;
        }

        size_t n = m_msgs[t].size();
        m_msgs[t].clear();
        return n;
      }
    }

    size_t count(qxir_audit_ticket_t t) {
      if (t == QXIR_AUDIT_ALL) {
        size_t n = 0;
        for (const auto &[_, msgs] : m_msgs) {
          n += msgs.size();
        }
        return n;
      } else if (t == QXIR_AUDIT_LAST) {
        return m_msgs[m_last_ticket].size();
      } else {
        if (!m_msgs.contains(t)) {
          return 0;
        }

        return m_msgs[t].size();
      }
    }
  };

  /* Set reference to the current qxir */
  void install_reference(qmodule_t *qxir) noexcept;

  /**
   * @brief Report a syntax error
   */
  void badtree(const qparse::Node *node, std::string_view fmt, ...);
};  // namespace qxir::diag

#endif  // __QUIX_QXIR_REPORT_H__
