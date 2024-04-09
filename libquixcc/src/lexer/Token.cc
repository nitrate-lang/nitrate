////////////////////////////////////////////////////////////////////////////////////
///                                                                              ///
///    ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░    ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░   ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░          ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░          ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░          ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░   ///
///    ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░    ///
///      ░▒▓█▓▒░                                                                 ///
///       ░▒▓██▓▒░                                                               ///
///                                                                              ///
///     * QUIX LANG COMPILER - The official compiler for the Quix language.      ///
///     * Copyright (C) 2020-2024 Wesley C. Jones                                ///
///                                                                              ///
///     The QUIX Compiler Suite is free software; you can redistribute it and/or ///
///     modify it under the terms of the GNU Lesser General Public               ///
///     License as published by the Free Software Foundation; either             ///
///     version 2.1 of the License, or (at your option) any later version.       ///
///                                                                              ///
///     The QUIX Compiler Suite is distributed in the hope that it will be       ///
///     useful, but WITHOUT ANY WARRANTY; without even the implied warranty of   ///
///     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        ///
///     Lesser General Public License for more details.                          ///
///                                                                              ///
///     You should have received a copy of the GNU Lesser General Public         ///
///     License along with the GNU C Library; if not, see                        ///
///     <https://www.gnu.org/licenses/>.                                         ///
///                                                                              ///
////////////////////////////////////////////////////////////////////////////////////

#define QUIXCC_INTERNAL

#include <lexer/Token.h>
#include <map>

namespace libquixcc
{
    extern std::map<libquixcc::Keyword, std::string> keyword_map_inverse;
    extern std::map<libquixcc::Operator, std::string> operator_map_inverse;
    extern std::map<libquixcc::Punctor, std::string> punctor_map_inverse;
}

libquixcc::Loc libquixcc::Loc::Unk = libquixcc::Loc(-1, -1);

libquixcc::Loc libquixcc::Loc::operator-(size_t rhs) const
{
    if (col == -1 || line == -1)
    {
        return *this;
    }

    if (rhs <= (uint64_t)col)
        return Loc(line, col - rhs, file);

    Loc new_loc = *this;

    while (rhs > (uint64_t)col)
    {
        rhs -= col;
        new_loc.line--;
        new_loc.col = 1;
    }

    new_loc.col -= rhs;

    return new_loc;
}

libquixcc::Token::Token(libquixcc::TokenType type, libquixcc::TokVal value, libquixcc::Loc loc)
{
    m_type = type;
    m_value = value;
    m_loc = loc;
}

libquixcc::TokenType libquixcc::Token::type() const
{
    return m_type;
}

const libquixcc::TokVal &libquixcc::Token::val() const
{
    return m_value;
}

const libquixcc::Loc &libquixcc::Token::loc() const
{
    return m_loc;
}

std::string libquixcc::Token::serialize(bool human_readable) const
{
    (void)human_readable;

    switch (m_type)
    {
    case TokenType::Eof:
        return "Eof";
    case TokenType::Unknown:
        return "Unknown";
    case TokenType::Identifier:
        return "Identifier(" + std::get<std::string>(m_value) + ")";
    case TokenType::Keyword:
        return "Keyword(" + keyword_map_inverse.at(std::get<Keyword>(m_value)) + ")";
    case TokenType::Operator:
        return "Operator(" + operator_map_inverse.at(std::get<Operator>(m_value)) + ")";
    case TokenType::Punctor:
        return "Punctor(" + punctor_map_inverse.at(std::get<Punctor>(m_value)) + ")";
    case TokenType::StringLiteral:
        return "\"" + std::get<std::string>(m_value) + "\"";
    case TokenType::CharLiteral:
        return "'" + std::get<std::string>(m_value) + "'";
    case TokenType::IntegerLiteral:
        return "Number(" + std::get<std::string>(m_value) + ")";
    case TokenType::FloatingLiteral:
        return "Float(" + std::get<std::string>(m_value) + ")";
    case TokenType::Comment:
        return "/* " + std::get<std::string>(m_value) + " */";
    case TokenType::MacroBlock:
        return "MacroBlock(" + std::get<std::string>(m_value) + ")";
    case TokenType::MacroSingleLine:
        return "MacroSingleLine(" + std::get<std::string>(m_value) + ")";
    default:
        return "Unknown";
    }

    return "Unknown";
}

bool libquixcc::Token::operator==(const libquixcc::Token &rhs) const
{
    return m_type == rhs.m_type && m_value == rhs.m_value;
}

bool libquixcc::Token::operator<(const libquixcc::Token &rhs) const
{
    if (m_type != rhs.m_type)
        return m_type < rhs.m_type;
    return m_value < rhs.m_value;
}