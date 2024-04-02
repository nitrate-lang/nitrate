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

bool libquixcc::parse_for(quixcc_job_t &job, std::shared_ptr<libquixcc::Scanner> scanner, std::shared_ptr<libquixcc::StmtNode> &node)
{
    std::shared_ptr<ExprNode> x0, x1, x2;

    Token tok = scanner->peek();
    if (tok.type() == TokenType::Punctor && std::get<Punctor>(tok.val()) == Punctor::OpenParen)
    {
        tok = scanner->next();

        if (!parse_expr(job, scanner, Token(TokenType::Punctor, Punctor::Semicolon), x0))
            return false;

        tok = scanner->next();
        if (!tok.is<Punctor>(Punctor::Semicolon))
        {
            PARMSG(tok, Err::ERROR, feedback[FOR_EXPECTED_SEMICOLON]);
            return false;
        }

        if (!parse_expr(job, scanner, Token(TokenType::Punctor, Punctor::Semicolon), x1))
            return false;

        tok = scanner->next();
        if (!tok.is<Punctor>(Punctor::Semicolon))
        {
            PARMSG(tok, Err::ERROR, feedback[FOR_EXPECTED_SEMICOLON]);
            return false;
        }

        if (!parse_expr(job, scanner, Token(TokenType::Punctor, Punctor::CloseParen), x2))
            return false;

        tok = scanner->next();
        if (!tok.is<Punctor>(Punctor::CloseParen))
        {
            PARMSG(tok, Err::ERROR, feedback[FOR_EXPECTED_CLOSING_PARANTHESIS]);
            return false;
        }

        std::shared_ptr<BlockNode> then_block;
        if (!parse(job, scanner, then_block, true))
            return false;

        node = std::make_shared<ForStmtNode>(x0, x1, x2, then_block);

        return true;
    }
    else
    {
        if (!parse_expr(job, scanner, Token(TokenType::Punctor, Punctor::Semicolon), x0))
            return false;

        tok = scanner->next();
        if (!tok.is<Punctor>(Punctor::Semicolon))
        {
            PARMSG(tok, Err::ERROR, feedback[FOR_EXPECTED_SEMICOLON]);
            return false;
        }

        if (!parse_expr(job, scanner, Token(TokenType::Punctor, Punctor::Semicolon), x1))
            return false;

        tok = scanner->next();
        if (!tok.is<Punctor>(Punctor::Semicolon))
        {
            PARMSG(tok, Err::ERROR, feedback[FOR_EXPECTED_SEMICOLON]);
            return false;
        }

        if (!parse_expr(job, scanner, Token(TokenType::Punctor, Punctor::OpenBrace), x2))
            return false;

        std::shared_ptr<BlockNode> then_block;
        if (!parse(job, scanner, then_block, true))
            return false;

        node = std::make_shared<ForStmtNode>(x0, x1, x2, then_block);

        return true;
    }
}
