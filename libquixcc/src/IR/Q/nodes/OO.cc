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

boost::uuids::uuid libquixcc::ir::q::RegionDef::hash_impl() const
{
    auto hasher = Hasher().gettag();
    for (auto &f : fields)
        hasher.add(f.first).add(f.second);

    for (auto &m : methods)
        hasher.add(m);

    return hasher.hash();
}

bool libquixcc::ir::q::RegionDef::verify_impl() const
{
    for (auto &f : fields)
        if (!f.second->verify())
            return false;

    for (auto &m : methods)
        if (!m->verify())
            return false;

    return true;
}

boost::uuids::uuid libquixcc::ir::q::GroupDef::hash_impl() const
{
    auto hasher = Hasher().gettag();
    for (auto &f : fields)
        hasher.add(f.first).add(f.second);

    for (auto &m : methods)
        hasher.add(m);

    return hasher.hash();
}

bool libquixcc::ir::q::GroupDef::verify_impl() const
{
    for (auto &f : fields)
        if (!f.second->verify())
            return false;

    for (auto &m : methods)
        if (!m->verify())
            return false;

    return true;
}

boost::uuids::uuid libquixcc::ir::q::UnionDef::hash_impl() const
{
    auto hasher = Hasher().gettag();
    for (auto &f : fields)
        hasher.add(f.first).add(f.second);

    for (auto &m : methods)
        hasher.add(m);

    return hasher.hash();
}

bool libquixcc::ir::q::UnionDef::verify_impl() const
{
    for (auto &f : fields)
        if (!f.second->verify())
            return false;

    for (auto &m : methods)
        if (!m->verify())
            return false;

    return true;
}