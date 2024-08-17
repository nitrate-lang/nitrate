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

#ifndef __QUIX_QXIR_MODULE_H__
#define __QUIX_QXIR_MODULE_H__

#include <quix-core/Arena.h>

#if (defined(__cplusplus) && defined(QXIR_USE_CPP_API)) || defined(__QXIR_IMPL__)

extern "C" {
typedef struct qxir_node_t qxir_node_t;
}

#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

namespace qxir {
  typedef uint16_t ModuleId;

  struct TypeID {
    uint64_t m_id : 40;

    TypeID(uint64_t id) : m_id(id) {}
  } __attribute__((packed));

  class Type;

  class TypeManager {
    std::vector<Type *> m_types;

  public:
    TypeManager() = default;

    TypeID add(Type *type) noexcept {
      m_types.push_back(type);
      return TypeID(m_types.size() - 1);
    }

    Type *get(TypeID tid) noexcept { return m_types.at(tid.m_id); }
  };

  class Module final {
    ModuleId m_id;
    TypeManager m_type_mgr;
    std::optional<qcore_arena_t> m_arena;
    std::unordered_set<std::string> m_passes_applied;
    qxir_node_t *m_root;

  public:
    Module(ModuleId id);
    ~Module();

    /**
     * @brief Get the module ID.
     * @return ModuleId
     */
    ModuleId getModuleId() noexcept { return m_id; }

    /**
     * @brief Lookup a type by its type ID.
     * @param tid Type ID
     * @return Type* or nullptr if not found
     */
    Type *lookupType(TypeID tid) noexcept { return m_type_mgr.get(tid); }

    /**
     * @brief Save a type to the module's type manager.
     * @param tid Type ID
     * @param type Type
     */
    void mapType(TypeID tid, Type *type) noexcept { m_type_mgr.add(type); }

    /**
     * @brief Set the root node of the module.
     * @param root Root node
     */
    void setRoot(qxir_node_t *root) noexcept { m_root = root; }

    /**
     * @brief Get the root node of the module.
     * @return qxir_node_t*
     */
    qxir_node_t *getRoot() noexcept { return m_root; }

    /**
     * @brief Make it known that a pass has been applied to the module.
     * @param label Pass label
     */
    void applyPassLabel(const std::string &label) noexcept { m_passes_applied.insert(label); }
    bool hasPassBeenRun(const std::string &label) noexcept { return m_passes_applied.count(label); }
  };

  std::unique_ptr<Module> createModule() noexcept;
  Module *getModule(ModuleId mid) noexcept;

  constexpr size_t MAX_MODULE_INSTANCES = std::numeric_limits<ModuleId>::max();

}  // namespace qxir

#endif

#endif  // __QUIX_QXIR_MODULE_H__
