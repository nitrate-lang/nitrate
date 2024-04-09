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

#include <parse/Parser.h>
#include <error/Logger.h>
#include <LibMacro.h>

bool libquixcc::parse_subsystem(quixcc_job_t &job, std::shared_ptr<libquixcc::Scanner> scanner, std::shared_ptr<libquixcc::StmtNode> &node)
{
    Token tok = scanner->next();
    if (tok.type() != TokenType::Identifier)
    {
        LOG(ERROR) << feedback[SUBSYSTEM_MISSING_IDENTIFIER] << tok << std::endl;
        return false;
    }

    std::string name = std::get<std::string>(tok.val());
    std::set<std::string> deps;

    tok = scanner->peek();

    // check if : item1, item2, item3
    if (tok.is<Punctor>(Punctor::Colon))
    {
        scanner->next(); // consume colon
        tok = scanner->next();
        if (tok.type() != TokenType::Identifier)
        {
            LOG(ERROR) << feedback[SUBSYSTEM_EXPECTED_IDENTIFIER] << tok << std::endl;
            return false;
        }
        deps.insert(std::get<std::string>(tok.val()));

        tok = scanner->peek();
        while (tok.is<Punctor>(Punctor::Comma))
        {
            scanner->next(); // consume comma
            tok = scanner->next();
            if (tok.type() != TokenType::Identifier)
            {
                LOG(ERROR) << feedback[SUBSYSTEM_EXPECTED_IDENTIFIER] << tok << std::endl;
                return false;
            }
            deps.insert(std::get<std::string>(tok.val()));
            tok = scanner->peek();
        }
    }

    std::shared_ptr<BlockNode> block;
    if (!parse(job, scanner, block))
        return false;

    tok = scanner->next();
    if (!tok.is<Punctor>(Punctor::Semicolon))
    {
        LOG(ERROR) << feedback[SUBSYSTEM_EXPECTED_SEMICOLON] << tok << std::endl;
        return false;
    }

    node = std::make_shared<SubsystemNode>(name, deps, block);

    return true;
}