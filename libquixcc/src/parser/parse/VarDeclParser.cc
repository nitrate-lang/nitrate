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
///     * Copyright (C) 2024 Wesley C. Jones                                     ///
///                                                                              ///
///     The QUIX Compiler Suite is free software; you can redistribute it and/or ///
///     modify it under the terms of the GNU Lesser General Public               ///
///     License as published by the Free Software Foundation; either             ///
///     version 2.1 of the License, or (at your option) any later version.       ///
///                                                                              ///
///     The QUIX Compiler Suite is distributed in the hope that it will be       ///
///     useful, but WITHOUT ANY WARRANTY; without even the implied warranty of   ///
///     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        ///
///     Lesser General Public License for more details.                          ///
///                                                                              ///
///     You should have received a copy of the GNU Lesser General Public         ///
///     License along with the QUIX Compiler Suite; if not, see                  ///
///     <https://www.gnu.org/licenses/>.                                         ///
///                                                                              ///
////////////////////////////////////////////////////////////////////////////////////

#define QUIXCC_INTERNAL

#include <parse/Parser.h>
#include <LibMacro.h>
#include <core/Logger.h>

using namespace libquixcc;

static bool parse_decl(quixcc_job_t &job, Token tok, std::shared_ptr<libquixcc::Scanner> scanner, std::pair<std::string, libquixcc::TypeNode *> &decl)
{
    std::string name = std::get<std::string>(tok.val());

    tok = scanner->next();
    if (!tok.is<Punctor>(Punctor::Colon))
    {
        LOG(ERROR) << feedback[VAR_DECL_MISSING_COLON] << tok << std::endl;
        return false;
    }

    TypeNode *type;

    if (!parse_type(job, scanner, &type))
    {
        LOG(ERROR) << feedback[VAR_DECL_TYPE_ERR] << name << tok << std::endl;
        return false;
    }

    decl = std::make_pair(name, type);
    return true;
}

bool libquixcc::parse_var(quixcc_job_t &job, std::shared_ptr<libquixcc::Scanner> scanner, std::vector<std::shared_ptr<libquixcc::StmtNode>> &nodes)
{
    Token tok = scanner->next();

    std::vector<std::pair<std::string, libquixcc::TypeNode *>> decls;
    bool multi_decl = false;
    if (tok.is<Punctor>(Punctor::OpenBracket))
    {
        multi_decl = true;
        /*
        var [x: i8, y: i8];
        */

        while (true)
        {
            tok = scanner->next();

            std::pair<std::string, libquixcc::TypeNode *> decl;
            if (!parse_decl(job, tok, scanner, decl))
                return false;

            decls.push_back(decl);

            tok = scanner->next();
            if (tok.is<Punctor>(Punctor::Comma))
                continue;
            else if (tok.is<Punctor>(Punctor::CloseBracket))
                break;
            else
            {
                LOG(ERROR) << feedback[VAR_DECL_MISSING_PUNCTOR] << decl.first << tok << std::endl;
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
        LOG(ERROR) << feedback[VAR_DECL_MISSING_IDENTIFIER] << tok << std::endl;
        return false;
    }

    tok = scanner->next();
    if (tok.is<Punctor>(Punctor::Semicolon))
    {
        // No initializer
        for (auto &decl : decls)
            nodes.push_back(std::make_shared<VarDeclNode>(decl.first, decl.second, nullptr));
    }
    else if (tok.is<Operator>(Operator::Assign))
    {
        if (multi_decl)
            throw std::runtime_error("Initializer not implemented for multiple declarations");

        // Parse initializer
        std::shared_ptr<ExprNode> init;
        if (!parse_expr(job, scanner, {Token(TokenType::Punctor, Punctor::Semicolon)}, init))
            return false;

        tok = scanner->next();
        if (!tok.is<Punctor>(Punctor::Semicolon))
        {
            LOG(ERROR) << feedback[VAR_DECL_MISSING_PUNCTOR] << decls[0].first << tok << std::endl;
            return false;
        }

        nodes.push_back(std::make_shared<VarDeclNode>(decls[0].first, decls[0].second, init));
    }
    else
    {
        LOG(ERROR) << feedback[VAR_DECL_MISSING_PUNCTOR] << tok << std::endl;
        return false;
    }

    return true;
}