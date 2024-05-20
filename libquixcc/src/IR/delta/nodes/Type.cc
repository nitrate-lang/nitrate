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

#include <IR/delta/Type.h>

boost::uuids::uuid libquixcc::ir::delta::I1::hash_impl() const
{
    return Hasher().gettag().hash();
}

bool libquixcc::ir::delta::I1::verify_impl() const
{
    return true;
}

boost::uuids::uuid libquixcc::ir::delta::I8::hash_impl() const
{
    return Hasher().gettag().hash();
}

bool libquixcc::ir::delta::I8::verify_impl() const
{
    return true;
}

boost::uuids::uuid libquixcc::ir::delta::I16::hash_impl() const
{
    return Hasher().gettag().hash();
}

bool libquixcc::ir::delta::I16::verify_impl() const
{
    return true;
}

boost::uuids::uuid libquixcc::ir::delta::I32::hash_impl() const
{
    return Hasher().gettag().hash();
}

bool libquixcc::ir::delta::I32::verify_impl() const
{
    return true;
}

boost::uuids::uuid libquixcc::ir::delta::I64::hash_impl() const
{
    return Hasher().gettag().hash();
}

bool libquixcc::ir::delta::I64::verify_impl() const
{
    return true;
}

boost::uuids::uuid libquixcc::ir::delta::I128::hash_impl() const
{
    return Hasher().gettag().hash();
}

bool libquixcc::ir::delta::I128::verify_impl() const
{
    return true;
}

boost::uuids::uuid libquixcc::ir::delta::U8::hash_impl() const
{
    return Hasher().gettag().hash();
}

bool libquixcc::ir::delta::U8::verify_impl() const
{
    return true;
}

boost::uuids::uuid libquixcc::ir::delta::U16::hash_impl() const
{
    return Hasher().gettag().hash();
}

bool libquixcc::ir::delta::U16::verify_impl() const
{
    return true;
}

boost::uuids::uuid libquixcc::ir::delta::U32::hash_impl() const
{
    return Hasher().gettag().hash();
}

bool libquixcc::ir::delta::U32::verify_impl() const
{
    return true;
}

boost::uuids::uuid libquixcc::ir::delta::U64::hash_impl() const
{
    return Hasher().gettag().hash();
}

bool libquixcc::ir::delta::U64::verify_impl() const
{
    return true;
}

boost::uuids::uuid libquixcc::ir::delta::U128::hash_impl() const
{
    return Hasher().gettag().hash();
}

bool libquixcc::ir::delta::U128::verify_impl() const
{
    return true;
}

boost::uuids::uuid libquixcc::ir::delta::F32::hash_impl() const
{
    return Hasher().gettag().hash();
}

bool libquixcc::ir::delta::F32::verify_impl() const
{
    return true;
}

boost::uuids::uuid libquixcc::ir::delta::F64::hash_impl() const
{
    return Hasher().gettag().hash();
}

bool libquixcc::ir::delta::F64::verify_impl() const
{
    return true;
}

boost::uuids::uuid libquixcc::ir::delta::Void::hash_impl() const
{
    return Hasher().gettag().hash();
}

bool libquixcc::ir::delta::Void::verify_impl() const
{
    return true;
}

boost::uuids::uuid libquixcc::ir::delta::Ptr::hash_impl() const
{
    return Hasher().gettag().add(type).hash();
}

bool libquixcc::ir::delta::Ptr::verify_impl() const
{
    return type->verify();
}

boost::uuids::uuid libquixcc::ir::delta::Packet::hash_impl() const
{
    auto h = Hasher().gettag().add(name);
    for (auto &t : fields)
        h.add(t.first).add(t.second);
    return h.hash();
}

bool libquixcc::ir::delta::Packet::verify_impl() const
{
    for (auto &t : fields)
        if (!t.second->verify())
            return false;
    return true;
}

boost::uuids::uuid libquixcc::ir::delta::Array::hash_impl() const
{
    return Hasher().gettag().add(type).add(size).hash();
}

bool libquixcc::ir::delta::Array::verify_impl() const
{
    return type->verify() && size > 0;
}

boost::uuids::uuid libquixcc::ir::delta::FType::hash_impl() const
{
    auto h = Hasher().gettag().add(ret).add(variadic);
    for (auto &p : params)
        h.add(p);
    return h.hash();
}

bool libquixcc::ir::delta::FType::verify_impl() const
{
    for (auto &p : params)
        if (!p->verify())
            return false;
    return ret->verify();
}


size_t libquixcc::ir::delta::Type::size() const
{
    return std::ceil(bitcount() / 8.0);
}

bool libquixcc::ir::delta::Type::is_ptr() const
{
    return this->is<Ptr>();
}

bool libquixcc::ir::delta::Type::is_integer() const
{
    return this->is<I1>() || this->is<I8>() || this->is<I16>() || this->is<I32>() || this->is<I64>() || this->is<I128>() || this->is<U8>() || this->is<U16>() || this->is<U32>() || this->is<U64>() || this->is<U128>();
}

bool libquixcc::ir::delta::Type::is_float() const
{
    return this->is<F32>() || this->is<F64>();
}

bool libquixcc::ir::delta::Type::is_void() const
{
    return this->is<Void>();
}

bool libquixcc::ir::delta::Type::is_signed() const
{
    return this->is<I1>() || this->is<I8>() || this->is<I16>() || this->is<I32>() || this->is<I64>() || this->is<I128>();
}

bool libquixcc::ir::delta::Type::is_unsigned() const
{
    return this->is<U8>() || this->is<U16>() || this->is<U32>() || this->is<U64>() || this->is<U128>();
}

bool libquixcc::ir::delta::Type::is_primitive() const
{
    return this->is_integer() || this->is_float() || this->is_void();
}