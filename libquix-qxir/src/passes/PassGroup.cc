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

#include <passes/PassGroup.hh>

std::unordered_map<qxir::passes::PassGroupName, std::shared_ptr<qxir::passes::PassGroup>>
    qxir::passes::PassGroup::m_groups;

const std::weak_ptr<qxir::passes::PassGroup> qxir::passes::PassGroup::create(
    const qxir::passes::PassGroupName &name, const std::vector<qxir::passes::PassName> &passes,
    const PassGroupDependencies &dependencies) {
  if (m_groups.contains(name)) {
    return m_groups[name];
  }

  return m_groups[name] = std::shared_ptr<PassGroup>(new PassGroup(name, passes, dependencies));
}

const std::weak_ptr<qxir::passes::PassGroup> qxir::passes::PassGroup::get(
    qxir::passes::PassGroupName name) {
  if (!m_groups.contains(name)) {
    return std::weak_ptr<PassGroup>();
  }

  return m_groups[name];
}

bool qxir::passes::PassGroup::hasPass(const qxir::passes::PassName &name) const {
  return std::find(m_passes.begin(), m_passes.end(), name) != m_passes.end();
}

bool qxir::passes::PassGroup::hasDependency(const qxir::passes::PassGroupName &name) const {
  return std::find_if(m_dependencies.begin(), m_dependencies.end(), [&name](const auto &dep) {
           return dep.first == name;
         }) != m_dependencies.end();
}

qxir::passes::PassGroupResult qxir::passes::PassGroup::transform(qxir::Module &module) const {
  PassGroupResult result;

  for (const auto &dep : m_dependencies) {
    auto depRef = PassGroup::get(dep.first).lock();
    if (!depRef) {
      qcore_panicf("Expected pass group %s to exist. But a weak_ptr reference was unacquireable.",
                   dep.first.c_str());
    }

    if (dep.second == DependencyFrequency::Once && module.hasPassBeenRun(dep.first)) {
      continue;
    }

    auto res = depRef->transform(module);
    result += res;

    if (!res) {
      return result;
    }
  }

  for (const auto &pass : m_passes) {
    auto passRef = Pass::get(pass).lock();
    if (!passRef) {
      qcore_panicf("Expected pass %s to exist. But a weak_ptr reference was unacquireable.",
                   pass.c_str());
    }

    auto res = passRef->transform(module);
    result |= res;

    if (!res) {
      break;
    }
  }

  return result;
}
