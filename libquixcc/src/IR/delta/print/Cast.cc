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

#include <IR/delta/nodes/Cast.h>

libquixcc::ir::Result<bool> libquixcc::ir::delta::SCast::print_impl(std::ostream &os, PState &state) const
{
    os << "scast ";
    if (!type->print(os, state))
        return false;
    os << ", ";
    if (!value->print(os, state))
        return false;
    return true;
}

libquixcc::ir::Result<bool> libquixcc::ir::delta::UCast::print_impl(std::ostream &os, PState &state) const
{
    os << "ucast ";
    if (!type->print(os, state))
        return false;
    os << ", ";
    if (!value->print(os, state))
        return false;
    return true;
}

libquixcc::ir::Result<bool> libquixcc::ir::delta::PtrICast::print_impl(std::ostream &os, PState &state) const
{
    os << "ptricast ";
    if (!type->print(os, state))
        return false;
    os << ", ";
    if (!value->print(os, state))
        return false;
    return true;
}

libquixcc::ir::Result<bool> libquixcc::ir::delta::IPtrCast::print_impl(std::ostream &os, PState &state) const
{
    os << "iptrcast ";
    if (!type->print(os, state))
        return false;
    os << ", ";
    if (!value->print(os, state))
        return false;
    return true;
}

libquixcc::ir::Result<bool> libquixcc::ir::delta::Bitcast::print_impl(std::ostream &os, PState &state) const
{
    os << "bitcast ";
    if (!type->print(os, state))
        return false;
    os << ", ";
    if (!value->print(os, state))
        return false;
    return true;
}
