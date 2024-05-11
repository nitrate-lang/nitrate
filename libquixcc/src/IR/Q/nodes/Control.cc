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

boost::uuids::uuid libquixcc::ir::q::IfElse::hash_impl() const
{
    return Hasher().gettag().add(cond).add(then).add(els).hash();
}

bool libquixcc::ir::q::IfElse::verify_impl() const
{
    return cond->verify() && then->verify() && els->verify();
}

boost::uuids::uuid libquixcc::ir::q::While::hash_impl() const
{
    return Hasher().gettag().add(cond).add(body).hash();
}

bool libquixcc::ir::q::While::verify_impl() const
{
    return cond->verify() && body->verify();
}

boost::uuids::uuid libquixcc::ir::q::For::hash_impl() const
{
    return Hasher().gettag().add(init).add(cond).add(step).add(body).hash();
}

bool libquixcc::ir::q::For::verify_impl() const
{
    return init->verify() && cond->verify() && step->verify() && body->verify();
}

boost::uuids::uuid libquixcc::ir::q::Loop::hash_impl() const
{
    return Hasher().gettag().add(body).hash();
}

bool libquixcc::ir::q::Loop::verify_impl() const
{
    return body->verify();
}

boost::uuids::uuid libquixcc::ir::q::Break::hash_impl() const
{
    return Hasher().gettag().hash();
}

bool libquixcc::ir::q::Break::verify_impl() const
{
    return true;
}

boost::uuids::uuid libquixcc::ir::q::Continue::hash_impl() const
{
    return Hasher().gettag().hash();
}

bool libquixcc::ir::q::Continue::verify_impl() const
{
    return true;
}

boost::uuids::uuid libquixcc::ir::q::Ret::hash_impl() const
{
    auto h = Hasher().gettag();

    if (value)
        h.add(value);

    return h.hash();
}

bool libquixcc::ir::q::Ret::verify_impl() const
{
    if (!value)
        return true;
    
    return value->verify();
}

boost::uuids::uuid libquixcc::ir::q::Throw::hash_impl() const
{
    return Hasher().gettag().add(value).hash();
}

bool libquixcc::ir::q::Throw::verify_impl() const
{
    return value->verify();
}

boost::uuids::uuid libquixcc::ir::q::TryCatchFinally::hash_impl() const
{
    auto h = Hasher().gettag().add(tryblock).add(finallyblock);

    for (auto &c : catchblocks)
        h.add(c.first).add(c.second);

    return h.hash();
}

bool libquixcc::ir::q::TryCatchFinally::verify_impl() const
{
    return tryblock->verify() && std::all_of(catchblocks.begin(), catchblocks.end(), [](const auto &c)
                                             { return c.second->verify(); }) &&
           finallyblock->verify();
}

boost::uuids::uuid libquixcc::ir::q::Case::hash_impl() const
{
    return Hasher().gettag().add(value).add(body).hash();
}

bool libquixcc::ir::q::Case::verify_impl() const
{
    return value->verify() && body->verify();
}

boost::uuids::uuid libquixcc::ir::q::Switch::hash_impl() const
{
    return Hasher().gettag().add(value).add(cases).add(defaultcase).hash();
}

bool libquixcc::ir::q::Switch::verify_impl() const
{
    return value->verify() && std::all_of(cases.begin(), cases.end(), [](const Case *c)
                                          { return c->verify(); }) &&
           defaultcase->verify();
}