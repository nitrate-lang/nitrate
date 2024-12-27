////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///     .-----------------.    .----------------.     .----------------.     ///
///    | .--------------. |   | .--------------. |   | .--------------. |    ///
///    | | ____  _____  | |   | |     ____     | |   | |    ______    | |    ///
///    | ||_   _|_   _| | |   | |   .'    `.   | |   | |   / ____ `.  | |    ///
///    | |  |   \ | |   | |   | |  /  .--.  \  | |   | |   `'  __) |  | |    ///
///    | |  | |\ \| |   | |   | |  | |    | |  | |   | |   _  |__ '.  | |    ///
///    | | _| |_\   |_  | |   | |  \  `--'  /  | |   | |  | \____) |  | |    ///
///    | ||_____|\____| | |   | |   `.____.'   | |   | |   \______.'  | |    ///
///    | |              | |   | |              | |   | |              | |    ///
///    | '--------------' |   | '--------------' |   | '--------------' |    ///
///     '----------------'     '----------------'     '----------------'     ///
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

#ifndef __NITRATE_IR_REPORT_H__
#define __NITRATE_IR_REPORT_H__

#include <boost/bimap.hpp>
#include <cstdarg>
#include <nitrate-ir/IR.hh>
#include <nitrate-lexer/Token.hh>
#include <nitrate-parser/ASTBase.hh>
#include <string_view>

namespace ncc::ir {
  enum class IC {
    Debug,
    Info,
    Warn,
    Error,
  };

  enum IssueCode {
    CompilerError,
    SignalReceived,
    PTreeInvalid,
    DSPolyCyclicRef,
    DSNullPtr,
    DSBadType,
    DSBadTmpNode,

    NameConflict,
    UnknownFunction,
    VariadicNotEnoughArguments,
    TwoManyArguments,
    TwoFewArguments,
    TypeInference,
    NameManglingTypeInfer,
    UnexpectedUndefLiteral,
    ReturnTypeMismatch,
    ConstAssign,

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
      uint32_t fileid;
    };

    virtual ~IReport() = default;

    virtual void report(IssueCode code, IC level,
                        std::vector<std::string_view> params = {},
                        std::tuple<uint32_t, uint32_t> location = {
                            ncc::lex::QLEX_EOFF, ncc::lex::QLEX_NOFILE}) = 0;

    void report(IssueCode code, IC level, std::string_view message,
                std::tuple<uint32_t, uint32_t> loc = {ncc::lex::QLEX_EOFF,
                                                      ncc::lex::QLEX_NOFILE}) {
      report(code, level, std::vector<std::string_view>({message}), loc);
    };

    virtual void erase_reports() = 0;

    virtual void stream_reports(std::function<void(const ReportData&)> cb) = 0;
  };
};  // namespace ncc::ir

#endif  // __NITRATE_IR_REPORT_H__
