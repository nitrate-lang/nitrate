#include <unordered_map>
#////////////////////////////////////////////////////////////////////////////////
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

#ifndef __NITRATE_QXIR_DIAGNOSE_PASSES_AUTO_REGISTER_H__
#define __NITRATE_QXIR_DIAGNOSE_PASSES_AUTO_REGISTER_H__

#include <atomic>
#include <functional>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

struct qmodule_t;

namespace qxir::pass {
  typedef std::function<bool(qmodule_t*)> pass_func_t;

  class ModulePass {
    std::string_view name;
    pass_func_t func;

  public:
    ModulePass(std::string_view name, pass_func_t func) : name(name), func(func) {}

    bool run(qmodule_t* module) const { return func(module); }

    std::string_view getName() const { return name; }
    pass_func_t getFunc() const { return func; }
  };

  class PassRegistry final {
    std::unordered_map<std::string, pass_func_t> m_passes;
    std::mutex m_lock;

    void link_builtin();
    PassRegistry() { link_builtin(); }

  public:
    static PassRegistry& the();

    void addPass(const std::string& name, pass_func_t func);
    bool hasPass(const std::string& name);
    static ModulePass get(const std::string& name);

    auto getPasses() {
      std::lock_guard<std::mutex> lock(m_lock);
      return m_passes;
    }
  };

  class PassGroup final {
    std::string_view m_name;
    std::span<const ModulePass> m_sequence;

  public:
    PassGroup() = default;
    PassGroup(std::string_view name, std::span<ModulePass> passes)
        : m_name(name), m_sequence(passes) {}

    bool run(qmodule_t* module, std::function<void(std::string_view name)> on_success = nullptr);

    std::string_view getName() const { return m_name; }

    auto begin() const { return m_sequence.begin(); }
    auto end() const { return m_sequence.end(); }
  };

  class PassGroupRegistry final {
    std::unordered_map<std::string, std::vector<ModulePass>> m_groups;
    std::mutex m_lock;

    PassGroupRegistry() {
      PassRegistry::the();  // Ensure the pass registry is already initialized
    }

  public:
    static PassGroupRegistry& the();

    void addGroup(const std::string& name, std::initializer_list<std::string_view> passes);
    void addGroup(const std::string& name, const std::vector<std::string>& passes);
    bool hasGroup(const std::string& name);
    static PassGroup get(const std::string& name);

    auto getGroups() {
      std::lock_guard<std::mutex> lock(m_lock);
      return m_groups;
    }

    static void RegisterBuiltinGroups();
  };

  class PassGroupBuilder final {
    std::vector<std::string> m_passes;
    std::pair<size_t, bool> m_verified;

    void phase_order();
    bool verify_state() const;

  public:
    PassGroupBuilder() = default;

    PassGroupBuilder& addPass(const std::string& name);
    PassGroupBuilder& addPassFrom(const std::vector<std::string>& list);
    PassGroupBuilder& addGroup(const std::string& name);
    PassGroupBuilder& addGroupFrom(const std::vector<std::string>& list);

    bool verify();

    PassGroup build(const std::string& name, bool optimize_order = true);
  };
}  // namespace qxir::pass

#endif  // __NITRATE_QXIR_DIAGNOSE_PASSES_AUTO_REGISTER_H__
