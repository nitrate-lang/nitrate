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

#include <quixcc/lexer/Lex.h>
#include <quixcc/lexer/Token.h>

#include <sstream>

using namespace libquixcc;

Loc Loc::operator-(Loc::LocPosType rhs) const {
  if (rhs <= m_col)
    return Loc(m_line, m_col - rhs, m_file);

  Loc new_loc = *this;

  while (true) {
    if (rhs < new_loc.m_col) {
      new_loc.m_col -= rhs;
      break;
    }

    if (new_loc.m_line == 1) {
      new_loc.m_col = 1;
      break;
    }

    rhs -= new_loc.m_col;
    new_loc.m_col = 1;
    new_loc.m_line--;
  }

  return new_loc;
}

Token::Token(TT _type, TokVal value, Loc _loc) {
  m_type = _type;
  m_value = value;
  m_loc = _loc;
}

std::string libquixcc::Token::serialize_human_readable() const {
  std::stringstream ss;

  switch (m_type) {
  case tEofF:
    ss << "Eof";
    break;
  case tErro:
    ss << "Unknown";
    break;
  case tName:
    ss << "Identifier(" << as<std::string>() << ")";
    break;
  case tKeyW:
    ss << "Keyword(" << keyword_map_inverse.at(as<Keyword>()).data() << ")";
    break;
  case tOper:
    ss << "Operator(" << operator_map_inverse.at(as<Operator>()).data() << ")";
    break;
  case tPunc:
    ss << "Punctor(" << punctor_map_inverse.at(as<Punctor>()).data() << ")";
    break;
  case tText:
    ss << "String(\"" << as<std::string>() << "\")";
    break;
  case tChar:
    ss << "Char('" << as<std::string>() << "')";
    break;
  case tIntL:
    ss << "Number(" << as<std::string>() << ")";
    break;
  case tNumL:
    ss << "Float(" << as<std::string>() << ")";
    break;
  case tNote:
    ss << "Comment(" << as<std::string>() << ")";
    break;
  case tMacB:
    ss << "MacroBlock(" << as<std::string>() << ")";
    break;
  case tMacr:
    ss << "MacroSingleLine(" << as<std::string>() << ")";
    break;
  default:
    ss << "Unknown(" << as<std::string>() << ")";
    break;
  }

  return ss.str();
}

std::string Token::serialize(bool human_readable) const {
  if (human_readable)
    return serialize_human_readable();

  std::stringstream ss;
  switch (m_type) {
  case tEofF:
    break;
  case tErro:
  case tName:
  case tIntL:
  case tNumL:
    ss << as<std::string>();
    break;
  case tNote:
    ss << "/*" <<  Scanner::escape_string(as<std::string>()) << "*/";
    break;
  case tText:
    ss << "\"" << Scanner::escape_string(as<std::string>()) << "\"";
    break;
  case tChar:
    ss << "'" << Scanner::escape_string(as<std::string>()) << "'";
    break;
  case tMacB:
    ss << "@(" <<  Scanner::escape_string(as<std::string>()) << ")";
    break;
  case tMacr:
    ss << "@" <<  Scanner::escape_string(as<std::string>());
    break;
  case tKeyW:
    ss << keyword_map_inverse.at(as<Keyword>()).data();
    break;
  case tOper:
    ss << operator_map_inverse.at(as<Operator>()).data();
    break;
  case tPunc:
    ss << punctor_map_inverse.at(as<Punctor>()).data();
    break;
  default:
    throw std::runtime_error("Invalid type");
  }

  return ss.str();
}
