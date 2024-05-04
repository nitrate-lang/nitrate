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

#include <IR/delta/nodes/Type.h>

libquixcc::ir::Result<bool> libquixcc::ir::delta::I1::print_impl(std::ostream &os, bool debug) const
{
    return os << "i1", true;
}

libquixcc::ir::Result<bool> libquixcc::ir::delta::I8::print_impl(std::ostream &os, bool debug) const
{
    return os << "i8", true;
}

libquixcc::ir::Result<bool> libquixcc::ir::delta::I16::print_impl(std::ostream &os, bool debug) const
{
    return os << "i16", true;
}

libquixcc::ir::Result<bool> libquixcc::ir::delta::I32::print_impl(std::ostream &os, bool debug) const
{
    return os << "i32", true;
}

libquixcc::ir::Result<bool> libquixcc::ir::delta::I64::print_impl(std::ostream &os, bool debug) const
{
    return os << "i64", true;
}

libquixcc::ir::Result<bool> libquixcc::ir::delta::I128::print_impl(std::ostream &os, bool debug) const
{
    return os << "i128", true;
}

libquixcc::ir::Result<bool> libquixcc::ir::delta::I256::print_impl(std::ostream &os, bool debug) const
{
    return os << "i256", true;
}

libquixcc::ir::Result<bool> libquixcc::ir::delta::U8::print_impl(std::ostream &os, bool debug) const
{
    return os << "u8", true;
}

libquixcc::ir::Result<bool> libquixcc::ir::delta::U16::print_impl(std::ostream &os, bool debug) const
{
    return os << "u16", true;
}

libquixcc::ir::Result<bool> libquixcc::ir::delta::U32::print_impl(std::ostream &os, bool debug) const
{
    return os << "u32", true;
}

libquixcc::ir::Result<bool> libquixcc::ir::delta::U64::print_impl(std::ostream &os, bool debug) const
{
    return os << "u64", true;
}

libquixcc::ir::Result<bool> libquixcc::ir::delta::U128::print_impl(std::ostream &os, bool debug) const
{
    return os << "u128", true;
}

libquixcc::ir::Result<bool> libquixcc::ir::delta::U256::print_impl(std::ostream &os, bool debug) const
{
    return os << "u256", true;
}

libquixcc::ir::Result<bool> libquixcc::ir::delta::F32::print_impl(std::ostream &os, bool debug) const
{
    return os << "f32", true;
}

libquixcc::ir::Result<bool> libquixcc::ir::delta::F64::print_impl(std::ostream &os, bool debug) const
{
    return os << "f64", true;
}

libquixcc::ir::Result<bool> libquixcc::ir::delta::Void::print_impl(std::ostream &os, bool debug) const
{
    return os << "void", true;
}

libquixcc::ir::Result<bool> libquixcc::ir::delta::Ptr::print_impl(std::ostream &os, bool debug) const
{
    if (!type->print(os, debug))
        return false;
    return os << "*", true;
}

libquixcc::ir::Result<bool> libquixcc::ir::delta::Packet::print_impl(std::ostream &os, bool debug) const
{
    os << "{";
    for (size_t i = 0; i < fields.size(); i++)
    {
        os << fields[i].first << ": ";
        if (!fields[i].second->print(os, debug))
            return false;
        if (i + 1 < fields.size())
            os << ", ";
    }
    return os << "}", true;
}

libquixcc::ir::Result<bool> libquixcc::ir::delta::Array::print_impl(std::ostream &os, bool debug) const
{
    os << "[";
    if (!type->print(os, debug))
        return false;
    return os << "; " << size << "]", true;
}