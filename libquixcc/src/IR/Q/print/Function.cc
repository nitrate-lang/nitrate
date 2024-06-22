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

#include <IR/Q/Function.h>
#include <IR/Q/Type.h>

bool libquixcc::ir::q::Block::print_impl(std::ostream &os,
                                         libquixcc::ir::PState &state) const {
  if (stmts.empty()) {
    os << "{}";
    return true;
  }

  os << "{\n";
  state.ind += 2;

  for (auto it = stmts.begin(); it != stmts.end(); it++) {
    os << std::string(state.ind, ' ');

    if (!(*it)->print(os, state)) return false;

    os << ";";

    if (it != stmts.end()) os << "\n";
  }

  state.ind -= 2;
  os << std::string(state.ind, ' ') << "}";

  return true;
}

bool libquixcc::ir::q::Segment::print_impl(std::ostream &os,
                                           libquixcc::ir::PState &state) const {
  os << "segment ";

  if (is_pure)
    os << "pure ";
  else
    os << "impure ";

  if (is_thread_safe) os << "tsafe ";
  if (is_no_throw) os << "noexcept ";
  if (is_no_return) os << "noreturn ";
  if (is_foriegn) os << "foreign ";

  os << "(";
  for (auto it = params.begin(); it != params.end(); it++) {
    os << it->first << ": ";
    if (!it->second->print(os, state)) return false;
    if (it != params.end() - 1)
      os << ", ";
    else if (is_variadic)
      os << ", ...";
  }

  os << ") -> (";
  if (!return_type->print(os, state)) return false;
  os << ")";

  if (!block)
    return true;
  else
    os << " ";

  if (!block->print(os, state)) return false;

  return true;
}

bool libquixcc::ir::q::RootNode::print_impl(
    std::ostream &os, libquixcc::ir::PState &state) const {
  for (auto it = children.begin(); it != children.end(); it++) {
    if (!(*it)->print(os, state)) return false;
    os << ";\n";

    if (state.last_node_type != (*it)->ntype)
      os << "\n";

    state.last_node_type = (*it)->ntype;
  }

  return true;
}