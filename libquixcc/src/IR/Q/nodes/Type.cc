////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///  ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
///  ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
///    ░▒▓█▓▒░                                                               ///
///     ░▒▓██▓▒░                                                             ///
///                                                                          ///
///   * QUIX LANG COMPILER - The official compiler for the Quix language.    ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The QUIX Compiler Suite is free software; you can redistribute it or   ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The QUIX Compiler Suite is distributed in the hope that it will be     ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the QUIX Compiler Suite; if not, see                ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#include <IR/Q/Type.h>

boost::uuids::uuid libquixcc::ir::q::I1::hash_impl() const {
  return Hasher().gettag().hash();
}

bool libquixcc::ir::q::I1::verify_impl() const { return true; }

boost::uuids::uuid libquixcc::ir::q::I8::hash_impl() const {
  return Hasher().gettag().hash();
}

bool libquixcc::ir::q::I8::verify_impl() const { return true; }

boost::uuids::uuid libquixcc::ir::q::I16::hash_impl() const {
  return Hasher().gettag().hash();
}

bool libquixcc::ir::q::I16::verify_impl() const { return true; }

boost::uuids::uuid libquixcc::ir::q::I32::hash_impl() const {
  return Hasher().gettag().hash();
}

bool libquixcc::ir::q::I32::verify_impl() const { return true; }

boost::uuids::uuid libquixcc::ir::q::I64::hash_impl() const {
  return Hasher().gettag().hash();
}

bool libquixcc::ir::q::I64::verify_impl() const { return true; }

boost::uuids::uuid libquixcc::ir::q::I128::hash_impl() const {
  return Hasher().gettag().hash();
}

bool libquixcc::ir::q::I128::verify_impl() const { return true; }

boost::uuids::uuid libquixcc::ir::q::U8::hash_impl() const {
  return Hasher().gettag().hash();
}

bool libquixcc::ir::q::U8::verify_impl() const { return true; }

boost::uuids::uuid libquixcc::ir::q::U16::hash_impl() const {
  return Hasher().gettag().hash();
}

bool libquixcc::ir::q::U16::verify_impl() const { return true; }

boost::uuids::uuid libquixcc::ir::q::U32::hash_impl() const {
  return Hasher().gettag().hash();
}

bool libquixcc::ir::q::U32::verify_impl() const { return true; }

boost::uuids::uuid libquixcc::ir::q::U64::hash_impl() const {
  return Hasher().gettag().hash();
}

bool libquixcc::ir::q::U64::verify_impl() const { return true; }

boost::uuids::uuid libquixcc::ir::q::U128::hash_impl() const {
  return Hasher().gettag().hash();
}

bool libquixcc::ir::q::U128::verify_impl() const { return true; }

boost::uuids::uuid libquixcc::ir::q::F32::hash_impl() const {
  return Hasher().gettag().hash();
}

bool libquixcc::ir::q::F32::verify_impl() const { return true; }

boost::uuids::uuid libquixcc::ir::q::F64::hash_impl() const {
  return Hasher().gettag().hash();
}

bool libquixcc::ir::q::F64::verify_impl() const { return true; }

boost::uuids::uuid libquixcc::ir::q::Void::hash_impl() const {
  return Hasher().gettag().hash();
}

bool libquixcc::ir::q::Void::verify_impl() const { return true; }

boost::uuids::uuid libquixcc::ir::q::Ptr::hash_impl() const {
  return Hasher().gettag().add(type).hash();
}

bool libquixcc::ir::q::Ptr::verify_impl() const { return type->verify(); }

boost::uuids::uuid libquixcc::ir::q::Array::hash_impl() const {
  return Hasher().gettag().add(type).add(size).hash();
}

bool libquixcc::ir::q::Array::verify_impl() const {
  return type->verify() && size > 0;
}

boost::uuids::uuid libquixcc::ir::q::Vector::hash_impl() const {
  return Hasher().gettag().add(type).hash();
}

bool libquixcc::ir::q::Vector::verify_impl() const { return type->verify(); }

boost::uuids::uuid libquixcc::ir::q::FType::hash_impl() const {
  auto h = Hasher().gettag().add(ret);
  for (auto &p : params) h.add(p);
  return h.hash();
}

bool libquixcc::ir::q::FType::verify_impl() const {
  for (auto &p : params)
    if (!p->verify()) return false;
  return ret->verify();
}

boost::uuids::uuid libquixcc::ir::q::Region::hash_impl() const {
  return Hasher().gettag().add(name).hash();
}

bool libquixcc::ir::q::Region::verify_impl() const { return !name.empty(); }

boost::uuids::uuid libquixcc::ir::q::Union::hash_impl() const {
  return Hasher().gettag().add(name).hash();
}

bool libquixcc::ir::q::Union::verify_impl() const { return !name.empty(); }

boost::uuids::uuid libquixcc::ir::q::Opaque::hash_impl() const {
  return Hasher().gettag().add(name).hash();
}

bool libquixcc::ir::q::Opaque::verify_impl() const { return !name.empty(); }

size_t libquixcc::ir::q::Type::size() const {
  return std::ceil(bitcount() / 8.0);
}

bool libquixcc::ir::q::Type::is_ptr() const { return this->is<Ptr>(); }

bool libquixcc::ir::q::Type::is_integer() const {
  return this->is<I1>() || this->is<I8>() || this->is<I16>() ||
         this->is<I32>() || this->is<I64>() || this->is<I128>() ||
         this->is<U8>() || this->is<U16>() || this->is<U32>() ||
         this->is<U64>() || this->is<U128>();
}

bool libquixcc::ir::q::Type::is_float() const {
  return this->is<F32>() || this->is<F64>();
}

bool libquixcc::ir::q::Type::is_void() const { return this->is<Void>(); }

bool libquixcc::ir::q::Type::is_signed() const {
  return this->is<I8>() || this->is<I16>() || this->is<I32>() ||
         this->is<I64>() || this->is<I128>();
}

bool libquixcc::ir::q::Type::is_unsigned() const {
  return this->is<I1>() || this->is<U8>() || this->is<U16>() ||
         this->is<U32>() || this->is<U64>() || this->is<U128>();
}

bool libquixcc::ir::q::Type::is_primitive() const {
  return this->is_integer() || this->is_float() || this->is_void();
}