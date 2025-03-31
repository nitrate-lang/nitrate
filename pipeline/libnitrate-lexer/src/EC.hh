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

#ifndef __NITRATE_LEXER_EC_H__
#define __NITRATE_LEXER_EC_H__

#include <nitrate-core/Logger.hh>

namespace ncc::lex {
  std::string Formatter(std::string_view msg, Sev sev);

  NCC_EC_GROUP(Lexer);

  NCC_EC_EX(Lexer, Runtime, Formatter);
  NCC_EC_EX(Lexer, UserRequest, Formatter);
  NCC_EC_EX(Lexer, UnexpectedEOF, Formatter);

  NCC_EC_EX(Lexer, LiteralOutOfRange, Formatter);
  NCC_EC_EX(Lexer, InvalidNumber, Formatter);
  NCC_EC_EX(Lexer, InvalidMantissa, Formatter);
  NCC_EC_EX(Lexer, InvalidExponent, Formatter);

  NCC_EC_EX(Lexer, InvalidHexDigit, Formatter);
  NCC_EC_EX(Lexer, InvalidDecimalDigit, Formatter);
  NCC_EC_EX(Lexer, InvalidOctalDigit, Formatter);
  NCC_EC_EX(Lexer, InvalidBinaryDigit, Formatter);

  NCC_EC_EX(Lexer, MissingUnicodeBrace, Formatter);
  NCC_EC_EX(Lexer, InvalidUnicodeCodepoint, Formatter);
  NCC_EC_EX(Lexer, LexicalGarbage, Formatter);
  NCC_EC_EX(Lexer, InvalidUTF8, Formatter);
  NCC_EC_EX(Lexer, InvalidIdentifier, Formatter);
}  // namespace ncc::lex

#endif
