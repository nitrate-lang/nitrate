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

static bool parse_decl(quixcc_job_t &job, Token tok, std::shared_ptr<libquixcc::Scanner> scanner, std::pair<std::string, libquixcc::TypeNode *> &decl)
{
    std::string name = std::get<std::string>(tok.val());

    tok = scanner->next();
    if (tok.type() != TokenType::Punctor || std::get<Punctor>(tok.val()) != Punctor::Colon)
    {
        PARMSG(tok, libquixcc::Err::ERROR, feedback[LET_DECL_MISSING_COLON]);
        return false;
    }

    TypeNode *type;

    if (!parse_type(job, scanner, &type))
    {
        PARMSG(tok, libquixcc::Err::ERROR, feedback[LET_DECL_TYPE_ERR], name.c_str());
        return false;
    }

    decl = std::make_pair(name, type);
    return true;
}

bool libquixcc::parse_let(quixcc_job_t &job, std::shared_ptr<libquixcc::Scanner> scanner, std::vector<std::shared_ptr<libquixcc::StmtNode>> &nodes)
{
    Token tok = scanner->next();

    std::vector<std::pair<std::string, libquixcc::TypeNode *>> decls;
    bool multi_decl = false;
    if (tok.type() == TokenType::Punctor && std::get<Punctor>(tok.val()) == Punctor::OpenBracket)
    {
        multi_decl = true;
        /*
        let [x: i8, y: i8];
        */

        while (true)
        {
            tok = scanner->next();

            std::pair<std::string, libquixcc::TypeNode *> decl;
            if (!parse_decl(job, tok, scanner, decl))
                return false;

            decls.push_back(decl);

            tok = scanner->next();
            if (tok.type() == TokenType::Punctor && std::get<Punctor>(tok.val()) == Punctor::Comma)
                continue;
            else if (tok.type() == TokenType::Punctor && std::get<Punctor>(tok.val()) == Punctor::CloseBracket)
                break;
            else
            {
                PARMSG(tok, libquixcc::Err::ERROR, feedback[LET_DECL_MISSING_PUNCTOR], decl.first.c_str());
                return false;
            }
        }
    }
    else if (tok.type() == TokenType::Identifier)
    {
        // Parse single variable declaration
        std::pair<std::string, libquixcc::TypeNode *> decl;
        if (!parse_decl(job, tok, scanner, decl))
            return false;

        decls.push_back(decl);
    }
    else
    {
        PARMSG(tok, libquixcc::Err::ERROR, feedback[LET_DECL_MISSING_IDENTIFIER]);
        return false;
    }

    if (decls.empty())
    {
        PARMSG(tok, libquixcc::Err::ERROR, feedback[LET_DECL_MISSING_IDENTIFIER]);
        return false;
    }

    tok = scanner->next();
    if (tok.type() == TokenType::Punctor && std::get<Punctor>(tok.val()) == Punctor::Semicolon)
    {
        // No initializer
        for (auto &decl : decls)
            nodes.push_back(std::make_shared<LetDeclNode>(decl.first, decl.second, nullptr));
    }
    else if (tok.type() == TokenType::Operator && std::get<Operator>(tok.val()) == Operator::Assign)
    {
        if (multi_decl)
            throw std::runtime_error("Initializer not implemented for multiple declarations");

        // Parse initializer
        std::shared_ptr<ExprNode> init;
        if (!parse_expr(job, scanner, Token(TokenType::Punctor, Punctor::Semicolon), init))
            return false;

        tok = scanner->next();
        if (tok.type() != TokenType::Punctor || std::get<Punctor>(tok.val()) != Punctor::Semicolon)
        {
            PARMSG(tok, libquixcc::Err::ERROR, feedback[LET_DECL_MISSING_PUNCTOR], decls[0].first.c_str());
            return false;
        }

        nodes.push_back(std::make_shared<LetDeclNode>(decls[0].first, decls[0].second, init));
    }
    else
    {
        PARMSG(tok, libquixcc::Err::ERROR, feedback[LET_DECL_MISSING_PUNCTOR]);
        return false;
    }

    return true;
}