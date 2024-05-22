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

#ifndef __QUIXCC_LEX_TOKEN_H__
#define __QUIXCC_LEX_TOKEN_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <string>
#include <variant>
#include <cstdint>

namespace libquixcc
{
    enum class Keyword
    {
        Subsystem,
        Import,
        Type,
        Let,
        Var,
        Const,
        Static,
        Struct,
        Region,
        Group,
        Union,
        Opaque,
        Fn,
        Nothrow,
        Foreign,
        Impure,
        Tsafe,
        Enum,
        Pub,
        If,
        Else,
        For,
        While,
        Do,
        Switch,
        Case,
        Default,
        Break,
        Continue,
        Return,
        Retif,
        Retz,
        Retv,
        __Asm__,
        Void,
        Undef,
        Null,
        True,
        False,
    };

    enum class Punctor
    {
        OpenParen,
        CloseParen,
        OpenBrace,
        CloseBrace,
        OpenBracket,
        CloseBracket,
        Dot,
        Comma,
        Colon,
        Semicolon,
    };

    enum class Operator
    {
        At,
        Question,
        Arrow,

        Plus,
        Minus,
        Multiply,
        Divide,
        Modulo,

        BitwiseAnd,
        BitwiseOr,
        BitwiseXor,
        BitwiseNot,

        LeftShift,
        RightShift,

        Increment,
        Decrement,

        Assign,
        PlusAssign,
        MinusAssign,
        MultiplyAssign,
        DivideAssign,
        ModuloAssign,
        BitwiseOrAssign,
        BitwiseAndAssign,
        BitwiseXorAssign,
        XorAssign,
        OrAssign,
        AndAssign,
        LeftShiftAssign,
        RightShiftAssign,

        LogicalNot,
        LogicalAnd,
        LogicalOr,
        LogicalXor,

        LessThan,
        GreaterThan,
        LessThanEqual,
        GreaterThanEqual,
        Equal,
        NotEqual,
    };

    enum class TT
    {
        Eof = -1,
        Unknown = 0,

        Identifier = 1,
        Keyword = 2,
        Operator = 3,
        Punctor = 4,

        Integer = 5,
        Float = 6,
        String = 7,
        Char = 8,

        MacroBlock = 9,
        MacroSingleLine = 10,

        Comment = 11,
    };

    typedef std::variant<std::string, Punctor, Keyword, Operator> TokVal;

    struct Loc
    {
        std::string file;
        int32_t line;
        int32_t col;

        Loc() : line(1), col(1) {}
        Loc(int32_t line, int32_t col, std::string file = "") : file(file), line(line), col(col) {}

        Loc operator-(int32_t rhs) const;
    };

    class Token
    {
        TokVal m_value;
        Loc m_loc;
        TT m_type;

    public:
        Token() : m_value(std::string()), m_loc(), m_type(TT::Unknown) {}
        Token(TT type, TokVal value, Loc loc = Loc());

        inline TT type() const { return m_type; }
        inline const TokVal &val() const { return m_value; }
        inline const Loc &loc() const { return m_loc; }

        inline bool is(TT val) const { return m_type == val; }

        template <typename T, typename V = T>
        bool is(V val) const
        {
            return std::holds_alternative<T>(m_value) && std::get<V>(m_value) == val;
        }

        std::string serialize(bool human_readable = true) const;

        inline bool operator==(const Token &rhs) const { return m_type == rhs.m_type && m_value == rhs.m_value; }
        inline bool operator<(const Token &rhs) const
        {
            if (m_type != rhs.m_type)
                return m_type < rhs.m_type;
            return m_value < rhs.m_value;
        }

        inline bool nil() const { return m_type == TT::Unknown; }
    };
};

#endif // __QUIXCC_LEX_TOKEN_H__