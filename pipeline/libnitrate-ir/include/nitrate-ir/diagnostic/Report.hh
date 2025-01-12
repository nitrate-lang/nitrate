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
                        SrcLoc location = SrcLoc()) = 0;

    void report(IssueCode code, IC level, std::string_view message,
                SrcLoc loc = SrcLoc()) {
      report(code, level, std::vector<std::string_view>({message}), loc);
    };

    virtual void erase_reports() = 0;

    virtual void stream_reports(std::function<void(const ReportData &)> cb) = 0;
  };

  ///==========================================================================///

  class MessageBuffer {
    std::stringstream m_buffer;
    std::function<void(std::string, SrcLoc)> m_on_flush;
    SrcLoc m_range;

  public:
    MessageBuffer(std::function<void(std::string, SrcLoc)> on_flush)
        : m_on_flush(on_flush) {}

    MessageBuffer(MessageBuffer &&O) {
      m_buffer = std::move(O.m_buffer);
      m_on_flush = std::move(O.m_on_flush);
      m_range = O.m_range;
      O.m_on_flush = nullptr;
    }

    ~MessageBuffer() {
      if (m_on_flush) {
        m_on_flush(m_buffer.str(), m_range);
      }
    }

    template <typename T>
    void write(const T &value) {
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

  inline thread_local std::unique_ptr<detail::IDiagnosticRouter> debug =
      std::make_unique<detail::NOPDiagnosticRouter>();
  inline thread_local std::unique_ptr<detail::IDiagnosticRouter> info =
      std::make_unique<detail::NOPDiagnosticRouter>();
  inline thread_local std::unique_ptr<detail::IDiagnosticRouter> warn =
      std::make_unique<detail::NOPDiagnosticRouter>();
  inline thread_local std::unique_ptr<detail::IDiagnosticRouter> error =
      std::make_unique<detail::NOPDiagnosticRouter>();

  namespace detail {
    void diagnostic_bind(MessageFunc func, lex::IScanner &scanner);
  }  // namespace detail

  template <typename T>
  MessageBuffer operator<<(detail::DiagnosticRouterInstance *ctx,
                           const T &value) {
    MessageBuffer buf(
        [ctx](std::string msg, SrcLoc loc) { ctx->EmitMessage(msg, loc); });

    buf.write(value);

    return buf;
  };

  template <typename T>
  MessageBuffer operator<<(MessageBuffer &&buf, const T &value) {
    buf.write(value);
    return std::move(buf);
  };
};  // namespace ncc::ir

#endif  // __NITRATE_IR_REPORT_H__
