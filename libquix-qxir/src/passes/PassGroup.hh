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

#ifndef __QUIX_QXIR_PASSES_PASSGROUP_H__
#define __QUIX_QXIR_PASSES_PASSGROUP_H__

#include <memory>
#include <passes/Pass.hh>
#include <unordered_map>
#include <vector>

namespace qxir::passes {
  typedef std::string PassGroupName;

  class PassGroupResult final {
    std::vector<PassResult> m_results;

  public:
    PassGroupResult() = default;

    void operator|=(const PassResult& result) { m_results.push_back(result); }
    void operator+=(const PassGroupResult& result) {
      m_results.insert(m_results.end(), result.m_results.begin(), result.m_results.end());
    }

    bool operator!() const {
      return std::find_if(m_results.begin(), m_results.end(),
                          [](const PassResult& result) { return !result; }) != m_results.end();
    }
  };

  enum class DependencyFrequency { Always, Once };

  typedef std::vector<std::pair<PassGroupName, DependencyFrequency>> PassGroupDependencies;

  class PassGroup final {
    PassGroupName m_name;
    std::vector<PassName> m_passes;
    PassGroupDependencies m_dependencies;

    static std::unordered_map<PassGroupName, std::shared_ptr<PassGroup>> m_groups;

    PassGroup(PassGroupName name, std::vector<PassName> passes, PassGroupDependencies dependencies)
        : m_name(std::move(name)),
          m_passes(std::move(passes)),
          m_dependencies(std::move(dependencies)) {}

  public:
    static const std::weak_ptr<PassGroup> create(const PassGroupName& name,
                                                 const std::vector<PassName>& passes,
                                                 const PassGroupDependencies& dependencies);

    static const std::weak_ptr<PassGroup> get(PassGroupName name);

    const PassGroupName& getName() const { return m_name; }
    const std::vector<PassName>& getPasses() const { return m_passes; }
    const PassGroupDependencies& getDependencies() const { return m_dependencies; }
    bool hasPass(const PassName& name) const;
    bool hasDependency(const PassGroupName& name) const;

    PassGroupResult transform(Module& module) const;
  };
}  // namespace qxir::passes

#endif  // __QUIX_QXIR_PASSES_PASSGROUP_H__
