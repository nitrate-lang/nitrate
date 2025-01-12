////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///     .-----------------.    .----------------.     .----------------.     ///
///    | .--------------. |   | .--------------. |   | .--------------. |    ///
///    | | ____  _____  | |   | |     ____     | |   | |    ______    | |    ///
///    | ||_   _|_   _| | |   | |   .'    `.   | |   | |   / ____ `.  | |    ///
///    | |  |   \ | |   | |   | |  /  .--.  \  | |   | |   `'  __) |  | |    ///
///    | |  | |\ \| |   | |   | |  | |    | |  | |   | |   _  |__ '.  | |    ///
///    | | _| |_\   |_  | |   | |  \  `--'  /  | |   | |  | \____) |  | |    ///
///    | ||_____|\____| | |   | |   `.____.'   | |   | |   \______.'  | |    ///
///    | |              | |   | |              | |   | |              | |    ///
///    | '--------------' |   | '--------------' |   | '--------------' |    ///
///     '----------------'     '----------------'     '----------------'     ///
///                                                                          ///
///   * NITRATE TOOLCHAIN - The official toolchain for the Nitrate language. ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The Nitrate Toolchain is free software; you can redistribute it or     ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The Nitrate Toolcain is distributed in the hope that it will be        ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the Nitrate Toolchain; if not, see                  ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#include <memory>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-ir/IR/Nodes.hh>
#include <nitrate-ir/Module.hh>

using namespace ncc;
using namespace ncc::ir;

NCC_EXPORT IRModule::IRModule(string module_name) {
  m_module_name = module_name;

  m_root = nullptr;
  m_diagnostics_enabled = true;

  m_ir_data = std::make_unique<ncc::dyn_arena>();
}

NCC_EXPORT IRModule::~IRModule() { m_root = nullptr; }

NCC_EXPORT bool IRModule::Diagnostics(std::optional<bool> state) {
  bool old_state = m_diagnostics_enabled;

  if (state.has_value()) {
    m_diagnostics_enabled = state.value();
  }

  return old_state;
}

NCC_EXPORT string IRModule::Name(std::optional<string> name) {
  string old_name = m_module_name;

  if (name.has_value()) {
    m_module_name = name.value();
  }

  return old_name;
}

NCC_EXPORT void IRModule::accept(IRVisitor<void> &visitor) {
  m_root.value()->accept(visitor);
}
