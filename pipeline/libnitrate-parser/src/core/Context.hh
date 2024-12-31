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

#ifndef __NITRATE_AST_CONTEXT_H__
#define __NITRATE_AST_CONTEXT_H__

#include <stdbool.h>

#include <cstdarg>
#include <functional>
#include <nitrate-core/Allocate.hh>
#include <nitrate-core/Environment.hh>
#include <nitrate-lexer/Lexer.hh>
#include <nitrate-lexer/Token.hh>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/Context.hh>
#include <sstream>

namespace ncc::parse {
  enum class FormatStyle {
    Clang16Color, /* Clang-like 16 color diagnostic format */
    ClangPlain,   /* Clang-like plain text diagnostic format */
    Default = Clang16Color,
  };

  std::string mint_clang16_message(ncc::lex::IScanner &lexer,
                                   std::string_view msg, ncc::lex::Token tok);

  class MessageBuffer {
    std::stringstream m_buffer;
    std::function<void(std::string, ncc::lex::Token)> m_publisher;
    ncc::lex::Token m_start_loc;

  public:
    MessageBuffer(std::function<void(std::string, ncc::lex::Token)> publisher)
        : m_publisher(publisher), m_start_loc({}) {}

    MessageBuffer(MessageBuffer &&O) {
      m_buffer = std::move(O.m_buffer);
      m_start_loc = std::move(O.m_start_loc);
      m_publisher = std::move(O.m_publisher);
      O.m_publisher = nullptr;
    }

    ~MessageBuffer() {
      if (m_publisher) {
        m_publisher(m_buffer.str(), m_start_loc);
      }
    }

    template <typename T>
    void write(const T &value) {
      if constexpr (std::is_same_v<T, ncc::lex::Token>) {
        m_start_loc = value;
      } else {
        m_buffer << value;
      }
    }
  };

  template <typename T>
  MessageBuffer operator<<(Parser *log, const T &value) {
    MessageBuffer buf([log](std::string msg, ncc::lex::Token start_loc) {
      log->SetFailBit();
      qcore_print(
          QCORE_ERROR,
          mint_clang16_message(log->GetLexer(), msg, start_loc).c_str());
    });

    buf.write(value);

    return buf;
  };

  template <typename T>
  MessageBuffer operator<<(MessageBuffer &&buf, const T &value) {
    buf.write(value);
    return std::move(buf);
  };

  inline thread_local Parser *diagnostic;
};  // namespace ncc::parse

#endif
