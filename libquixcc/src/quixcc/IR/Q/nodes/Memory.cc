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

#include <quixcc/IR/Q/Memory.h>

boost::uuids::uuid libquixcc::ir::q::Assign::hash_impl() const {
  return Hasher().gettag().add(lhs).add(rhs).hash();
}

bool libquixcc::ir::q::Assign::verify_impl() const { return lhs->verify() && rhs->verify(); }

boost::uuids::uuid libquixcc::ir::q::PostInc::hash_impl() const {
  return Hasher().gettag().add(lhs).hash();
}

bool libquixcc::ir::q::PostInc::verify_impl() const { return lhs->verify(); }

boost::uuids::uuid libquixcc::ir::q::PostDec::hash_impl() const {
  return Hasher().gettag().add(lhs).hash();
}

bool libquixcc::ir::q::PostDec::verify_impl() const { return lhs->verify(); }

boost::uuids::uuid libquixcc::ir::q::AddressOf::hash_impl() const {
  return Hasher().gettag().add(lhs).hash();
}

bool libquixcc::ir::q::AddressOf::verify_impl() const { return lhs->verify(); }

boost::uuids::uuid libquixcc::ir::q::Deref::hash_impl() const {
  return Hasher().gettag().add(lhs).hash();
}

bool libquixcc::ir::q::Deref::verify_impl() const { return lhs->verify(); }

boost::uuids::uuid libquixcc::ir::q::Member::hash_impl() const {
  return Hasher().gettag().add(lhs).add(field).add(field_type).hash();
}

bool libquixcc::ir::q::Member::verify_impl() const { return lhs->verify() && field_type->verify(); }

boost::uuids::uuid libquixcc::ir::q::Index::hash_impl() const {
  return Hasher().gettag().add(lhs).add(index).add(type).hash();
}

bool libquixcc::ir::q::Index::verify_impl() const {
  return lhs->verify() && index->verify() && type->verify();
}