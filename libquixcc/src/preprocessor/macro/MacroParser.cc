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

#include <preprocessor/macro/MacroParser.h>
#include <stdexcept>
#include <map>
#include <LibMacro.h>
#include <core/Logger.h>

static std::string trim(const std::string &str)
{
    return str.substr(str.find_first_not_of(" \t\n\r\f\v"), str.find_last_not_of(" \t\n\r\f\v") + 1);
}

bool libquixcc::MacroParser::parse(const libquixcc::Token &macro, std::vector<libquixcc::Token> &exp) const
{
    std::string content = trim(std::get<std::string>(macro.val()));

    if (macro.type() == TokenType::MacroSingleLine)
    {
        std::string directive;
        std::string parameter;
        size_t start = content.find('(');
        if (start != std::string::npos)
        {
            size_t end = content.find(')', start);
            if (end == std::string::npos)
            {
                LOG(ERROR) << "Invalid macro directive: {}" << content << macro << std::endl;
                return false;
            }

            directive = trim(content.substr(0, start));
            if (start + 1 < content.size())
                parameter = content.substr(start + 1, end - start - 1);

            if (!m_routines.contains(directive))
                LOG(ERROR) << "Unknown macro directive: {}" << directive << macro << std::endl;

            if (!m_routines.at(directive)(job, macro, directive, parameter, exp))
                LOG(ERROR) << "Failed to process macro directive: {}" << directive << macro << std::endl;

            return true;
        }
        else
        {
            for (auto &routine : m_routines)
            {
                if (content.starts_with(routine.first))
                {
                    if (!routine.second(job, macro, routine.first, content.substr(routine.first.size()), exp))
                        LOG(ERROR) << "Failed to process macro directive: {}" << routine.first << macro << std::endl;

                    return true;
                }
            }

            LOG(ERROR) << "Unknown macro directive: {}" << content << macro << std::endl;
            return false;
        }
    }
    else if (macro.type() == TokenType::MacroBlock)
    {
        /// TODO: implement block macro parsing
        return parse(Token(TokenType::MacroSingleLine, content), exp);
    }
    else
    {
        throw std::runtime_error("Invalid macro type");
    }

    return false;
}