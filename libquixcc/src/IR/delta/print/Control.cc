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

#include <IR/delta/Control.h>

bool libquixcc::ir::delta::IfElse::print_impl(std::ostream &os,
                                              PState &state) const {
  os << "if (";
  if (!cond->print(os, state)) return false;

  os << ") ";
  if (!then->print(os, state)) return false;

  os << " else ";
  if (!els->print(os, state)) return false;

  return true;
}

bool libquixcc::ir::delta::While::print_impl(std::ostream &os,
                                             PState &state) const {
  os << "while (";
  if (!cond->print(os, state)) return false;

  os << ") ";
  if (!body->print(os, state)) return false;
  return true;
}

bool libquixcc::ir::delta::Jmp::print_impl(std::ostream &os,
                                           PState &state) const {
  os << "jmp " << target;

  return true;
}

bool libquixcc::ir::delta::Label::print_impl(std::ostream &os,
                                             PState &state) const {
  os << name << " ";
  if (!code->print(os, state)) return false;
  return true;
}

bool libquixcc::ir::delta::Ret::print_impl(std::ostream &os,
                                           PState &state) const {
  os << "ret";
  if (value) {
    os << " ";
    if (!value->print(os, state)) return false;
  }

  return true;
}

bool libquixcc::ir::delta::Call::print_impl(std::ostream &os,
                                            PState &state) const {
  os << callee << "(";
  for (size_t i = 0; i < args.size(); i++) {
    if (!args[i]->print(os, state)) return false;

    if (i != args.size() - 1) os << ", ";
  }

  os << ")";

  return true;
}

bool libquixcc::ir::delta::PtrCall::print_impl(std::ostream &os,
                                               PState &state) const {
  throw std::runtime_error("PtrCall::print_impl not implemented");
}

bool libquixcc::ir::delta::Halt::print_impl(std::ostream &os,
                                            PState &state) const {
  os << "halt";
  return true;
}

bool libquixcc::ir::delta::Break::print_impl(
    std::ostream &os, libquixcc::ir::PState &state) const {
  os << "break";
  return true;
}

bool libquixcc::ir::delta::Continue::print_impl(
    std::ostream &os, libquixcc::ir::PState &state) const {
  os << "continue";
  return true;
}

bool libquixcc::ir::delta::Switch::print_impl(
    std::ostream &os, libquixcc::ir::PState &state) const {
  os << "switch (";
  if (!cond->print(os, state)) return false;

  os << ") {";
  for (auto &c : cases) {
    if (!c->print(os, state)) return false;
  }

  if (def) {
    os << "default: ";
    if (!def->print(os, state)) return false;
  }

  os << "}";
  return true;
}

bool libquixcc::ir::delta::Case::print_impl(
    std::ostream &os, libquixcc::ir::PState &state) const {
  os << "case ";
  if (!value->print(os, state)) return false;

  os << ": ";
  if (!code->print(os, state)) return false;

  return true;
}