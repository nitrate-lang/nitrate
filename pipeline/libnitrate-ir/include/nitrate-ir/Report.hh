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

#include <boost/bimap.hpp>
#include <cstdarg>
#include <span>
#include <string_view>

namespace qparse {
  class Node;
}

namespace nr {
  enum class IC {
    Debug = 0,
    Info,
    Warn,
    Error,
    FatalError,
  };

  enum IssueCode {
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

  class IReport {
  public:
    struct ReportData {
      IssueCode code;
      IC level;
      std::string_view param;
      uint32_t start_offset;
      uint32_t end_offset;
      std::string_view filename;
    };

    virtual ~IReport() = default;

    virtual void report(IssueCode code, IC level, std::span<std::string_view> params = {},
                        uint32_t start_offset = 1, uint32_t end_offset = 0,
                        std::string_view filename = "") = 0;

    void report(IssueCode code, IC level, std::string_view message,
                std::pair<uint32_t, uint32_t> loc = {UINT32_MAX, UINT32_MAX},
                std::string_view filename = "") {
      std::array<std::string_view, 1> x = {message};
      report(code, level, x, loc.first, loc.second, filename);
    };

    virtual void erase_reports() = 0;

    virtual void stream_reports(std::function<void(const ReportData&)> cb) = 0;
  };

  class ISourceView {
  public:
    virtual ~ISourceView() = default;

    virtual std::optional<std::pair<uint32_t, uint32_t>> off2rc(uint32_t offset) noexcept = 0;
  };
};  // namespace nr

#endif  // __NITRATE_QXIR_REPORT_H__
