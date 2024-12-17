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

#include <nitrate-core/Macro.h>

#include <core/PassManager.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-ir/Module.hh>

using namespace nr::pass;

CPP_EXPORT PassRegistry& PassRegistry::the() {
  static PassRegistry instance;
  return instance;
}

CPP_EXPORT void PassRegistry::addPass(const std::string& name,
                                      pass_func_t func) {
  std::lock_guard<std::mutex> lock(m_lock);

  m_passes[name] = func;
}

CPP_EXPORT bool PassRegistry::hasPass(const std::string& name) {
  std::lock_guard<std::mutex> lock(m_lock);

  return m_passes.contains(name);
}

CPP_EXPORT ModulePass PassRegistry::get(const std::string& name) {
  PassRegistry& inst = the();

  std::lock_guard<std::mutex> lock(inst.m_lock);

  auto it = inst.m_passes.find(name);
  if (it == inst.m_passes.end()) {
    qcore_panicf("Pass '%s' not registered", name.c_str());
  }

  return ModulePass(it->first, it->second);
}

///==============================================================================

CPP_EXPORT bool PassGroup::run(
    qmodule_t* module, std::function<void(std::string_view)> on_success) {
  for (const auto& pass : m_sequence) {
    if (!pass.run(module, module->getDiag().get())) {
      return false;
    }

    if (on_success) {
      on_success(pass.getName());
    }
  }

  return true;
}

///==============================================================================

CPP_EXPORT PassGroupRegistry& PassGroupRegistry::the() {
  static PassGroupRegistry instance;
  return instance;
}

CPP_EXPORT void PassGroupRegistry::addGroup(
    const std::string& name, std::initializer_list<std::string_view> passes) {
  std::lock_guard<std::mutex> lock(m_lock);

  m_groups[name] = std::vector<ModulePass>();
  for (const auto& pass : passes) {
    m_groups[name].push_back(PassRegistry::get(std::string(pass)));
  }
}

CPP_EXPORT void PassGroupRegistry::addGroup(
    const std::string& name, const std::vector<std::string>& passes) {
  std::lock_guard<std::mutex> lock(m_lock);

  m_groups[name] = std::vector<ModulePass>();
  for (const auto& pass : passes) {
    m_groups[name].push_back(PassRegistry::get(std::string(pass)));
  }
}

CPP_EXPORT bool PassGroupRegistry::hasGroup(const std::string& name) {
  std::lock_guard<std::mutex> lock(m_lock);
  return m_groups.contains(name);
}

CPP_EXPORT PassGroup PassGroupRegistry::get(const std::string& name) {
  PassGroupRegistry& inst = the();

  std::lock_guard<std::mutex> lock(inst.m_lock);

  auto it = inst.m_groups.find(name);
  if (it == inst.m_groups.end()) {
    qcore_panicf("Pass group '%s' not registered", name.c_str());
  }

  return PassGroup(name, it->second);
}

///==============================================================================

CPP_EXPORT void PassGroupBuilder::phase_order() {
  // This is a hard problem. I'll deal with it later
}

CPP_EXPORT bool PassGroupBuilder::verify_state() const {
  // We can't verify because passes don't have a way to specifiy

  // their dependancies and constraints
  return true;
}

CPP_EXPORT PassGroupBuilder& PassGroupBuilder::addPass(
    const std::string& name) {
  m_passes.push_back(name);
  return *this;
}

CPP_EXPORT PassGroupBuilder& PassGroupBuilder::addPassFrom(
    const std::vector<std::string>& list) {
  for (const auto& pass : list) {
    addPass(pass);
  }

  return *this;
}

CPP_EXPORT PassGroupBuilder& PassGroupBuilder::addGroup(
    const std::string& name) {
  for (const auto& pass : PassGroupRegistry::get(name)) {
    addPass(std::string(pass.getName()));
  }

  return *this;
}

CPP_EXPORT PassGroupBuilder& PassGroupBuilder::addGroupFrom(
    const std::vector<std::string>& list) {
  for (auto group_name : list) {
    for (const auto& pass : PassGroupRegistry::get(group_name)) {
      addPass(std::string(pass.getName()));
    }
  }
  return *this;
}

CPP_EXPORT bool PassGroupBuilder::verify() {
  for (const auto& pass : m_passes) {
    if (!PassRegistry::the().hasPass(pass)) {
      return false;
    }
  }

  bool ok = verify_state();
  m_verified = {m_passes.size(), ok};

  return ok;
}

CPP_EXPORT PassGroup PassGroupBuilder::build(const std::string& name,
                                             bool optimize_order) {
  if (optimize_order) {
    phase_order();
  }

  if (!m_verified.second || m_verified.first != m_passes.size()) {
    if (!verify()) {
      qcore_panic("Pass group builder failed verification");
    }
  }

  PassGroupRegistry::the().addGroup(name, m_passes);

  return PassGroupRegistry::get(name);
}
