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

#include <parse/parser.h>
#include <macro.h>
#include <error/message.h>

using namespace libquixcc;

static bool parse_enum_field(quixcc_job_t &job, std::shared_ptr<libquixcc::Scanner> scanner, EnumDefNode::Field &node)
{
    // <name> [ = <value> ] [,]

    Token tok = scanner->next();
    if (tok.type() != TokenType::Identifier)
    {
        PARMSG(tok, libquixcc::Err::ERROR, feedback[ENUM_FIELD_EXPECTED_IDENTIFIER], tok.serialize().c_str());
        return false;
    }

    node.name = std::get<std::string>(tok.val());

    tok = scanner->peek();
    if (tok.type() == TokenType::Operator && std::get<Operator>(tok.val()) == Operator::Assign)
    {
        scanner->next();
        if (!parse_const_expr(job, scanner, Token(TokenType::Punctor, Punctor::Comma), node.value))
        {
            PARMSG(tok, libquixcc::Err::ERROR, feedback[ENUM_FIELD_EXPECTED_CONST_EXPR], node.name.c_str());
            return false;
        }

        tok = scanner->peek();
    }

    if (tok.type() == TokenType::Punctor && std::get<Punctor>(tok.val()) == Punctor::Comma)
    {
        scanner->next();
        return true;
    }

    if (tok.type() != TokenType::Punctor || std::get<Punctor>(tok.val()) != Punctor::CloseBrace)
    {
        PARMSG(tok, libquixcc::Err::ERROR, feedback[ENUM_FIELD_EXPECTED_SEMICOLON], tok.serialize().c_str());
        return false;
    }

    return true;
}

bool libquixcc::parse_enum(quixcc_job_t &job, std::shared_ptr<libquixcc::Scanner> scanner, std::shared_ptr<libquixcc::StmtNode> &node)
{
    Token tok = scanner->next();
    if (tok.type() != TokenType::Identifier)
    {
        PARMSG(tok, libquixcc::Err::ERROR, feedback[ENUM_EXPECTED_IDENTIFIER], tok.serialize().c_str());
        return false;
    }

    std::string name = std::get<std::string>(tok.val());

    tok = scanner->next();
    if (tok.type() != TokenType::Punctor || std::get<Punctor>(tok.val()) != Punctor::Colon)
    {
        PARMSG(tok, libquixcc::Err::ERROR, feedback[ENUM_EXPECTED_COLON], tok.serialize().c_str());
        return false;
    }

    std::shared_ptr<TypeNode> type;
    if (!parse_type(job, scanner, type))
        return false;

    tok = scanner->next();
    if (tok.type() == TokenType::Punctor && std::get<Punctor>(tok.val()) == Punctor::Semicolon)
    {
        node = std::make_shared<EnumDeclNode>(name, type);
        return true;
    }

    if (tok.type() != TokenType::Punctor || std::get<Punctor>(tok.val()) != Punctor::OpenBrace)
    {
        PARMSG(tok, libquixcc::Err::ERROR, feedback[ENUM_EXPECTED_LEFT_BRACE], tok.serialize().c_str());
        return false;
    }

    std::vector<EnumDefNode::Field> fields;

    while (true)
    {
        tok = scanner->peek();
        if (tok.type() == TokenType::Punctor && std::get<Punctor>(tok.val()) == Punctor::CloseBrace)
        {
            scanner->next();
            break;
        }

        EnumDefNode::Field field;
        if (!parse_enum_field(job, scanner, field))
            return false;
        fields.push_back(field);
    }

    tok = scanner->next();
    if (tok.type() != TokenType::Punctor || std::get<Punctor>(tok.val()) != Punctor::Semicolon)
    {
        PARMSG(tok, libquixcc::Err::ERROR, feedback[ENUM_DEF_EXPECTED_SEMICOLON]);
        return false;
    }

    auto edef = std::make_shared<EnumDefNode>();
    edef->m_name = name;
    edef->m_fields = fields;
    edef->m_type = type;
    node = edef;
    return true;
}