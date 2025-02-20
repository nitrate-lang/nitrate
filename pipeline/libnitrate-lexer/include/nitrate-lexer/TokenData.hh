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

#ifndef __NITRATE_LEXER_TOKEN_DATA_HH__
#define __NITRATE_LEXER_TOKEN_DATA_HH__

#include <nitrate-core/String.hh>
#include <nitrate-lexer/Enums.hh>

namespace ncc::lex {
  union TokenData {
    Punctor m_punc;
    Operator m_op;
    Keyword m_key;
    string m_str;

    constexpr TokenData(Punctor punc) : m_punc(punc) {}
    constexpr TokenData(Operator op) : m_op(op) {}
    constexpr TokenData(Keyword key) : m_key(key) {}
    constexpr TokenData(string str) : m_str(str) {}

    static constexpr TokenData GetDefault(TokenType ty) {
      switch (ty) {
        case EofF:
          return Operator();
        case Punc:
          return Punctor();
        case Oper:
          return Operator();
        case KeyW:
          return Keyword();
        case IntL:
        case NumL:
        case Text:
        case Name:
        case Char:
        case MacB:
        case Macr:
        case Note:
          return string();
      }
    }
  } __attribute__((packed));

  string to_string(TokenType, TokenData);  // NOLINT(readability-identifier-naming)
}  // namespace ncc::lex

#endif
