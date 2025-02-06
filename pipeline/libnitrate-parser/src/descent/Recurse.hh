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

#ifndef __NITRATE_PARSE_H__
#define __NITRATE_PARSE_H__

#include <core/ParserImpl.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-lexer/Scanner.hh>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/ASTCommon.hh>
#include <nitrate-parser/Context.hh>
#include <nitrate-parser/EC.hh>

namespace ncc::parse {
  using namespace ec;

#define next() m_rd.Next()
#define peek() m_rd.Peek()
#define current() m_rd.Current()

  template <auto tok>
  static inline auto NextIfImpl(ncc::lex::IScanner &m_rd) -> std::optional<ncc::lex::Token> {
    auto t = m_rd.Peek();
    if constexpr (std::is_same_v<decltype(tok), ncc::lex::TokenType>) {
      if (t.Is(tok)) {
        m_rd.Next();
        return t;
      }

      return std::nullopt;
    } else {
      if (t.Is<tok>()) {
        m_rd.Next();
        return t;
      }

      return std::nullopt;
    }
  }

#define NextIf(tok) NextIfImpl<tok>(m_rd)

  static inline auto BindComments(auto node, auto comments) {
    node->SetComments(std::move(comments));
    return node;
  }

};  // namespace ncc::parse

#endif  // __NITRATE_PARSE_H__
