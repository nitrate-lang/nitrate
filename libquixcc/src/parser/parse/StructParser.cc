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
#include <error/Logger.h>

using namespace libquixcc;

static bool parse_struct_field(quixcc_job_t &job, std::shared_ptr<libquixcc::Scanner> scanner, std::shared_ptr<StructFieldNode> &node)
{
    Token tok = scanner->next();
    if (tok.type() != TokenType::Identifier)
    {
        LOG(ERROR) << feedback[STRUCT_FIELD_MISSING_IDENTIFIER] << tok << std::endl;
        return false;
    }

    auto name = std::get<std::string>(tok.val());

    tok = scanner->next();
    if (!tok.is<Punctor>(Punctor::Colon))
    {
        LOG(ERROR) << feedback[STRUCT_FIELD_MISSING_COLON] << tok << std::endl;
        return false;
    }

    TypeNode *type;
    if (!parse_type(job, scanner, &type))
    {
        LOG(ERROR) << feedback[STRUCT_FIELD_TYPE_ERR] << name << tok << std::endl;
        return false;
    }

    std::shared_ptr<ConstExprNode> value;

    tok = scanner->next();
    if (tok.is<Punctor>(Punctor::Semicolon))
    {
        node = std::make_shared<StructFieldNode>(name, type);
        return true;
    }
    else if (tok.is<Operator>(Operator::Assign))
    {
        if (!parse_const_expr(job, scanner, Token(TokenType::Punctor, Punctor::Semicolon), value))
        {
            LOG(ERROR) << feedback[STRUCT_FIELD_INIT_ERR] << name << tok << std::endl;
            return false;
        }
    }
    else
    {
        LOG(ERROR) << feedback[STRUCT_FIELD_MISSING_PUNCTOR] << name << tok << std::endl;
        return false;
    }

    tok = scanner->next();
    if (!tok.is<Punctor>(Punctor::Semicolon))
    {
        LOG(ERROR) << feedback[STRUCT_FIELD_MISSING_PUNCTOR] << name << tok << std::endl;
        return false;
    }

    node = std::make_shared<StructFieldNode>(name, type, value);

    return true;
}

bool libquixcc::parse_struct(quixcc_job_t &job, std::shared_ptr<libquixcc::Scanner> scanner, std::shared_ptr<libquixcc::StmtNode> &node)
{
    Token tok = scanner->next();
    if (tok.type() != TokenType::Identifier)
    {
        LOG(ERROR) << feedback[STRUCT_DECL_MISSING_IDENTIFIER] << tok << std::endl;
        return false;
    }

    std::string name = std::get<std::string>(tok.val());

    tok = scanner->next();
    if (tok.is<Punctor>(Punctor::Semicolon))
    {
        node = std::make_shared<StructDeclNode>(name);
        return true;
    }
    else if (!tok.is<Punctor>(Punctor::OpenBrace))
    {
        LOG(ERROR) << feedback[STRUCT_DEF_EXPECTED_OPEN_BRACE] << tok << std::endl;
        return false;
    }

    std::vector<std::shared_ptr<StructFieldNode>> fields;

    while (true)
    {
        tok = scanner->peek();
        if (tok.is<Punctor>(Punctor::CloseBrace))
        {
            scanner->next();
            break;
        }

        std::shared_ptr<StructFieldNode> field;
        if (!parse_struct_field(job, scanner, field))
            return false;
        fields.push_back(field);
    }

    tok = scanner->next();
    if (!tok.is<Punctor>(Punctor::Semicolon))
    {
        LOG(ERROR) << feedback[STRUCT_DEF_EXPECTED_SEMICOLON] << tok << std::endl;
        return false;
    }

    auto sdef = std::make_shared<StructDefNode>();
    sdef->m_name = name;
    sdef->m_fields = fields;
    node = sdef;
    return true;
}