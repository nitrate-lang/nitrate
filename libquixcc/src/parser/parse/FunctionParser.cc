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

struct GetPropState
{
    bool did_nothrow;
    bool did_foreign;
    bool did_pure;
    bool did_impure;
    bool did_tsafe;
};

static bool fn_get_property(quixcc_job_t &job, std::shared_ptr<libquixcc::Scanner> scanner, std::shared_ptr<libquixcc::FunctionDeclNode> &node, GetPropState &state)
{
    Token tok = scanner->peek();

    if (tok.type() == TokenType::Keyword && std::get<Keyword>(tok.val()) == Keyword::Nothrow)
    {
        if (state.did_nothrow)
        {
            PARMSG(tok, libquixcc::Err::ERROR, feedback[FN_NO_THROW_ALREADY_SPECIFIED], tok.serialize().c_str());
            return false;
        }

        node->m_nothrow = true;
        scanner->next();
        state.did_nothrow = true;
        return true;
    }

    if (tok.type() == TokenType::Keyword && std::get<Keyword>(tok.val()) == Keyword::Foreign)
    {
        if (state.did_foreign)
        {
            PARMSG(tok, libquixcc::Err::ERROR, feedback[FN_FOREIGN_ALREADY_SPECIFIED], tok.serialize().c_str());
            return false;
        }
        node->m_foreign = true;
        scanner->next();
        state.did_foreign = true;
        return true;
    }

    if (tok.type() == TokenType::Keyword && std::get<Keyword>(tok.val()) == Keyword::Tsafe)
    {
        if (state.did_tsafe)
        {
            PARMSG(tok, libquixcc::Err::ERROR, feedback[FN_THREAD_SAFE_ALREADY_SPECIFIED], tok.serialize().c_str());
            return false;
        }
        node->m_thread_safe = true;
        scanner->next();
        state.did_tsafe = true;
        return true;
    }

    if (tok.type() == TokenType::Keyword && std::get<Keyword>(tok.val()) == Keyword::Pure)
    {
        if (state.did_pure)
        {
            PARMSG(tok, libquixcc::Err::ERROR, feedback[FN_PURE_ALREADY_SPECIFIED], tok.serialize().c_str());
            return false;
        }

        if (state.did_impure)
        {
            PARMSG(tok, libquixcc::Err::ERROR, feedback[FN_PURE_AND_IMPURE_SPECIFIED], tok.serialize().c_str());
            return false;
        }
        node->m_pure = true;
        scanner->next();
        state.did_pure = true;
        return true;
    }

    if (tok.type() == TokenType::Keyword && std::get<Keyword>(tok.val()) == Keyword::Impure)
    {
        if (state.did_impure)
        {
            PARMSG(tok, libquixcc::Err::ERROR, feedback[FN_IMPURE_ALREADY_SPECIFIED], tok.serialize().c_str());
            return false;
        }
        if (state.did_pure)
        {
            PARMSG(tok, libquixcc::Err::ERROR, feedback[FN_PURE_AND_IMPURE_SPECIFIED], tok.serialize().c_str());
            return false;
        }
        node->m_pure = false;
        scanner->next();
        state.did_impure = true;
        return true;
    }

    return false;
}

static bool parse_fn_parameter(quixcc_job_t &job, std::shared_ptr<libquixcc::Scanner> scanner, std::shared_ptr<FunctionParamNode> &param)
{
    return false;
}

bool libquixcc::parse_function(quixcc_job_t &job, std::shared_ptr<libquixcc::Scanner> scanner, std::shared_ptr<libquixcc::StmtNode> &node)
{
    // fn [nothrow] [foreign] [pure | impure] [tsafe] <name> ( [param]... ) [: <type>]; or {}
    auto fndecl = std::make_shared<FunctionDeclNode>();

    GetPropState state = {false, false, false, false, false};

    while (fn_get_property(job, scanner, fndecl, state))
    {
        // get all properties
    }

    Token tok = scanner->next();

    if (tok.type() != TokenType::Identifier)
    {
        PARMSG(tok, libquixcc::Err::ERROR, feedback[FN_EXPECTED_IDENTIFIER], tok.serialize().c_str());
        return false;
    }

    fndecl->m_name = std::get<std::string>(tok.val());

    tok = scanner->next();

    if (tok.type() != TokenType::Punctor || std::get<Punctor>(tok.val()) != Punctor::OpenParen)
    {
        PARMSG(tok, libquixcc::Err::ERROR, feedback[FN_EXPECTED_OPEN_PAREN], tok.serialize().c_str());
        return false;
    }

    while ((tok = scanner->next()).type() != TokenType::Punctor || std::get<Punctor>(tok.val()) != Punctor::CloseParen)
    {
        std::shared_ptr<FunctionParamNode> param;

        if (!parse_fn_parameter(job, scanner, param))
            return false;

        fndecl->m_params.push_back(param);

        tok = scanner->peek();

        if (tok.type() != TokenType::Punctor || (std::get<Punctor>(tok.val()) != Punctor::Comma && std::get<Punctor>(tok.val()) != Punctor::CloseParen))
        {
            PARMSG(tok, libquixcc::Err::ERROR, feedback[FN_EXPECTED_CLOSE_PAREN_OR_COMMA], tok.serialize().c_str());
            return false;
        }
    }

    tok = scanner->peek();

    if (tok.type() == TokenType::Punctor && std::get<Punctor>(tok.val()) == Punctor::Semicolon)
    {
        fndecl->m_return_type = VoidTypeNode::create();
        scanner->next();
        node = fndecl;
        return true;
    }

    if (tok.type() == TokenType::Punctor && std::get<Punctor>(tok.val()) == Punctor::Colon)
    {
        scanner->next();
        if (!parse_type(job, scanner, &fndecl->m_return_type))
            return false;

        tok = scanner->peek();
        if (tok.type() == TokenType::Punctor && std::get<Punctor>(tok.val()) == Punctor::Semicolon)
        {
            scanner->next();
            node = fndecl;
            return true;
        }
    }
    else if (tok.type() != TokenType::Punctor || std::get<Punctor>(tok.val()) != Punctor::OpenBrace)
    {
        PARMSG(tok, libquixcc::Err::ERROR, feedback[FN_EXPECTED_OPEN_BRACE], tok.serialize().c_str());
        return false;
    }

    auto fnbody = std::make_shared<BlockNode>();

    if (!parse(job, scanner, fnbody))
        return false;

    if (!fndecl->m_return_type)
        fndecl->m_return_type = VoidTypeNode::create();

    auto fndef = std::make_shared<FunctionDefNode>();
    fndef->m_decl = fndecl;
    fndef->m_body = fnbody;

    node = fndef;

    return true;
}