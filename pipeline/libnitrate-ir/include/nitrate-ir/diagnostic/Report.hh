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
#include <memory>
#include <nitrate-core/Logger.hh>
#include <nitrate-ir/IR.hh>
#include <nitrate-ir/diagnostic/EC.hh>
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
      IssueCode m_code;
      IC m_level;
      std::string_view m_param;
      uint32_t m_start_offset;
      uint32_t m_fileid;
    };

    virtual ~IReport() = default;

    virtual void Report(IssueCode code, IC level,
                        std::vector<std::string_view> params = {},
                        SrcLoc location = SrcLoc()) = 0;

    void Report(IssueCode code, IC level, std::string_view message,
                SrcLoc loc = SrcLoc()) {
      Report(code, level, std::vector<std::string_view>({message}), loc);
    };

    virtual void EraseReports() = 0;

    virtual void StreamReports(std::function<void(const ReportData &)> cb) = 0;
  };

  ///==========================================================================///

  class MessageBuffer {
    std::stringstream m_buffer;
    std::function<void(std::string, SrcLoc)> m_on_flush;
    SrcLoc m_range;

  public:
    MessageBuffer(std::function<void(std::string, SrcLoc)> on_flush)
        : m_on_flush(on_flush) {}

    MessageBuffer(MessageBuffer &&o) {
      m_buffer = std::move(o.m_buffer);
      m_on_flush = std::move(o.m_on_flush);
      m_range = o.m_range;
      o.m_on_flush = nullptr;
    }

    ~MessageBuffer() {
      if (m_on_flush) {
        m_on_flush(m_buffer.str(), m_range);
      }
    }

    template <typename T>
    void Write(const T &value) {
      if constexpr (std::is_same_v<T, SrcLoc>) {
        m_range = value;
      } else {
        m_buffer << value;
      }
    }
  };

  namespace detail {
    using MessageFunc = std::function<void(std::string_view)>;

    class IDiagnosticRouter {
    public:
      virtual ~IDiagnosticRouter() = default;
      virtual void EmitMessage(std::string_view msg, SrcLoc loc) = 0;
    };

    class NOPDiagnosticRouter final : public IDiagnosticRouter {
    public:
      virtual ~NOPDiagnosticRouter() = default;
      void EmitMessage(std::string_view, SrcLoc) override{};
    };

    class DiagnosticRouterInstance final : public IDiagnosticRouter {
      MessageFunc m_func;
      lex::IScanner *m_scanner;

    public:
      virtual ~DiagnosticRouterInstance() = default;

      DiagnosticRouterInstance(MessageFunc func, lex::IScanner &scanner)
          : m_func(func), m_scanner(&scanner) {}

      void EmitMessage(std::string_view, SrcLoc) override {
        /// TODO: Implement this function
        qcore_implement();
        (void)m_scanner;
      }
    };
  }  // namespace detail

  namespace detail {
    void DiagnosticBind(MessageFunc func, lex::IScanner &scanner);
  }  // namespace detail

  template <typename T>
  MessageBuffer operator<<(detail::DiagnosticRouterInstance *ctx,
                           const T &value) {
    MessageBuffer buf(
        [ctx](std::string msg, SrcLoc loc) { ctx->EmitMessage(msg, loc); });

    buf.Write(value);

    return buf;
  };

  template <typename T>
  MessageBuffer operator<<(MessageBuffer &&buf, const T &value) {
    buf.Write(value);
    return std::move(buf);
  };
};  // namespace ncc::ir

#endif  // __NITRATE_IR_REPORT_H__
