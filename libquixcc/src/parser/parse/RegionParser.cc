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
#include <error/Logger.h>

using namespace libquixcc;

static bool parse_region_field(quixcc_job_t &job, std::shared_ptr<libquixcc::Scanner> scanner, std::shared_ptr<RegionFieldNode> &node)
{
    Token tok = scanner->next();
    if (tok.type() != TokenType::Identifier)
    {
        LOG(ERROR) << feedback[REGION_FIELD_MISSING_IDENTIFIER] << tok << std::endl;
        return false;
    }

    auto name = std::get<std::string>(tok.val());

    tok = scanner->next();
    if (!tok.is<Punctor>(Punctor::Colon))
    {
        LOG(ERROR) << feedback[REGION_FIELD_MISSING_COLON] << tok << std::endl;
        return false;
    }

    TypeNode *type;
    if (!parse_type(job, scanner, &type))
    {
        LOG(ERROR) << feedback[REGION_FIELD_TYPE_ERR] << name << tok << std::endl;
        return false;
    }

    std::shared_ptr<ConstExprNode> value;

    tok = scanner->next();
    if (tok.is<Punctor>(Punctor::Comma))
    {
        node = std::make_shared<RegionFieldNode>(name, type);
        return true;
    }
    else if (tok.is<Operator>(Operator::Assign))
    {
        if (!parse_const_expr(job, scanner, Token(TokenType::Punctor, Punctor::Comma), value))
        {
            LOG(ERROR) << feedback[REGION_FIELD_INIT_ERR] << name << tok << std::endl;
            return false;
        }
    }
    else
    {
        LOG(ERROR) << feedback[REGION_FIELD_MISSING_PUNCTOR] << name << tok << std::endl;
        return false;
    }

    tok = scanner->next();
    if (!tok.is<Punctor>(Punctor::Comma))
    {
        LOG(ERROR) << feedback[REGION_FIELD_MISSING_PUNCTOR] << name << tok << std::endl;
        return false;
    }

    node = std::make_shared<RegionFieldNode>(name, type, value);

    return true;
}

bool libquixcc::parse_region(quixcc_job_t &job, std::shared_ptr<libquixcc::Scanner> scanner, std::shared_ptr<libquixcc::StmtNode> &node)
{
    Token tok = scanner->next();
    if (tok.type() != TokenType::Identifier)
    {
        LOG(ERROR) << feedback[REGION_DECL_MISSING_IDENTIFIER] << tok << std::endl;
        return false;
    }

    std::string name = std::get<std::string>(tok.val());

    tok = scanner->next();
    if (!tok.is<Punctor>(Punctor::OpenBrace))
    {
        LOG(ERROR) << feedback[REGION_DEF_EXPECTED_OPEN_BRACE] << tok << std::endl;
        return false;
    }

    std::vector<std::shared_ptr<RegionFieldNode>> fields;

    while (true)
    {
        tok = scanner->peek();
        if (tok.is<Punctor>(Punctor::CloseBrace))
        {
            scanner->next();
            break;
        }

        std::shared_ptr<RegionFieldNode> field;
        if (!parse_region_field(job, scanner, field))
            return false;
        fields.push_back(field);
    }

    auto sdef = std::make_shared<RegionDefNode>();
    sdef->m_name = name;
    sdef->m_fields = fields;
    node = sdef;
    return true;
}