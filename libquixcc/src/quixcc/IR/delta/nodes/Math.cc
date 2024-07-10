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

#include <quixcc/IR/delta/Math.h>

boost::uuids::uuid libquixcc::ir::delta::Add::hash_impl() const {
  return Hasher().gettag().add(lhs).add(rhs).hash();
}

bool libquixcc::ir::delta::Add::verify_impl() const {
  return lhs->verify() && rhs->verify();
}

boost::uuids::uuid libquixcc::ir::delta::Sub::hash_impl() const {
  return Hasher().gettag().add(lhs).add(rhs).hash();
}

bool libquixcc::ir::delta::Sub::verify_impl() const {
  return lhs->verify() && rhs->verify();
}

boost::uuids::uuid libquixcc::ir::delta::Mul::hash_impl() const {
  return Hasher().gettag().add(lhs).add(rhs).hash();
}

bool libquixcc::ir::delta::Mul::verify_impl() const {
  return lhs->verify() && rhs->verify();
}

boost::uuids::uuid libquixcc::ir::delta::Div::hash_impl() const {
  return Hasher().gettag().add(lhs).add(rhs).hash();
}

bool libquixcc::ir::delta::Div::verify_impl() const {
  return lhs->verify() && rhs->verify();
}

boost::uuids::uuid libquixcc::ir::delta::Mod::hash_impl() const {
  return Hasher().gettag().add(lhs).add(rhs).hash();
}

bool libquixcc::ir::delta::Mod::verify_impl() const {
  return lhs->verify() && rhs->verify();
}

boost::uuids::uuid libquixcc::ir::delta::BitAnd::hash_impl() const {
  return Hasher().gettag().add(lhs).add(rhs).hash();
}

bool libquixcc::ir::delta::BitAnd::verify_impl() const {
  return lhs->verify() && rhs->verify();
}

boost::uuids::uuid libquixcc::ir::delta::BitOr::hash_impl() const {
  return Hasher().gettag().add(lhs).add(rhs).hash();
}

bool libquixcc::ir::delta::BitOr::verify_impl() const {
  return lhs->verify() && rhs->verify();
}

boost::uuids::uuid libquixcc::ir::delta::BitXor::hash_impl() const {
  return Hasher().gettag().add(lhs).add(rhs).hash();
}

bool libquixcc::ir::delta::BitXor::verify_impl() const {
  return lhs->verify() && rhs->verify();
}

boost::uuids::uuid libquixcc::ir::delta::BitNot::hash_impl() const {
  return Hasher().gettag().add(operand).hash();
}

bool libquixcc::ir::delta::BitNot::verify_impl() const {
  return operand->verify();
}

boost::uuids::uuid libquixcc::ir::delta::Shl::hash_impl() const {
  return Hasher().gettag().add(lhs).add(rhs).hash();
}

bool libquixcc::ir::delta::Shl::verify_impl() const {
  return lhs->verify() && rhs->verify();
}

boost::uuids::uuid libquixcc::ir::delta::Shr::hash_impl() const {
  return Hasher().gettag().add(lhs).add(rhs).hash();
}

bool libquixcc::ir::delta::Shr::verify_impl() const {
  return lhs->verify() && rhs->verify();
}

boost::uuids::uuid libquixcc::ir::delta::Rotl::hash_impl() const {
  return Hasher().gettag().add(lhs).add(rhs).hash();
}

bool libquixcc::ir::delta::Rotl::verify_impl() const {
  return lhs->verify() && rhs->verify();
}

boost::uuids::uuid libquixcc::ir::delta::Rotr::hash_impl() const {
  return Hasher().gettag().add(lhs).add(rhs).hash();
}

bool libquixcc::ir::delta::Rotr::verify_impl() const {
  return lhs->verify() && rhs->verify();
}

boost::uuids::uuid libquixcc::ir::delta::Eq::hash_impl() const {
  return Hasher().gettag().add(lhs).add(rhs).hash();
}

bool libquixcc::ir::delta::Eq::verify_impl() const {
  return lhs->verify() && rhs->verify();
}

boost::uuids::uuid libquixcc::ir::delta::Ne::hash_impl() const {
  return Hasher().gettag().add(lhs).add(rhs).hash();
}

bool libquixcc::ir::delta::Ne::verify_impl() const {
  return lhs->verify() && rhs->verify();
}

boost::uuids::uuid libquixcc::ir::delta::Lt::hash_impl() const {
  return Hasher().gettag().add(lhs).add(rhs).hash();
}

bool libquixcc::ir::delta::Lt::verify_impl() const {
  return lhs->verify() && rhs->verify();
}

boost::uuids::uuid libquixcc::ir::delta::Gt::hash_impl() const {
  return Hasher().gettag().add(lhs).add(rhs).hash();
}

bool libquixcc::ir::delta::Gt::verify_impl() const {
  return lhs->verify() && rhs->verify();
}

boost::uuids::uuid libquixcc::ir::delta::Le::hash_impl() const {
  return Hasher().gettag().add(lhs).add(rhs).hash();
}

bool libquixcc::ir::delta::Le::verify_impl() const {
  return lhs->verify() && rhs->verify();
}

boost::uuids::uuid libquixcc::ir::delta::Ge::hash_impl() const {
  return Hasher().gettag().add(lhs).add(rhs).hash();
}

bool libquixcc::ir::delta::Ge::verify_impl() const {
  return lhs->verify() && rhs->verify();
}

boost::uuids::uuid libquixcc::ir::delta::And::hash_impl() const {
  return Hasher().gettag().add(lhs).add(rhs).hash();
}

bool libquixcc::ir::delta::And::verify_impl() const {
  return lhs->verify() && rhs->verify();
}

boost::uuids::uuid libquixcc::ir::delta::Or::hash_impl() const {
  return Hasher().gettag().add(lhs).add(rhs).hash();
}

bool libquixcc::ir::delta::Or::verify_impl() const {
  return lhs->verify() && rhs->verify();
}

boost::uuids::uuid libquixcc::ir::delta::Not::hash_impl() const {
  return Hasher().gettag().add(operand).hash();
}

bool libquixcc::ir::delta::Not::verify_impl() const {
  return operand->verify();
}

boost::uuids::uuid libquixcc::ir::delta::Xor::hash_impl() const {
  return Hasher().gettag().add(lhs).add(rhs).hash();
}

bool libquixcc::ir::delta::Xor::verify_impl() const {
  return lhs->verify() && rhs->verify();
}