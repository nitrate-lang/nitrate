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

bool libquixcc::parse_return(quixcc_job_t &job, std::shared_ptr<libquixcc::Scanner> scanner, std::shared_ptr<libquixcc::StmtNode> &node)
{
    Token tok = scanner->peek();

    if (tok.is<Punctor>(Punctor::Semicolon))
    {
        scanner->next();
        node = std::make_shared<ReturnStmtNode>(nullptr); // void
        return true;
    }

    std::shared_ptr<ExprNode> expr;
    if (!parse_expr(job, scanner, Token(TokenType::Punctor, Punctor::Semicolon), expr))
        return false;
    node = std::make_shared<ReturnStmtNode>(expr);

    tok = scanner->next();

    if (!tok.is<Punctor>(Punctor::Semicolon))
    {
        PARMSG(tok, libquixcc::E::ERROR, feedback[RETIF_MISSING_SEMICOLON]);
        return false;
    }

    return true;
}

bool libquixcc::parse_retif(quixcc_job_t &job, std::shared_ptr<libquixcc::Scanner> scanner, std::shared_ptr<libquixcc::StmtNode> &node)
{
    /*
        Syntax:
            "retif" <return_expr>, <condition>;

        Translates to:
        if (<condition>)
        {
            return <return_expr>;
        }
    */

    Token tok;

    std::shared_ptr<ExprNode> return_expr;
    if (!parse_expr(job, scanner, Token(TokenType::Punctor, Punctor::Comma), return_expr))
        return false;

    tok = scanner->next();
    if (!tok.is<Punctor>(Punctor::Comma))
    {
        PARMSG(tok, libquixcc::E::ERROR, feedback[RETIF_MISSING_COMMA]);
        return false;
    }

    std::shared_ptr<ExprNode> condition;
    if (!parse_expr(job, scanner, Token(TokenType::Punctor, Punctor::Semicolon), condition))
        return false;

    tok = scanner->next();
    if (!tok.is<Punctor>(Punctor::Semicolon))
    {
        PARMSG(tok, libquixcc::E::ERROR, feedback[RETIF_MISSING_SEMICOLON]);
        return false;
    }
    node = std::make_shared<RetifStmtNode>(condition, return_expr);

    return true;
}

bool libquixcc::parse_retz(quixcc_job_t &job, std::shared_ptr<libquixcc::Scanner> scanner, std::shared_ptr<libquixcc::StmtNode> &node)
{
    /*
        Syntax:
            "retz" <return_expr>, <condition>;

        Translates to:
        if (<condition>)
        {
            return <return_expr>;
        }
    */

    Token tok;

    std::shared_ptr<ExprNode> return_expr;
    if (!parse_expr(job, scanner, Token(TokenType::Punctor, Punctor::Comma), return_expr))
        return false;

    tok = scanner->next();
    if (!tok.is<Punctor>(Punctor::Comma))
    {
        PARMSG(tok, libquixcc::E::ERROR, feedback[RETZ_MISSING_COMMA]);
        return false;
    }

    std::shared_ptr<ExprNode> condition;
    if (!parse_expr(job, scanner, Token(TokenType::Punctor, Punctor::Semicolon), condition))
        return false;

    tok = scanner->next();
    if (!tok.is<Punctor>(Punctor::Semicolon))
    {
        PARMSG(tok, libquixcc::E::ERROR, feedback[RETZ_MISSING_SEMICOLON]);
        return false;
    }
    node = std::make_shared<RetzStmtNode>(condition, return_expr);

    return true;
}

bool libquixcc::parse_retv(quixcc_job_t &job, std::shared_ptr<libquixcc::Scanner> scanner, std::shared_ptr<libquixcc::StmtNode> &node)
{
    /*
        Syntax:
            "retv" <condition>;

        Translates to:
        if (<condition>)
        {
            return;
        }
    */

    Token tok;

    std::shared_ptr<ExprNode> cond;
    if (!parse_expr(job, scanner, Token(TokenType::Punctor, Punctor::Semicolon), cond))
        return false;

    tok = scanner->next();
    if (!tok.is<Punctor>(Punctor::Semicolon))
    {
        PARMSG(tok, libquixcc::E::ERROR, feedback[RETV_MISSING_SEMICOLON]);
        return false;
    }

    node = std::make_shared<RetvStmtNode>(cond);

    return true;
}