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

#include <quixcc/IR/delta/Control.h>

boost::uuids::uuid libquixcc::ir::delta::IfElse::hash_impl() const {
  return Hasher().gettag().add(cond).add(then).add(els).hash();
}

bool libquixcc::ir::delta::IfElse::verify_impl() const {
  return cond->verify() && then->verify() && els->verify();
}

boost::uuids::uuid libquixcc::ir::delta::While::hash_impl() const {
  return Hasher().gettag().add(cond).add(body).hash();
}

bool libquixcc::ir::delta::While::verify_impl() const {
  return cond->verify() && body->verify();
}

boost::uuids::uuid libquixcc::ir::delta::Jmp::hash_impl() const {
  return Hasher().gettag().add(target).hash();
}

bool libquixcc::ir::delta::Jmp::verify_impl() const { return true; }

boost::uuids::uuid libquixcc::ir::delta::Label::hash_impl() const {
  return Hasher().gettag().add(name).add(code).hash();
}

bool libquixcc::ir::delta::Label::verify_impl() const { return code->verify(); }

boost::uuids::uuid libquixcc::ir::delta::Ret::hash_impl() const {
  auto h = Hasher().gettag();
  if (value) h.add(value);
  return h.hash();
}

bool libquixcc::ir::delta::Ret::verify_impl() const {
  if (!value) return true;

  return value->verify();
}

boost::uuids::uuid libquixcc::ir::delta::Call::hash_impl() const {
  auto h = Hasher().gettag().add(callee).add(ftype);
  for (auto arg : args) {
    h.add(arg);
  }

  return h.hash();
}

bool libquixcc::ir::delta::Call::verify_impl() const {
  for (auto arg : args) {
    if (!arg->verify()) return false;
  }

  if (!ftype->verify()) return false;

  return true;
}

boost::uuids::uuid libquixcc::ir::delta::PtrCall::hash_impl() const {
  auto h = Hasher().gettag().add(callee);
  for (auto arg : args) {
    h.add(arg);
  }

  return h.hash();
}

bool libquixcc::ir::delta::PtrCall::verify_impl() const {
  if (!callee->verify()) return false;

  for (auto arg : args) {
    if (!arg->verify()) return false;
  }

  return true;
}

boost::uuids::uuid libquixcc::ir::delta::Halt::hash_impl() const {
  return Hasher().gettag().hash();
}

bool libquixcc::ir::delta::Halt::verify_impl() const { return true; }

boost::uuids::uuid libquixcc::ir::delta::Break::hash_impl() const {
  return Hasher().gettag().hash();
}

bool libquixcc::ir::delta::Break::verify_impl() const { return true; }

boost::uuids::uuid libquixcc::ir::delta::Continue::hash_impl() const {
  return Hasher().gettag().hash();
}

bool libquixcc::ir::delta::Continue::verify_impl() const { return true; }

boost::uuids::uuid libquixcc::ir::delta::Case::hash_impl() const {
  return Hasher().gettag().add(value).add(code).hash();
}

bool libquixcc::ir::delta::Case::verify_impl() const {
  return value->verify() && code->verify();
}

boost::uuids::uuid libquixcc::ir::delta::Switch::hash_impl() const {
  auto h = Hasher().gettag().add(cond);
  for (auto c : cases) {
    h.add(c);
  }

  if (def) h.add(def);

  return h.hash();
}

bool libquixcc::ir::delta::Switch::verify_impl() const {
  if (!cond->verify()) return false;

  for (auto c : cases) {
    if (!c->verify()) return false;
  }

  if (def && !def->verify()) return false;

  return true;
}