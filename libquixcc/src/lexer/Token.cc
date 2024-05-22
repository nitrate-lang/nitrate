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
///     * Copyright (C) 2024 Wesley C. Jones                                     ///
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
///     License along with the QUIX Compiler Suite; if not, see                  ///
///     <https://www.gnu.org/licenses/>.                                         ///
///                                                                              ///
////////////////////////////////////////////////////////////////////////////////////

#define QUIXCC_INTERNAL

#include <lexer/Token.h>
#include <lexer/Lex.h>

libquixcc::Loc libquixcc::Loc::operator-(int32_t rhs) const
{
    if (rhs <= col)
        return Loc(line, col - rhs, file);

    Loc new_loc = *this;

    while (rhs > col)
    {
        rhs -= col;
        new_loc.line--;
        new_loc.col = 1;
    }

    new_loc.col -= rhs;

    return new_loc;
}

libquixcc::Token::Token(libquixcc::TT type, libquixcc::TokVal value, libquixcc::Loc loc)
{
    m_type = type;
    m_value = value;
    m_loc = loc;
}

std::string libquixcc::Token::serialize(bool human_readable) const
{
    (void)human_readable;

    switch (m_type)
    {
    case TT::Eof:
        return "Eof";
    case TT::Unknown:
        return "Unknown";
    case TT::Identifier:
        return "Identifier(" + std::get<std::string>(m_value) + ")";
    case TT::Keyword:
        return "Keyword(" + keyword_map_inverse.at(std::get<Keyword>(m_value)) + ")";
    case TT::Operator:
        return "Operator(" + operator_map_inverse.at(std::get<Operator>(m_value)) + ")";
    case TT::Punctor:
        return "Punctor(" + punctor_map_inverse.at(std::get<Punctor>(m_value)) + ")";
    case TT::String:
        return "\"" + std::get<std::string>(m_value) + "\"";
    case TT::Char:
        return "'" + std::get<std::string>(m_value) + "'";
    case TT::Integer:
        return "Number(" + std::get<std::string>(m_value) + ")";
    case TT::Float:
        return "Float(" + std::get<std::string>(m_value) + ")";
    case TT::Comment:
        return "/* " + std::get<std::string>(m_value) + " */";
    case TT::MacroBlock:
        return "MacroBlock(" + std::get<std::string>(m_value) + ")";
    case TT::MacroSingleLine:
        return "MacroSingleLine(" + std::get<std::string>(m_value) + ")";
    default:
        return "Unknown";
    }

    return "Unknown";
}
