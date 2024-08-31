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

#include <quix-core/Error.h>

#define __QXIR_IMPL__
#include <quix-qxir/Module.h>

#include <transform/PassGroup.hh>

using namespace qxir::passes;

void PassGroupResult::print(std::ostream &out) const {
  for (const auto &[name, result] : m_results) {
    if (!result) {
      out << "Pass " << name << " failed.\n";
    }
  }
}

thread_local std::unordered_map<PassGroupName, std::shared_ptr<PassGroup>> PassGroup::m_groups;

void PassGroup::register_group(const PassGroupName &name, const std::vector<PassName> &passes,
                               const PassGroupDependencies &dependencies) {
  if (m_groups.contains(name)) {
    auto group = m_groups[name];
    group->m_passes = passes;
    group->m_dependencies = dependencies;
  } else {
    m_groups[name] = std::shared_ptr<PassGroup>(new PassGroup(name, passes, dependencies));
  }
}

const PassGroup *PassGroup::get(PassGroupName name) {
  if (!m_groups.contains(name)) {
    qcore_panicf("Expected pass group %s to exist. But it was not found.", name.c_str());
  }

  return m_groups[name].get();
}

PassGroupName PassGroup::getName() const { return m_name; }

std::vector<PassName> PassGroup::getPasses() const { return m_passes; }

PassGroupDependencies PassGroup::getDependencies() const { return m_dependencies; }

bool PassGroup::hasPass(const PassName &name) const {
  return std::find(m_passes.begin(), m_passes.end(), name) != m_passes.end();
}

bool PassGroup::hasDependency(const PassGroupName &name) const {
  return std::find_if(m_dependencies.begin(), m_dependencies.end(), [&name](const auto &dep) {
           return dep.first == name;
         }) != m_dependencies.end();
}

PassGroupResult PassGroup::transform(qmodule_t *module) const {
  PassGroupResult result;

  for (const auto &dep : m_dependencies) {
    auto depRef = PassGroup::get(dep.first);

    if (dep.second == DependencyFrequency::Once && module->hasPassGroupBeenRun(dep.first)) {
      continue;
    }

    auto res = depRef->transform(module);
    result += res;

    module->applyPassGroupLabel(dep.first);

    if (!res) {
      return result;
    }
  }

  for (const auto &pass : m_passes) {
    auto passRef = Pass::get(pass);
    auto res = passRef->transform(module);
    result.add(pass, res);

    if (!res) {
      break;
    }
  }

  return result;
}
