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

struct GetPropState
{
    bool did_nothrow;
    bool did_foreign;
    bool did_impure;
    bool did_tsafe;
};

static bool fn_get_property(quixcc_job_t &job, std::shared_ptr<libquixcc::Scanner> scanner, GetPropState &state)
{
    Token tok = scanner->peek();

    if (tok.is<Keyword>(Keyword::Nothrow))
    {
        if (state.did_nothrow)
        {
            LOG(ERROR) << feedback[FN_NO_THROW_ALREADY_SPECIFIED] << tok << std::endl;
            return false;
        }

        scanner->next();
        state.did_nothrow = true;
        return true;
    }

    if (tok.is<Keyword>(Keyword::Foreign))
    {
        if (state.did_foreign)
        {
            LOG(ERROR) << feedback[FN_FOREIGN_ALREADY_SPECIFIED] << tok << std::endl;
            return false;
        }
        scanner->next();
        state.did_foreign = true;
        return true;
    }

    if (tok.is<Keyword>(Keyword::Tsafe))
    {
        if (state.did_tsafe)
        {
            LOG(ERROR) << feedback[FN_THREAD_SAFE_ALREADY_SPECIFIED] << tok << std::endl;
            return false;
        }
        scanner->next();
        state.did_tsafe = true;
        return true;
    }

    if (tok.is<Keyword>(Keyword::Impure))
    {
        if (state.did_impure)
        {
            LOG(ERROR) << feedback[FN_IMPURE_ALREADY_SPECIFIED] << tok << std::endl;
            return false;
        }
        scanner->next();
        state.did_impure = true;
        return true;
    }

    return false;
}

static bool parse_fn_parameter(quixcc_job_t &job, std::shared_ptr<libquixcc::Scanner> scanner, std::shared_ptr<FunctionParamNode> &param)
{
    /*
     <name> : <type> [?] [= <value>]
    */

    auto tok = scanner->next();

    if (tok.type() != TokenType::Identifier)
    {
        LOG(ERROR) << feedback[FN_PARAM_EXPECTED_IDENTIFIER] << tok << std::endl;
        return false;
    }

    std::string name = std::get<std::string>(tok.val());

    tok = scanner->next();
    if (!tok.is<Punctor>(Punctor::Colon))
    {
        LOG(ERROR) << feedback[FN_PARAM_EXPECTED_COLON] << tok << std::endl;
        return false;
    }

    TypeNode *type;

    if (!parse_type(job, scanner, &type))
    {
        LOG(ERROR) << feedback[FN_PARAM_TYPE_ERR] << tok << std::endl;
        return false;
    }

    tok = scanner->peek();
    bool is_optional = false;
    if (tok.is<Operator>(Operator::Question))
    {
        scanner->next();
        is_optional = true;

        tok = scanner->peek();
    }

    if (tok.is<Operator>(Operator::Assign))
    {
        scanner->next();

        std::shared_ptr<ExprNode> value;
        if (!parse_expr(job, scanner, Token(TokenType::Punctor, Punctor::Comma), value))
        {
            LOG(ERROR) << feedback[FN_PARAM_INIT_ERR] << tok << std::endl;
            return false;
        }

        param = std::make_shared<FunctionParamNode>(name, type, value, is_optional);
    }
    else
    {
        param = std::make_shared<FunctionParamNode>(name, type, nullptr, is_optional);
    }

    return true;
}

bool libquixcc::parse_function(quixcc_job_t &job, std::shared_ptr<libquixcc::Scanner> scanner, std::shared_ptr<libquixcc::StmtNode> &node)
{
    // fn [nothrow] [foreign] [pure] [tsafe] <name> ( [param]... ) [: <type>]; or {}
    auto fndecl = std::make_shared<FunctionDeclNode>();

    GetPropState state = {false, false, false, false};

    while (fn_get_property(job, scanner, state))
    {
        // get all properties
    }

    Token tok = scanner->next();

    if (tok.type() != TokenType::Identifier)
    {
        LOG(ERROR) << feedback[FN_EXPECTED_IDENTIFIER] << tok << std::endl;
        return false;
    }

    fndecl->m_name = std::get<std::string>(tok.val());

    tok = scanner->next();

    if (!tok.is<Punctor>(Punctor::OpenParen))
    {
        LOG(ERROR) << feedback[FN_EXPECTED_OPEN_PAREN] << tok << std::endl;
        return false;
    }

    while (1)
    {
        tok = scanner->peek();
        if (tok.is<Punctor>(Punctor::CloseParen))
        {
            scanner->next();
            break;
        }

        std::shared_ptr<FunctionParamNode> param;
        if (!parse_fn_parameter(job, scanner, param))
        {
            LOG(ERROR) << feedback[FN_PARAM_PARSE_ERROR] << tok << std::endl;
            return false;
        }
        fndecl->m_params.push_back(param);

        tok = scanner->peek();
        if (tok.is<Punctor>(Punctor::Comma))
        {
            scanner->next();
            continue;
        }
    }

    std::vector<TypeNode *> params;
    for (auto &param : fndecl->m_params)
        params.push_back(param->m_type);

    tok = scanner->peek();

    if (tok.is<Punctor>(Punctor::Semicolon))
    {
        fndecl->m_type = FunctionTypeNode::create(VoidTypeNode::create(), params, false, !state.did_impure, state.did_tsafe, state.did_foreign, state.did_nothrow);

        scanner->next();
        node = fndecl;
        return true;
    }

    if (tok.is<Punctor>(Punctor::Colon))
    {
        scanner->next();
        TypeNode *type;

        if (!parse_type(job, scanner, &type))
            return false;

        fndecl->m_type = FunctionTypeNode::create(type, params, false, !state.did_impure, state.did_tsafe, state.did_foreign, state.did_nothrow);

        tok = scanner->peek();
        if (tok.is<Punctor>(Punctor::Semicolon))
        {
            scanner->next();
            node = fndecl;
            return true;
        }
    }
    else if (!tok.is<Punctor>(Punctor::OpenBrace))
    {
        LOG(ERROR) << feedback[FN_EXPECTED_OPEN_BRACE] << tok << std::endl;
        return false;
    }

    auto fnbody = std::make_shared<BlockNode>();

    if (!parse(job, scanner, fnbody))
        return false;

    if (!fndecl->m_type)
        fndecl->m_type = FunctionTypeNode::create(VoidTypeNode::create(), params, false, !state.did_impure, state.did_tsafe, state.did_foreign, state.did_nothrow);

    auto fndef = std::make_shared<FunctionDefNode>();
    fndef->m_decl = fndecl;
    fndef->m_body = fnbody;

    node = fndef;

    return true;
}