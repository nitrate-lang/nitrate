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

#include <IR/Q/Control.h>

libquixcc::ir::Result<bool> libquixcc::ir::q::IfElse::print_impl(std::ostream &os, libquixcc::ir::PState &state) const
{
    os << "if (";
    if (!cond->print(os, state))
        return false;
    os << ") ";
    if (!then->print(os, state))
        return false;
    os << " else ";
    if (!els->print(os, state))
        return false;
    return true;
}

libquixcc::ir::Result<bool> libquixcc::ir::q::While::print_impl(std::ostream &os, libquixcc::ir::PState &state) const
{
    os << "while (";
    if (!cond->print(os, state))
        return false;
    os << ") ";
    if (!body->print(os, state))
        return false;
    return true;
}

libquixcc::ir::Result<bool> libquixcc::ir::q::For::print_impl(std::ostream &os, libquixcc::ir::PState &state) const
{
    os << "for (";
    if (!init->print(os, state))
        return false;
    os << "; ";
    if (!cond->print(os, state))
        return false;
    os << "; ";
    if (!step->print(os, state))
        return false;
    os << ") ";
    if (!body->print(os, state))
        return false;
    return true;
}

libquixcc::ir::Result<bool> libquixcc::ir::q::Loop::print_impl(std::ostream &os, libquixcc::ir::PState &state) const
{
    os << "loop ";
    if (!body->print(os, state))
        return false;
    return true;
}

libquixcc::ir::Result<bool> libquixcc::ir::q::Break::print_impl(std::ostream &os, libquixcc::ir::PState &state) const
{
    os << "break;";
    return true;
}

libquixcc::ir::Result<bool> libquixcc::ir::q::Continue::print_impl(std::ostream &os, libquixcc::ir::PState &state) const
{
    os << "continue;";
    return true;
}

libquixcc::ir::Result<bool> libquixcc::ir::q::Ret::print_impl(std::ostream &os, libquixcc::ir::PState &state) const
{
    os << "ret";

    if (value)
    {
        os << " ";
        if (!value->print(os, state))
            return false;
    }
    os << ";";
    return true;
}

libquixcc::ir::Result<bool> libquixcc::ir::q::Throw::print_impl(std::ostream &os, libquixcc::ir::PState &state) const
{
    os << "throw ";
    if (!value->print(os, state))
        return false;
    return true;
}

libquixcc::ir::Result<bool> libquixcc::ir::q::TryCatchFinally::print_impl(std::ostream &os, libquixcc::ir::PState &state) const
{
    os << "try ";
    if (!tryblock->print(os, state))
        return false;
    for (const auto &catchblock : catchblocks)
    {
        os << "catch (";
        if (!catchblock.first->print(os, state))
            return false;
        os << ") ";
        if (!catchblock.second->print(os, state))
            return false;
    }

    os << "finally ";
    if (!finallyblock->print(os, state))
        return false;

    return true;
}

libquixcc::ir::Result<bool> libquixcc::ir::q::Case::print_impl(std::ostream &os, libquixcc::ir::PState &state) const
{
    os << "case ";
    if (!value->print(os, state))
        return false;
    os << ": ";
    if (!body->print(os, state))
        return false;
    return true;
}

libquixcc::ir::Result<bool> libquixcc::ir::q::Switch::print_impl(std::ostream &os, libquixcc::ir::PState &state) const
{
    os << "switch (";
    if (!value->print(os, state))
        return false;
    os << ") {";
    for (const auto &c : cases)
    {
        if (!c->print(os, state))
            return false;
    }
    os << "}";
    return true;
}