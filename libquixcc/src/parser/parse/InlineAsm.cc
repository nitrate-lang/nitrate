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
///     * Copyright (C) 2020-2024 Wesley C. Jones                                ///
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

/*
    Syntax: __asm__("mov x0, $0", {}, {'$0': x}, ["x0"]);
*/

static bool asm_parse_param(quixcc_job_t &job, std::shared_ptr<libquixcc::Scanner> scanner, std::vector<std::pair<std::string, std::shared_ptr<ExprNode>>> &result)
{
    Token tok;

    tok = scanner->next();
    if (!tok.is<Punctor>(Punctor::OpenBrace))
    {
        LOG(ERROR) << feedback[ASM_EXPECTED_LEFT_BRACE] << tok.serialize() << tok << std::endl;
        return false;
    }

    while (!(tok = scanner->next()).is<Punctor>(Punctor::CloseBrace))
    {
        if (tok.type() != TokenType::StringLiteral)
        {
            LOG(ERROR) << feedback[ASM_PARAM_EXPECTED_STRING_LITERAL] << tok.serialize() << tok << std::endl;
            return false;
        }

        std::string name = std::get<std::string>(tok.val());

        tok = scanner->next();
        if (!tok.is<Punctor>(Punctor::Colon))
        {
            LOG(ERROR) << feedback[ASM_PARAM_EXPECTED_COLON] << tok.serialize() << tok << std::endl;
            return false;
        }

        std::shared_ptr<ExprNode> expr;
        if (!parse_expr(job, scanner, {Token(TokenType::Punctor, Punctor::Comma), Token(TokenType::Punctor, Punctor::CloseBrace)}, expr))
        {
            LOG(ERROR) << feedback[ASM_PARAM_EXPECTED_EXPR] << tok.serialize() << tok << std::endl;
            return false;
        }

        result.push_back({name, expr});
    }

    return true;
}

static bool asm_parse_clobbers(quixcc_job_t &job, std::shared_ptr<libquixcc::Scanner> scanner, std::vector<std::string> &result)
{
    // ['a', 'b', 'c']

    Token tok;

    tok = scanner->next();
    if (!tok.is<Punctor>(Punctor::OpenBracket))
    {
        LOG(ERROR) << feedback[ASM_EXPECTED_LEFT_BRACE] << tok.serialize() << tok << std::endl;
        return false;
    }

    while (!(tok = scanner->next()).is<Punctor>(Punctor::CloseBracket))
    {
        if (tok.type() != TokenType::StringLiteral)
        {
            LOG(ERROR) << feedback[ASM_PARAM_EXPECTED_STRING_LITERAL] << tok.serialize() << tok << std::endl;
            return false;
        }

        result.push_back(std::get<std::string>(tok.val()));
    }

    return true;
}

bool libquixcc::parse_inline_asm(quixcc_job_t &job, std::shared_ptr<libquixcc::Scanner> scanner, std::shared_ptr<libquixcc::StmtNode> &node)
{
    Token tok = scanner->next();
    if (!tok.is<Punctor>(Punctor::OpenParen))
    {
        LOG(ERROR) << feedback[ASM_EXPECTED_LEFT_PAREN] << tok.serialize() << tok << std::endl;
        return false;
    }

    tok = scanner->next();
    if (tok.type() != TokenType::StringLiteral)
    {
        LOG(ERROR) << feedback[ASM_EXPECTED_STRING_LITERAL] << tok.serialize() << tok << std::endl;
        return false;
    }

    std::string asmcode = std::get<std::string>(tok.val());

    tok = scanner->next();
    if (!tok.is<Punctor>(Punctor::Comma))
    {
        LOG(ERROR) << feedback[ASM_EXPECTED_COMMA] << tok.serialize() << tok << std::endl;
        return false;
    }

    std::vector<std::pair<std::string, std::shared_ptr<ExprNode>>> outputs;
    std::vector<std::pair<std::string, std::shared_ptr<ExprNode>>> inputs;

    if (!asm_parse_param(job, scanner, outputs))
    {
        LOG(ERROR) << feedback[ASM_EXPECTED_OUTPUTS] << tok.serialize() << tok << std::endl;
        return false;
    }

    tok = scanner->next();
    if (!tok.is<Punctor>(Punctor::Comma))
    {
        LOG(ERROR) << feedback[ASM_EXPECTED_COMMA] << tok.serialize() << tok << std::endl;
        return false;
    }

    if (!asm_parse_param(job, scanner, inputs))
    {
        LOG(ERROR) << feedback[ASM_EXPECTED_INPUTS] << tok.serialize() << tok << std::endl;
        return false;
    }

    tok = scanner->next();
    if (!tok.is<Punctor>(Punctor::Comma))
    {
        LOG(ERROR) << feedback[ASM_EXPECTED_COMMA] << tok.serialize() << tok << std::endl;
        return false;
    }

    std::vector<std::string> clobbers;
    if (!asm_parse_clobbers(job, scanner, clobbers))
    {
        LOG(ERROR) << feedback[ASM_EXPECTED_CLOBBERS] << tok.serialize() << tok << std::endl;
        return false;
    }

    tok = scanner->next();
    if (!tok.is<Punctor>(Punctor::CloseParen))
    {
        LOG(ERROR) << feedback[ASM_EXPECTED_RIGHT_PAREN] << tok.serialize() << tok << std::endl;
        return false;
    }

    tok = scanner->next();
    if (!tok.is<Punctor>(Punctor::Semicolon))
    {
        LOG(ERROR) << feedback[ASM_EXPECTED_SEMICOLON] << tok.serialize() << tok << std::endl;
        return false;
    }

    node = std::make_shared<InlineAsmNode>(asmcode, outputs, inputs, clobbers);

    return true;
}