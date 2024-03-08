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

#define QUIXCC_INTERNAL

#include <prep/macro/DefineMacro.h>

bool libquixcc::macro::ParseDefine(quixcc_job_t *job, const Token &tok, const std::string &directive, const std::string &parameter, std::vector<libquixcc::Token> &exp)
{
    (void)job;
    (void)tok;
    (void)directive;

    std::string name;
    std::string value;
    enum Type
    {
        Int,
        String,
        Bool
    } type;

    size_t pos = parameter.find(' ');

    if (pos == std::string::npos)
    {
        name = parameter;
        type = Bool;
    }
    else
    {
        name = parameter.substr(0, pos);
        if (pos + 1 < parameter.size())
            value = parameter.substr(pos + 1);

        bool is_int = true;
        for (char c : value)
        {
            if (!std::isdigit(c))
            {
                is_int = false;
                break;
            }
        }

        if (value == "true" || value == "false")
            type = Bool;
        else if (is_int)
            type = Int;
        else
            type = String;
    }

    exp.push_back(Token(TokenType::Keyword, Keyword::Const));
    exp.push_back(Token(TokenType::Identifier, name));
    exp.push_back(Token(TokenType::Punctor, Punctor::Colon));

    switch (type)
    {
    case Int:
        exp.push_back(Token(TokenType::Identifier, "i64"));
        exp.push_back(Token(TokenType::Operator, Operator::Assign));
        exp.push_back(Token(TokenType::IntegerLiteral, value));
        break;
    case String:
        exp.push_back(Token(TokenType::Identifier, "string"));
        exp.push_back(Token(TokenType::Operator, Operator::Assign));
        exp.push_back(Token(TokenType::StringLiteral, value));
        break;
    case Bool:
        exp.push_back(Token(TokenType::Identifier, "bool"));
        exp.push_back(Token(TokenType::Operator, Operator::Assign));
        if (value == "true")
            exp.push_back(Token(TokenType::Keyword, Keyword::True));
        else
            exp.push_back(Token(TokenType::Keyword, Keyword::False));
        break;
    default:
        break;
    }

    exp.push_back(Token(TokenType::Punctor, Punctor::Semicolon));

    return true;
}