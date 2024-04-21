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
#include <optional>
#include <variant>

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
        LessThan,
        GreaterThan,
        Assign,
        At,
        Minus,
        Plus,
        Multiply,
        Divide,
        Modulo,
        BitwiseAnd,
        BitwiseOr,
        BitwiseXor,
        BitwiseNot,
        Not,
        Question,
        PlusAssign,
        MinusAssign,
        MultiplyAssign,
        DivideAssign,
        ModuloAssign,
        BitwiseOrAssign,
        BitwiseAndAssign,
        BitwiseXorAssign,
        LeftShift,
        RightShift,
        Equal,
        NotEqual,
        And,
        Or,
        Xor,
        LessThanEqual,
        GreaterThanEqual,
        Increment,
        Decrement,
        XorAssign,
        OrAssign,
        AndAssign,
        LeftShiftAssign,
        RightShiftAssign
    };

    enum class TokenType
    {
        Eof = -1,
        Unknown,

        Identifier,
        Keyword,
        Operator,

        IntegerLiteral,
        FloatingLiteral,
        StringLiteral,
        CharLiteral,
        MacroBlock,
        MacroSingleLine,

        Punctor,

        Comment
    };

    typedef std::variant<std::string, Punctor, Keyword, Operator> TokVal;

    struct Loc
    {
        int64_t line;
        int64_t col;
        std::string file;

        Loc() : line(1), col(1) {}
        Loc(int64_t line, int64_t col, std::string file = "") : line(line), col(col), file(file) {}

        Loc operator-(size_t rhs) const;

        static Loc Unk;
    };

    class Token
    {
        TokenType m_type;
        TokVal m_value;
        Loc m_loc;

    public:
        Token() : m_type(TokenType::Unknown), m_value(std::string()) {}
        Token(TokenType type, TokVal value, Loc loc = Loc());

        TokenType type() const;
        const TokVal &val() const;
        const Loc &loc() const;

        bool is(TokenType val) const
        {
            return m_type == val;
        }

        template <typename T, typename V = T>
        bool is(V val) const
        {
            return std::holds_alternative<T>(m_value) && std::get<V>(m_value) == val;
        }

        std::string serialize(bool human_readable = true) const;

        bool operator==(const Token &rhs) const;
        bool operator<(const Token &rhs) const;

        bool nil() const
        {
            return m_type == TokenType::Unknown;
        }
    };
};

#endif // __QUIXCC_LEX_TOKEN_H__