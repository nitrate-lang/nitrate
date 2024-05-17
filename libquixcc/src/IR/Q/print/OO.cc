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

#include <IR/Q/OO.h>

libquixcc::ir::Result<bool> libquixcc::ir::q::RegionDef::print_impl(std::ostream &os, libquixcc::ir::PState &state) const
{
    os << "region " << name << " {\n";
    state.ind += 2;
    for (auto it = fields.begin(); it != fields.end(); ++it)
    {
        os << std::string(state.ind, ' ') << it->first << ": ";
        if (!it->second->print(os, state))
            return false;

        os << ";\n";
    }

    if (!methods.empty())
        os << "\n";

    for (auto it = methods.begin(); it != methods.end(); ++it)
    {
        os << std::string(state.ind, ' ');
        if (!(*it)->print(os, state))
            return false;

        os << ";\n";
    }

    state.ind -= 2;

    os << "}";

    return true;
}

libquixcc::ir::Result<bool> libquixcc::ir::q::GroupDef::print_impl(std::ostream &os, libquixcc::ir::PState &state) const
{
    os << "group " << name << " {\n";
    state.ind += 2;
    for (auto it = fields.begin(); it != fields.end(); ++it)
    {
        os << std::string(state.ind, ' ') << it->first << ": ";
        if (!it->second->print(os, state))
            return false;

        os << ";\n";
    }

    if (!methods.empty())
        os << "\n";

    for (auto it = methods.begin(); it != methods.end(); ++it)
    {
        os << std::string(state.ind, ' ');
        if (!(*it)->print(os, state))
            return false;

        os << ";\n";
    }

    state.ind -= 2;

    os << "}";

    return true;
}

libquixcc::ir::Result<bool> libquixcc::ir::q::UnionDef::print_impl(std::ostream &os, libquixcc::ir::PState &state) const
{
    os << "union " << name << " {\n";
    state.ind += 2;
    for (auto it = fields.begin(); it != fields.end(); ++it)
    {
        os << std::string(state.ind, ' ') << it->first << ": ";
        if (!it->second->print(os, state))
            return false;

        os << ";\n";
    }

    if (!methods.empty())
        os << "\n";

    for (auto it = methods.begin(); it != methods.end(); ++it)
    {
        os << std::string(state.ind, ' ');
        if (!(*it)->print(os, state))
            return false;

        os << ";\n";
    }

    state.ind -= 2;

    os << "}";

    return true;
}