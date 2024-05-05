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

#include <IR/delta/nodes/Control.h>

libquixcc::ir::Result<bool> libquixcc::ir::delta::IfElse::print_impl(std::ostream &os, bool debug) const
{
    os << "if (";
    if (!then->print(os, debug))
        return false;

    os << ") {";
    if (!then->print(os, debug))
        return false;

    os << "} else {";
    if (!els->print(os, debug))
        return false;

    os << "}";
    return true;
}

libquixcc::ir::Result<bool> libquixcc::ir::delta::While::print_impl(std::ostream &os, bool debug) const
{
    os << "while (";
    if (!cond->print(os, debug))
        return false;

    os << ") {";
    if (!body->print(os, debug))
        return false;

    os << "}";
    return true;
}

libquixcc::ir::Result<bool> libquixcc::ir::delta::Jmp::print_impl(std::ostream &os, bool debug) const
{
    os << "jmp " << target;

    return true;
}

libquixcc::ir::Result<bool> libquixcc::ir::delta::Label::print_impl(std::ostream &os, bool debug) const
{
    os << name << ":";
    return true;
}

libquixcc::ir::Result<bool> libquixcc::ir::delta::Ret::print_impl(std::ostream &os, bool debug) const
{
    os << "ret";
    if (value)
    {
        os << " ";
        if (!value->print(os, debug))
            return false;
    }
    return true;
}

libquixcc::ir::Result<bool> libquixcc::ir::delta::Call::print_impl(std::ostream &os, bool debug) const
{
    throw std::runtime_error("Call::print_impl not implemented");
}

libquixcc::ir::Result<bool> libquixcc::ir::delta::PtrCall::print_impl(std::ostream &os, bool debug) const
{
    throw std::runtime_error("PtrCall::print_impl not implemented");
}

libquixcc::ir::Result<bool> libquixcc::ir::delta::Halt::print_impl(std::ostream &os, bool debug) const
{
    os << "halt";
    return true;
}
