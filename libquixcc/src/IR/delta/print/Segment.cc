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

#include <IR/delta/Segment.h>

bool libquixcc::ir::delta::Block::print_impl(std::ostream &os, libquixcc::ir::PState &state) const
{
    if (stmts.empty())
    {
        os << "{}";
        return true;
    }

    os << "{\n";
    state.ind += 2;

    for (auto it = stmts.begin(); it != stmts.end(); it++)
    {
        os << std::string(state.ind, ' ');

        if (!(*it)->print(os, state))
            return false;

        os << ";";

        if (it != stmts.end())
            os << "\n";
    }

    state.ind -= 2;
    os << std::string(state.ind, ' ') << "}";

    return true;
}

bool libquixcc::ir::delta::Segment::print_impl(std::ostream &os, PState &state) const
{
    os << "segment ";

    if (pure)
        os << "pure";
    else
        os << "impure";

    os << " (";
    for (uint64_t i = 0; i < params.size(); i++)
    {
        os << params[i].first << " : ";
        if (!params[i].second->print(os, state))
            return false;
        if (i + 1 < params.size())
            os << ", ";
    }
    os << ") (";
    if (!ret->print(os, state))
        return false;
    os << ")";

    if (!block)
        return true;
    else
        os << " ";

    if (!block->print(os, state))
        return false;

    return true;
}

bool libquixcc::ir::delta::RootNode::print_impl(std::ostream &os, PState &state) const
{
    for (auto &child : children)
    {
        if (!child->print(os, state))
            return false;
    }

    return true;
}