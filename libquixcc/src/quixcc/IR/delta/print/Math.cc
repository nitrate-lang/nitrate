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

bool libquixcc::ir::delta::Add::print_impl(std::ostream &os,
                                           PState &state) const {
  os << "(";
  if (!lhs->print(os, state)) return false;
  os << "+";
  if (!rhs->print(os, state)) return false;
  os << ")";
  return true;
}

bool libquixcc::ir::delta::Sub::print_impl(std::ostream &os,
                                           PState &state) const {
  os << "(";
  if (!lhs->print(os, state)) return false;
  os << "-";
  if (!rhs->print(os, state)) return false;
  os << ")";
  return true;
}

bool libquixcc::ir::delta::Mul::print_impl(std::ostream &os,
                                           PState &state) const {
  os << "(";
  if (!lhs->print(os, state)) return false;
  os << "*";
  if (!rhs->print(os, state)) return false;
  os << ")";
  return true;
}

bool libquixcc::ir::delta::Div::print_impl(std::ostream &os,
                                           PState &state) const {
  os << "(";
  if (!lhs->print(os, state)) return false;
  os << "/";
  if (!rhs->print(os, state)) return false;
  os << ")";
  return true;
}

bool libquixcc::ir::delta::Mod::print_impl(std::ostream &os,
                                           PState &state) const {
  os << "(";
  if (!lhs->print(os, state)) return false;
  os << "%";
  if (!rhs->print(os, state)) return false;
  os << ")";
  return true;
}

bool libquixcc::ir::delta::BitAnd::print_impl(std::ostream &os,
                                              PState &state) const {
  os << "(";
  if (!lhs->print(os, state)) return false;
  os << "&";
  if (!rhs->print(os, state)) return false;
  os << ")";
  return true;
}

bool libquixcc::ir::delta::BitOr::print_impl(std::ostream &os,
                                             PState &state) const {
  os << "(";
  if (!lhs->print(os, state)) return false;
  os << "|";
  if (!rhs->print(os, state)) return false;
  os << ")";
  return true;
}

bool libquixcc::ir::delta::BitXor::print_impl(std::ostream &os,
                                              PState &state) const {
  os << "(";
  if (!lhs->print(os, state)) return false;
  os << "^";
  if (!rhs->print(os, state)) return false;
  os << ")";
  return true;
}

bool libquixcc::ir::delta::BitNot::print_impl(std::ostream &os,
                                              PState &state) const {
  os << "(~";
  if (!operand->print(os, state)) return false;
  os << ")";
  return true;
}

bool libquixcc::ir::delta::Shl::print_impl(std::ostream &os,
                                           PState &state) const {
  os << "(";
  if (!lhs->print(os, state)) return false;
  os << "<<";
  if (!rhs->print(os, state)) return false;
  os << ")";
  return true;
}

bool libquixcc::ir::delta::Shr::print_impl(std::ostream &os,
                                           PState &state) const {
  os << "(";
  if (!lhs->print(os, state)) return false;
  os << ">>";
  if (!rhs->print(os, state)) return false;
  os << ")";
  return true;
}

bool libquixcc::ir::delta::Rotl::print_impl(std::ostream &os,
                                            PState &state) const {
  os << "(";
  if (!lhs->print(os, state)) return false;
  os << "<<<";
  if (!rhs->print(os, state)) return false;
  os << ")";
  return true;
}

bool libquixcc::ir::delta::Rotr::print_impl(std::ostream &os,
                                            PState &state) const {
  os << "(";
  if (!lhs->print(os, state)) return false;
  os << ">>>";
  if (!rhs->print(os, state)) return false;
  os << ")";
  return true;
}

bool libquixcc::ir::delta::Eq::print_impl(std::ostream &os,
                                          PState &state) const {
  os << "(";
  if (!lhs->print(os, state)) return false;
  os << "==";
  if (!rhs->print(os, state)) return false;
  os << ")";
  return true;
}

bool libquixcc::ir::delta::Ne::print_impl(std::ostream &os,
                                          PState &state) const {
  os << "(";
  if (!lhs->print(os, state)) return false;
  os << "!=";
  if (!rhs->print(os, state)) return false;
  os << ")";
  return true;
}

bool libquixcc::ir::delta::Lt::print_impl(std::ostream &os,
                                          PState &state) const {
  os << "(";
  if (!lhs->print(os, state)) return false;
  os << "<";
  if (!rhs->print(os, state)) return false;
  os << ")";
  return true;
}

bool libquixcc::ir::delta::Gt::print_impl(std::ostream &os,
                                          PState &state) const {
  os << "(";
  if (!lhs->print(os, state)) return false;
  os << ">";
  if (!rhs->print(os, state)) return false;
  os << ")";
  return true;
}

bool libquixcc::ir::delta::Le::print_impl(std::ostream &os,
                                          PState &state) const {
  os << "(";
  if (!lhs->print(os, state)) return false;
  os << "<=";
  if (!rhs->print(os, state)) return false;
  os << ")";
  return true;
}

bool libquixcc::ir::delta::Ge::print_impl(std::ostream &os,
                                          PState &state) const {
  os << "(";
  if (!lhs->print(os, state)) return false;
  os << ">=";
  if (!rhs->print(os, state)) return false;
  os << ")";
  return true;
}

bool libquixcc::ir::delta::And::print_impl(std::ostream &os,
                                           PState &state) const {
  os << "(";
  if (!lhs->print(os, state)) return false;
  os << "&&";
  if (!rhs->print(os, state)) return false;
  os << ")";
  return true;
}

bool libquixcc::ir::delta::Or::print_impl(std::ostream &os,
                                          PState &state) const {
  os << "(";
  if (!lhs->print(os, state)) return false;
  os << "||";
  if (!rhs->print(os, state)) return false;
  os << ")";
  return true;
}

bool libquixcc::ir::delta::Not::print_impl(std::ostream &os,
                                           PState &state) const {
  os << "(!";
  if (!operand->print(os, state)) return false;
  os << ")";
  return true;
}

bool libquixcc::ir::delta::Xor::print_impl(std::ostream &os,
                                           PState &state) const {
  os << "(";
  if (!lhs->print(os, state)) return false;
  os << "^^";
  if (!rhs->print(os, state)) return false;
  os << ")";
  return true;
}