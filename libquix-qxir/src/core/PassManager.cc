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

#include <core/LibMacro.h>
#include <quix-core/Error.h>

#include <core/PassManager.hh>

using namespace qxir::pass;

CPP_EXPORT PassRegistry& PassRegistry::the() {
  static PassRegistry instance;
  return instance;
}

CPP_EXPORT void PassRegistry::addPass(const std::string& name, pass_func_t func) {
  if (m_passes.contains(name)) {
    qcore_panicf("Pass '%s' already registered to function @ %p", name.c_str(),
                 m_passes[name].target<void>());
  }

  m_passes[name] = func;
}

CPP_EXPORT bool PassRegistry::hasPass(const std::string& name) const {
  return m_passes.contains(name);
}

CPP_EXPORT ModulePass PassRegistry::get(const std::string& name) {
  PassRegistry& inst = the();

  auto it = inst.m_passes.find(name);
  if (it == inst.m_passes.end()) {
    qcore_panicf("Pass '%s' not registered", name.c_str());
  }

  return ModulePass(it->first, it->second);
}

///==============================================================================

CPP_EXPORT bool PassGroup::run(qmodule_t* module,
                               std::function<void(std::string_view)> on_success) {
  for (const auto& pass : m_sequence) {
    if (!pass.run(module)) {
      return false;
    }

    if (on_success) {
      on_success(pass.getName());
    }
  }

  return true;
}

///==============================================================================

PassGroupRegistry& PassGroupRegistry::the() {
  static PassGroupRegistry instance;
  return instance;
}

CPP_EXPORT void PassGroupRegistry::addGroup(const std::string& name,
                                            std::initializer_list<std::string_view> passes) {
  if (m_groups.contains(name)) {
    qcore_panicf("Pass group '%s' already registered", name.c_str());
  }

  m_groups[name] = std::vector<ModulePass>();
  for (const auto& pass : passes) {
    m_groups[name].push_back(PassRegistry::get(std::string(pass)));
  }
}

CPP_EXPORT bool PassGroupRegistry::hasGroup(const std::string& name) const {
  return m_groups.contains(name);
}

CPP_EXPORT PassGroup PassGroupRegistry::get(const std::string& name) {
  PassGroupRegistry& inst = the();

  auto it = inst.m_groups.find(name);
  if (it == inst.m_groups.end()) {
    qcore_panicf("Pass group '%s' not registered", name.c_str());
  }

  return PassGroup(name, it->second);
}

///==============================================================================
