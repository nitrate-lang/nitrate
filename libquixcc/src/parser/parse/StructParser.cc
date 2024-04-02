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

static bool parse_struct_field(quixcc_job_t &job, std::shared_ptr<libquixcc::Scanner> scanner, std::shared_ptr<StructFieldNode> &node)
{
    Token tok = scanner->next();
    if (tok.type() != TokenType::Identifier)
    {
        PARMSG(tok, libquixcc::Err::ERROR, feedback[STRUCT_FIELD_MISSING_IDENTIFIER]);
        return false;
    }

    auto name = std::get<std::string>(tok.val());

    tok = scanner->next();
    if (!tok.is<Punctor>(Punctor::Colon))
    {
        PARMSG(tok, libquixcc::Err::ERROR, feedback[STRUCT_FIELD_MISSING_COLON]);
        return false;
    }

    TypeNode *type;
    if (!parse_type(job, scanner, &type))
    {
        PARMSG(tok, libquixcc::Err::ERROR, feedback[STRUCT_FIELD_TYPE_ERR], name.c_str());
        return false;
    }

    std::shared_ptr<ConstExprNode> value;

    tok = scanner->next();
    if (tok.type() == TokenType::Punctor && std::get<Punctor>(tok.val()) == Punctor::Semicolon)
    {
        node = std::make_shared<StructFieldNode>(name, type);
        return true;
    }
    else if (tok.type() == TokenType::Operator && std::get<Operator>(tok.val()) == Operator::Assign)
    {
        if (!parse_const_expr(job, scanner, Token(TokenType::Punctor, Punctor::Semicolon), value))
        {
            PARMSG(tok, libquixcc::Err::ERROR, feedback[STRUCT_FIELD_INIT_ERR], name.c_str());
            return false;
        }
    }
    else
    {
        PARMSG(tok, libquixcc::Err::ERROR, feedback[STRUCT_FIELD_MISSING_PUNCTOR], name.c_str());
        return false;
    }

    tok = scanner->next();
    if (!tok.is<Punctor>(Punctor::Semicolon))
    {
        PARMSG(tok, libquixcc::Err::ERROR, feedback[STRUCT_FIELD_MISSING_PUNCTOR], name.c_str());
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
        PARMSG(tok, libquixcc::Err::ERROR, feedback[STRUCT_DECL_MISSING_IDENTIFIER]);
        return false;
    }

    std::string name = std::get<std::string>(tok.val());

    tok = scanner->next();
    if (tok.type() == TokenType::Punctor && std::get<Punctor>(tok.val()) == Punctor::Semicolon)
    {
        node = std::make_shared<StructDeclNode>(name);
        return true;
    }
    else if (!tok.is<Punctor>(Punctor::OpenBrace))
    {
        PARMSG(tok, libquixcc::Err::ERROR, feedback[STRUCT_DEF_EXPECTED_OPEN_BRACE]);
        return false;
    }

    std::vector<std::shared_ptr<StructFieldNode>> fields;

    while (true)
    {
        tok = scanner->peek();
        if (tok.type() == TokenType::Punctor && std::get<Punctor>(tok.val()) == Punctor::CloseBrace)
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
        PARMSG(tok, libquixcc::Err::ERROR, feedback[STRUCT_DEF_EXPECTED_SEMICOLON]);
        return false;
    }

    auto sdef = std::make_shared<StructDefNode>();
    sdef->m_name = name;
    sdef->m_fields = fields;
    node = sdef;
    return true;
}