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

#define QUIXCC_INTERNAL

#include <lexer/Lex.h>
#include <lexer/Token.h>

#include <sstream>

using namespace libquixcc;

Loc Loc::operator-(uint_fast32_t rhs) const {
  if (rhs <= col) return Loc(line, col - rhs, file);

  Loc new_loc = *this;

  while (rhs > col) {
    rhs -= col;
    new_loc.col = 1;

    if (new_loc.line == 0) break;
    new_loc.line--;
  }

  new_loc.col -= rhs;

  return new_loc;
}

Token::Token(TT _type, TokVal value, Loc _loc) {
  type = _type;
  m_value = value;
  m_loc = _loc;
}

std::string libquixcc::Token::serialize_human_readable() const {
  std::stringstream ss;

  switch (type) {
    case TT::Eof:
      ss << "Eof";
      break;
    case TT::Unknown:
      ss << "Unknown";
      break;
    case TT::Identifier:
      ss << "Identifier(" << as<std::string>() << ")";
      break;
    case TT::Keyword:
      ss << "Keyword(" << keyword_map_inverse.at(as<Keyword>()).data() << ")";
      break;
    case TT::Operator:
      ss << "Operator(" << operator_map_inverse.at(as<Operator>()).data()
         << ")";
      break;
    case TT::Punctor:
      ss << "Punctor(" << punctor_map_inverse.at(as<Punctor>()).data() << ")";
      break;
    case TT::String:
      ss << "String(\"" << as<std::string>() << "\")";
      break;
    case TT::Char:
      ss << "Char('" << as<std::string>() << "')";
      break;
    case TT::Integer:
      ss << "Number(" << as<std::string>() << ")";
      break;
    case TT::Float:
      ss << "Float(" << as<std::string>() << ")";
      break;
    case TT::Comment:
      ss << "Comment(" << as<std::string>() << ")";
      break;
    case TT::MacroBlock:
      ss << "MacroBlock(" << as<std::string>() << ")";
      break;
    case TT::MacroSingleLine:
      ss << "MacroSingleLine(" << as<std::string>() << ")";
      break;
    default:
      ss << "Unknown(" << as<std::string>() << ")";
      break;
  }

  return ss.str();
}

std::string Token::serialize(bool human_readable) const {
  if (human_readable) return serialize_human_readable();

  std::stringstream ss;
  switch (type) {
    case TT::Eof:
      break;
    case TT::Unknown:
    case TT::Identifier:
    case TT::Integer:
    case TT::Float:
      ss << as<std::string>();
      break;
    case TT::Comment:
      ss << "/*" << as<std::string>() << "*/";
      break;
    case TT::String:
      ss << "\"" << as<std::string>() << "\"";
      break;
    case TT::Char:
      ss << "'" << as<std::string>() << "'";
      break;
    case TT::MacroBlock:
      ss << "@(" << as<std::string>() << ")";
      break;
    case TT::MacroSingleLine:
      ss << "@" << as<std::string>();
      break;
    case TT::Keyword:
      ss << keyword_map_inverse.at(as<Keyword>()).data();
      break;
    case TT::Operator:
      ss << operator_map_inverse.at(as<Operator>()).data();
      break;
    case TT::Punctor:
      ss << punctor_map_inverse.at(as<Punctor>()).data();
      break;
    default:
      throw std::runtime_error("Invalid type");
  }

  return ss.str();
}
