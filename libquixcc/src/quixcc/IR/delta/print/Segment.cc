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

#include <quixcc/IR/delta/Segment.h>

bool libquixcc::ir::delta::Block::print_impl(std::ostream &os, libquixcc::ir::PState &state) const {
  if (stmts.empty()) {
    os << "{}";
    return true;
  }

  os << "{\n";
  state.ind += 2;

  for (auto it = stmts.begin(); it != stmts.end(); it++) {
    os << std::string(state.ind, ' ');

    if (!(*it)->print(os, state))
      return false;
    os << ";\n";

    if (it < stmts.end() - 1) {
      int tid = (*it)->ntype;
      int next_node_type = (*std::next(it))->ntype;
      if (next_node_type != tid)
        os << "\n";
    }
  }

  state.ind -= 2;
  os << std::string(state.ind, ' ') << "}";

  return true;
}

bool libquixcc::ir::delta::Segment::print_impl(std::ostream &os, PState &state) const {
  os << "segment ";

  os << "(";
  for (auto it = params.begin(); it != params.end(); it++) {
    os << it->first << ": ";
    if (!it->second->print(os, state))
      return false;
    if (it != params.end() - 1)
      os << ", ";
    else if (variadic)
      os << ", ...";
  }

  os << ") -> (";
  if (!ret->print(os, state))
    return false;
  os << ")";

  if (!block)
    return true;
  else
    os << " ";

  if (!block->print(os, state))
    return false;

  return true;
}

bool libquixcc::ir::delta::Asm::print_impl(std::ostream &os, libquixcc::ir::PState &state) const {
  os << "asm(" << asm_str << ", {";
  for (uint64_t i = 0; i < outputs.size(); i++) {
    os << outputs[i].first << ": ";
    if (!outputs[i].second->print(os, state))
      return false;
    if (i + 1 < outputs.size())
      os << ", ";
  }

  os << "}, {";
  for (uint64_t i = 0; i < inputs.size(); i++) {
    os << inputs[i].first << ": ";
    if (!inputs[i].second->print(os, state))
      return false;
    if (i + 1 < inputs.size())
      os << ", ";
  }

  os << "}, [";
  for (uint64_t i = 0; i < clobbers.size(); i++) {
    os << clobbers[i];
    if (i + 1 < clobbers.size())
      os << ", ";
  }

  os << "])";
  return true;
}

bool libquixcc::ir::delta::RootNode::print_impl(std::ostream &os, PState &state) const {
  for (auto it = children.begin(); it != children.end(); it++) {
    if (!(*it)->print(os, state))
      return false;
    os << ";\n";

    if (it < children.end() - 1) {
      int tid = (*it)->ntype;
      int next_node_type = (*std::next(it))->ntype;
      if (next_node_type != tid)
        os << "\n";
    }
  }

  return true;
}