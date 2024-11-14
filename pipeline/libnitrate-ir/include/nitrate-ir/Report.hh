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

#ifndef __NITRATE_QXIR_REPORT_H__
#define __NITRATE_QXIR_REPORT_H__

#include <nitrate-ir/IR.h>
#include <nitrate-lexer/Token.h>

#include <boost/bimap.hpp>
#include <cstdarg>
#include <functional>
#include <span>
#include <stdexcept>
#include <string_view>
#include <unordered_set>

namespace qparse {
  class Node;
}

namespace nr {
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
    CompilerError,
    SignalReceived,
    PTreeInvalid,
    DSPolyCyclicRef,
    DSNullPtr,
    DSBadType,
    DSBadTmpNode,

    FunctionRedefinition,
    VariableRedefinition,
    UnknownFunction,
    TooManyArguments,
    UnknownArgument,
    TypeInference,
    NameManglingTypeInfer,
    UnexpectedUndefLiteral,

    UnknownType,
    UnresolvedIdentifier,
    TypeRedefinition,
    BadCast,

    MissingReturn,

    Info,
  };

  struct IssueInfo {
    std::string_view flagname;
    std::string overview;
    std::vector<std::string_view> hints;

    bool operator<(const IssueInfo &rhs) const { return flagname < rhs.flagname; }
  };

  extern const boost::bimap<IssueCode, IssueInfo> issue_info;

  typedef std::function<void(std::string_view, IssueClass)> DiagnosticMessageHandler;

  class IReport {
  public:
    virtual void report(IssueCode code, IssueClass level, std::span<std::string_view> params = {},
                        uint32_t start_offset = 1, uint32_t end_offset = 0,
                        std::string_view filename = "") = 0;

    void report(IssueCode code, IssueClass level, std::string_view message,
                std::pair<uint32_t, uint32_t> loc = {0, 0}, std::string_view filename = "") {
      std::array<std::string_view, 1> x = {message};
      report(code, level, x, loc.first, loc.second, filename);
    };
    virtual ~IReport() = default;
  };

  class IOffsetResolver {
  public:
    virtual std::optional<std::pair<uint32_t, uint32_t>> resolve(uint32_t offset) noexcept = 0;
    virtual ~IOffsetResolver() = default;
  };

  struct DiagMessage {
    std::string m_msg;
    uint32_t m_start, m_end;
    IssueClass m_type;
    IssueCode m_code;

    DiagMessage(std::string_view msg = "", IssueClass type = IssueClass::Debug,
                IssueCode code = IssueCode::Info, uint32_t start = 0, uint32_t end = 0)
        : m_msg(msg), m_start(start), m_end(end), m_type(type), m_code(code) {}

    uint64_t hash() const;
  };

  class DiagnosticManager : public IReport {
    std::vector<DiagMessage> m_vec;
    std::unordered_set<uint64_t> m_visited;
    std::shared_ptr<IOffsetResolver> m_resolver;

    std::string mint_clang16_message(const DiagMessage &msg) const;
    std::string mint_plain_message(const DiagMessage &msg) const;
    std::string mint_modern_message(const DiagMessage &msg) const;

  public:
    DiagnosticManager(std::shared_ptr<IOffsetResolver> resolver = nullptr)
        : m_resolver(std::move(resolver)) {}

    virtual void report(IssueCode code, IssueClass level, std::span<std::string_view> params = {},
                        uint32_t start_offset = 1, uint32_t end_offset = 0,
                        std::string_view filename = "") override;

    size_t render(DiagnosticMessageHandler handler, nr_diag_format_t style);

    void clear() {
      m_vec.clear();
      m_visited.clear();
    }

    size_t size() { return m_vec.size(); }
  };

#ifdef REFACTOR_KEEP_PRESENT
  /**
   * @brief Report a diagnostic message
   * @return true always
   */
  bool report(IssueCode code, IssueClass type, std::string_view subject = "",
              uint32_t loc_start = 0, uint32_t loc_end = 0);

  static inline bool report(IssueCode code, IssueClass type, std::pair<uint32_t, uint32_t> loc,
                            std::string_view subject = "") {
    return report(code, type, subject, loc.first, loc.second);
  }
#endif

};  // namespace nr

#endif  // __NITRATE_QXIR_REPORT_H__
