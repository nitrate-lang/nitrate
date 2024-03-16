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
///     * Copyright (c) 2024, Wesley C. Jones. All rights reserved.              ///
///     * License terms may be found in the LICENSE file.                        ///
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
        Export,
        Let,
        Var,
        Const,
        Struct,
        Region,
        Union,
        Packet,
        Fn,
        Nothrow,
        Foreign,
        Pure,
        Impure,
        Tsafe,
        Typedef,
        Static,
        Volatile,
        Enum,
        Class,
        Bundle,
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
        Abortif,
        Retz,
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
        BitAnd,
        BitOr,
        BitXor,
        BitNot,
        Not,
        Question,
        PlusAssign,
        MinusAssign,
        MultiplyAssign,
        DivideAssign,
        ModuloAssign,
        BitOrAssign,
        BitAndAssign,
        BitXorAssign,
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

        std::string serialize(bool human_readable = true) const;

        bool operator==(const Token &rhs) const;
    };
};

#endif // __QUIXCC_LEX_TOKEN_H__