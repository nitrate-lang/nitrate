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

#include <IR/Q/Math.h>

boost::uuids::uuid libquixcc::ir::q::Add::hash_impl() const {
  return Hasher().gettag().add(lhs).add(rhs).hash();
}

bool libquixcc::ir::q::Add::verify_impl() const {
  return lhs->verify() && rhs->verify();
}

boost::uuids::uuid libquixcc::ir::q::Sub::hash_impl() const {
  return Hasher().gettag().add(lhs).add(rhs).hash();
}

bool libquixcc::ir::q::Sub::verify_impl() const {
  return lhs->verify() && rhs->verify();
}

boost::uuids::uuid libquixcc::ir::q::Mul::hash_impl() const {
  return Hasher().gettag().add(lhs).add(rhs).hash();
}

bool libquixcc::ir::q::Mul::verify_impl() const {
  return lhs->verify() && rhs->verify();
}

boost::uuids::uuid libquixcc::ir::q::Div::hash_impl() const {
  return Hasher().gettag().add(lhs).add(rhs).hash();
}

bool libquixcc::ir::q::Div::verify_impl() const {
  return lhs->verify() && rhs->verify();
}

boost::uuids::uuid libquixcc::ir::q::Mod::hash_impl() const {
  return Hasher().gettag().add(lhs).add(rhs).hash();
}

bool libquixcc::ir::q::Mod::verify_impl() const {
  return lhs->verify() && rhs->verify();
}

boost::uuids::uuid libquixcc::ir::q::BitAnd::hash_impl() const {
  return Hasher().gettag().add(lhs).add(rhs).hash();
}

bool libquixcc::ir::q::BitAnd::verify_impl() const {
  return lhs->verify() && rhs->verify();
}

boost::uuids::uuid libquixcc::ir::q::BitOr::hash_impl() const {
  return Hasher().gettag().add(lhs).add(rhs).hash();
}

bool libquixcc::ir::q::BitOr::verify_impl() const {
  return lhs->verify() && rhs->verify();
}

boost::uuids::uuid libquixcc::ir::q::BitXor::hash_impl() const {
  return Hasher().gettag().add(lhs).add(rhs).hash();
}

bool libquixcc::ir::q::BitXor::verify_impl() const {
  return lhs->verify() && rhs->verify();
}

boost::uuids::uuid libquixcc::ir::q::BitNot::hash_impl() const {
  return Hasher().gettag().add(operand).hash();
}

bool libquixcc::ir::q::BitNot::verify_impl() const { return operand->verify(); }

boost::uuids::uuid libquixcc::ir::q::Shl::hash_impl() const {
  return Hasher().gettag().add(lhs).add(rhs).hash();
}

bool libquixcc::ir::q::Shl::verify_impl() const {
  return lhs->verify() && rhs->verify();
}

boost::uuids::uuid libquixcc::ir::q::Shr::hash_impl() const {
  return Hasher().gettag().add(lhs).add(rhs).hash();
}

bool libquixcc::ir::q::Shr::verify_impl() const {
  return lhs->verify() && rhs->verify();
}

boost::uuids::uuid libquixcc::ir::q::Rotl::hash_impl() const {
  return Hasher().gettag().add(lhs).add(rhs).hash();
}

bool libquixcc::ir::q::Rotl::verify_impl() const {
  return lhs->verify() && rhs->verify();
}

boost::uuids::uuid libquixcc::ir::q::Rotr::hash_impl() const {
  return Hasher().gettag().add(lhs).add(rhs).hash();
}

bool libquixcc::ir::q::Rotr::verify_impl() const {
  return lhs->verify() && rhs->verify();
}

boost::uuids::uuid libquixcc::ir::q::Eq::hash_impl() const {
  return Hasher().gettag().add(lhs).add(rhs).hash();
}

bool libquixcc::ir::q::Eq::verify_impl() const {
  return lhs->verify() && rhs->verify();
}

boost::uuids::uuid libquixcc::ir::q::Ne::hash_impl() const {
  return Hasher().gettag().add(lhs).add(rhs).hash();
}

bool libquixcc::ir::q::Ne::verify_impl() const {
  return lhs->verify() && rhs->verify();
}

boost::uuids::uuid libquixcc::ir::q::Lt::hash_impl() const {
  return Hasher().gettag().add(lhs).add(rhs).hash();
}

bool libquixcc::ir::q::Lt::verify_impl() const {
  return lhs->verify() && rhs->verify();
}

boost::uuids::uuid libquixcc::ir::q::Gt::hash_impl() const {
  return Hasher().gettag().add(lhs).add(rhs).hash();
}

bool libquixcc::ir::q::Gt::verify_impl() const {
  return lhs->verify() && rhs->verify();
}

boost::uuids::uuid libquixcc::ir::q::Le::hash_impl() const {
  return Hasher().gettag().add(lhs).add(rhs).hash();
}

bool libquixcc::ir::q::Le::verify_impl() const {
  return lhs->verify() && rhs->verify();
}

boost::uuids::uuid libquixcc::ir::q::Ge::hash_impl() const {
  return Hasher().gettag().add(lhs).add(rhs).hash();
}

bool libquixcc::ir::q::Ge::verify_impl() const {
  return lhs->verify() && rhs->verify();
}

boost::uuids::uuid libquixcc::ir::q::And::hash_impl() const {
  return Hasher().gettag().add(lhs).add(rhs).hash();
}

bool libquixcc::ir::q::And::verify_impl() const {
  return lhs->verify() && rhs->verify();
}

boost::uuids::uuid libquixcc::ir::q::Or::hash_impl() const {
  return Hasher().gettag().add(lhs).add(rhs).hash();
}

bool libquixcc::ir::q::Or::verify_impl() const {
  return lhs->verify() && rhs->verify();
}

boost::uuids::uuid libquixcc::ir::q::Not::hash_impl() const {
  return Hasher().gettag().add(operand).hash();
}

bool libquixcc::ir::q::Not::verify_impl() const { return operand->verify(); }

boost::uuids::uuid libquixcc::ir::q::Xor::hash_impl() const {
  return Hasher().gettag().add(lhs).add(rhs).hash();
}

bool libquixcc::ir::q::Xor::verify_impl() const {
  return lhs->verify() && rhs->verify();
}