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

#include <IR/delta/DeltaIR.h>

#include <IR/delta/nodes/Type.h>
#include <IR/delta/nodes/Variable.h>
#include <IR/delta/nodes/Memory.h>
#include <IR/delta/nodes/Cast.h>
#include <IR/delta/nodes/Control.h>
#include <IR/delta/nodes/Segment.h>
#include <IR/delta/nodes/Math.h>

libquixcc::ir::Result<bool> libquixcc::ir::delta::IRDelta::print_impl(std::ostream &os, bool debug) const
{
    if (!m_root)
    {
        os << "IRDelta_1_0(" + m_name + ")";
        return true;
    }

    os << "IRDelta_1_0(" + m_name + ",[";

    Result<bool> result;
    if (debug)
        result = m_root->print<PrintMode::Debug>(os);
    else
        result = m_root->print<PrintMode::Text>(os);

    os << "])";

    return result;
}

libquixcc::ir::Result<bool> libquixcc::ir::delta::IRDelta::deserialize_impl(std::istream &is)
{
    throw std::runtime_error("Not implemented");
}

bool libquixcc::ir::delta::IRDelta::verify_impl() const
{
    if (!m_root)
        return false;

    return m_root->verify();
}

std::string_view libquixcc::ir::delta::IRDelta::ir_dialect_name_impl() const
{
    return "QIR-Delta";
}

unsigned int libquixcc::ir::delta::IRDelta::ir_dialect_version_impl() const
{
    return 1;
}

std::string_view libquixcc::ir::delta::IRDelta::ir_dialect_family_impl() const
{
    return "QIR";
}

std::string_view libquixcc::ir::delta::IRDelta::ir_dialect_description_impl() const
{
    return "Quix Delta Intermediate Representation (QIR-Delta-V1.0) is an intermediate representation for the Quix language.";
}
