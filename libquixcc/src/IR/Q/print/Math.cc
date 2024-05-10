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

#include <IR/Q/Math.h>

libquixcc::ir::Result<bool> libquixcc::ir::q::Add::print_impl(std::ostream &os, PState &state) const
{
    os << "(";
    if (!lhs->print(os, state))
        return false;
    os << "+";
    if (!rhs->print(os, state))
        return false;
    os << ")";
    return true;
}

libquixcc::ir::Result<bool> libquixcc::ir::q::Sub::print_impl(std::ostream &os, PState &state) const
{
    os << "(";
    if (!lhs->print(os, state))
        return false;
    os << "-";
    if (!rhs->print(os, state))
        return false;
    os << ")";
    return true;
}

libquixcc::ir::Result<bool> libquixcc::ir::q::Mul::print_impl(std::ostream &os, PState &state) const
{
    os << "(";
    if (!lhs->print(os, state))
        return false;
    os << "*";
    if (!rhs->print(os, state))
        return false;
    os << ")";
    return true;
}

libquixcc::ir::Result<bool> libquixcc::ir::q::Div::print_impl(std::ostream &os, PState &state) const
{
    os << "(";
    if (!lhs->print(os, state))
        return false;
    os << "/";
    if (!rhs->print(os, state))
        return false;
    os << ")";
    return true;
}

libquixcc::ir::Result<bool> libquixcc::ir::q::Mod::print_impl(std::ostream &os, PState &state) const
{
    os << "(";
    if (!lhs->print(os, state))
        return false;
    os << "%";
    if (!rhs->print(os, state))
        return false;
    os << ")";
    return true;
}

libquixcc::ir::Result<bool> libquixcc::ir::q::BitAnd::print_impl(std::ostream &os, PState &state) const
{
    os << "(";
    if (!lhs->print(os, state))
        return false;
    os << "&";
    if (!rhs->print(os, state))
        return false;
    os << ")";
    return true;
}

libquixcc::ir::Result<bool> libquixcc::ir::q::BitOr::print_impl(std::ostream &os, PState &state) const
{
    os << "(";
    if (!lhs->print(os, state))
        return false;
    os << "|";
    if (!rhs->print(os, state))
        return false;
    os << ")";
    return true;
}

libquixcc::ir::Result<bool> libquixcc::ir::q::BitXor::print_impl(std::ostream &os, PState &state) const
{
    os << "(";
    if (!lhs->print(os, state))
        return false;
    os << "^";
    if (!rhs->print(os, state))
        return false;
    os << ")";
    return true;
}

libquixcc::ir::Result<bool> libquixcc::ir::q::BitNot::print_impl(std::ostream &os, PState &state) const
{
    os << "(~";
    if (!operand->print(os, state))
        return false;
    os << ")";
    return true;
}

libquixcc::ir::Result<bool> libquixcc::ir::q::Shl::print_impl(std::ostream &os, PState &state) const
{
    os << "(";
    if (!lhs->print(os, state))
        return false;
    os << "<<";
    if (!rhs->print(os, state))
        return false;
    os << ")";
    return true;
}

libquixcc::ir::Result<bool> libquixcc::ir::q::Shr::print_impl(std::ostream &os, PState &state) const
{
    os << "(";
    if (!lhs->print(os, state))
        return false;
    os << ">>";
    if (!rhs->print(os, state))
        return false;
    os << ")";
    return true;
}

libquixcc::ir::Result<bool> libquixcc::ir::q::Rotl::print_impl(std::ostream &os, PState &state) const
{
    os << "(";
    if (!lhs->print(os, state))
        return false;
    os << "<~";
    if (!rhs->print(os, state))
        return false;
    os << ")";
    return true;
}

libquixcc::ir::Result<bool> libquixcc::ir::q::Rotr::print_impl(std::ostream &os, PState &state) const
{
    os << "(";
    if (!lhs->print(os, state))
        return false;
    os << "~>";
    if (!rhs->print(os, state))
        return false;
    os << ")";
    return true;
}

libquixcc::ir::Result<bool> libquixcc::ir::q::Eq::print_impl(std::ostream &os, PState &state) const
{
    os << "(";
    if (!lhs->print(os, state))
        return false;
    os << "==";
    if (!rhs->print(os, state))
        return false;
    os << ")";
    return true;
}

libquixcc::ir::Result<bool> libquixcc::ir::q::Ne::print_impl(std::ostream &os, PState &state) const
{
    os << "(";
    if (!lhs->print(os, state))
        return false;
    os << "!=";
    if (!rhs->print(os, state))
        return false;
    os << ")";
    return true;
}

libquixcc::ir::Result<bool> libquixcc::ir::q::Lt::print_impl(std::ostream &os, PState &state) const
{
    os << "(";
    if (!lhs->print(os, state))
        return false;
    os << "<";
    if (!rhs->print(os, state))
        return false;
    os << ")";
    return true;
}

libquixcc::ir::Result<bool> libquixcc::ir::q::Gt::print_impl(std::ostream &os, PState &state) const
{
    os << "(";
    if (!lhs->print(os, state))
        return false;
    os << ">";
    if (!rhs->print(os, state))
        return false;
    os << ")";
    return true;
}

libquixcc::ir::Result<bool> libquixcc::ir::q::Le::print_impl(std::ostream &os, PState &state) const
{
    os << "(";
    if (!lhs->print(os, state))
        return false;
    os << "<=";
    if (!rhs->print(os, state))
        return false;
    os << ")";
    return true;
}

libquixcc::ir::Result<bool> libquixcc::ir::q::Ge::print_impl(std::ostream &os, PState &state) const
{
    os << "(";
    if (!lhs->print(os, state))
        return false;
    os << ">=";
    if (!rhs->print(os, state))
        return false;
    os << ")";
    return true;
}

libquixcc::ir::Result<bool> libquixcc::ir::q::And::print_impl(std::ostream &os, PState &state) const
{
    os << "(";
    if (!lhs->print(os, state))
        return false;
    os << "&&";
    if (!rhs->print(os, state))
        return false;
    os << ")";
    return true;
}

libquixcc::ir::Result<bool> libquixcc::ir::q::Or::print_impl(std::ostream &os, PState &state) const
{
    os << "(";
    if (!lhs->print(os, state))
        return false;
    os << "||";
    if (!rhs->print(os, state))
        return false;
    os << ")";
    return true;
}

libquixcc::ir::Result<bool> libquixcc::ir::q::Not::print_impl(std::ostream &os, PState &state) const
{
    os << "(!";
    if (!operand->print(os, state))
        return false;
    os << ")";
    return true;
}

libquixcc::ir::Result<bool> libquixcc::ir::q::Xor::print_impl(std::ostream &os, PState &state) const
{
    os << "(";
    if (!lhs->print(os, state))
        return false;
    os << "^^";
    if (!rhs->print(os, state))
        return false;
    os << ")";
    return true;
}