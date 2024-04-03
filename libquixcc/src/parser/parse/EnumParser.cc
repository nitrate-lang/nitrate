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

#include <parse/Parser.h>
#include <LibMacro.h>
#include <error/Message.h>

using namespace libquixcc;

static bool parse_enum_field(quixcc_job_t &job, std::shared_ptr<libquixcc::Scanner> scanner, std::shared_ptr<EnumFieldNode> &node)
{
    // <name> [ = <value> ] [,]

    Token tok = scanner->next();
    if (tok.type() != TokenType::Identifier)
    {
        LOG(ERROR) << feedback[ENUM_FIELD_EXPECTED_IDENTIFIER] << tok << std::endl;
        return false;
    }

    node = std::make_shared<EnumFieldNode>();

    node->m_name = std::get<std::string>(tok.val());

    tok = scanner->peek();
    if (tok.is<Operator>(Operator::Assign))
    {
        scanner->next();
        if (!parse_const_expr(job, scanner, Token(TokenType::Punctor, Punctor::Comma), node->m_value))
        {
            LOG(ERROR) << feedback[ENUM_FIELD_EXPECTED_CONST_EXPR] << node->m_name << tok << std::endl;
            return false;
        }

        tok = scanner->peek();
    }

    if (tok.is<Punctor>(Punctor::Comma))
    {
        scanner->next();
        return true;
    }

    if (!tok.is<Punctor>(Punctor::CloseBrace))
    {
        LOG(ERROR) << feedback[ENUM_FIELD_EXPECTED_SEMICOLON] << tok << std::endl;
        return false;
    }

    return true;
}

bool libquixcc::parse_enum(quixcc_job_t &job, std::shared_ptr<libquixcc::Scanner> scanner, std::shared_ptr<libquixcc::StmtNode> &node)
{
    Token tok = scanner->next();
    if (tok.type() != TokenType::Identifier)
    {
        LOG(ERROR) << feedback[ENUM_EXPECTED_IDENTIFIER] << tok << std::endl;
        return false;
    }

    std::string name = std::get<std::string>(tok.val());

    tok = scanner->next();
    if (!tok.is<Punctor>(Punctor::Colon))
    {
        LOG(ERROR) << feedback[ENUM_EXPECTED_COLON] << tok << std::endl;
        return false;
    }

    TypeNode *type;
    if (!parse_type(job, scanner, &type))
        return false;

    tok = scanner->next();
    if (tok.is<Punctor>(Punctor::Semicolon))
    {
        node = std::make_shared<EnumDeclNode>(EnumTypeNode::create(name, type));
        return true;
    }

    if (!tok.is<Punctor>(Punctor::OpenBrace))
    {
        LOG(ERROR) << feedback[ENUM_EXPECTED_LEFT_BRACE] << tok << std::endl;
        return false;
    }

    std::vector<std::shared_ptr<EnumFieldNode>> fields;

    while (true)
    {
        tok = scanner->peek();
        if (tok.is<Punctor>(Punctor::CloseBrace))
        {
            scanner->next();
            break;
        }

        std::shared_ptr<EnumFieldNode> field;
        if (!parse_enum_field(job, scanner, field))
            return false;
        fields.push_back(field);
    }

    tok = scanner->next();
    if (!tok.is<Punctor>(Punctor::Semicolon))
    {
        LOG(ERROR) << feedback[ENUM_DEF_EXPECTED_SEMICOLON] << tok << std::endl;
        return false;
    }

    auto decl = std::make_shared<EnumDeclNode>(EnumTypeNode::create(name, type));
    node = std::make_shared<EnumDefNode>(decl, true, fields);
    return true;
}