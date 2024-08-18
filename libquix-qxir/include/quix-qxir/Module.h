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

#include <quix-lexer/Lexer.h>
#include <quix-qxir/TypeDecl.h>

#ifdef __cplusplus
extern "C" {
#endif

qlex_t *qxir_get_lexer(qmodule_t *mod);
void qxir_set_lexer(qmodule_t *mod, qlex_t *lexer);
qxir_node_t *qxir_base(qmodule_t *mod);
qxir_conf_t *qxir_get_conf(qmodule_t *mod);
void qxir_set_conf(qmodule_t *mod, qxir_conf_t *conf);
bool qxir_ok(qmodule_t *mod);

#ifdef __cplusplus
}
#endif

#if (defined(__cplusplus) && defined(QXIR_USE_CPP_API)) || defined(__QXIR_IMPL__)

#define QPARSE_USE_CPP_API
#include <quix-core/Arena.h>
#include <quix-parser/Node.h>

#include <cstdint>
#include <diagnostic/Report.hh>
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

    TypeID add(Type *type)  {
      m_types.push_back(type);
      return TypeID(m_types.size() - 1);
    }

    Type *get(TypeID tid)  { return m_types.at(tid.m_id); }
  };

  constexpr size_t MAX_MODULE_INSTANCES = std::numeric_limits<ModuleId>::max();

}  // namespace qxir

class qmodule_t {
public:
  std::unordered_set<std::string> m_passes_applied;
  std::unordered_set<std::string> m_strings;
  std::unique_ptr<qxir::diag::DiagnosticManager> m_diag;
  std::unique_ptr<qxir::TypeManager> m_type_mgr;
  qcore_arena_t m_arena;
  qxir_conf_t *m_conf;
  qlex_t *m_lexer;
  qxir_node_t *m_root;
  qxir::ModuleId m_id;

  qmodule_t(qxir::ModuleId id);
  ~qmodule_t();

  /**
   * @brief Get the module ID.
   * @return ModuleId
   */
  qxir::ModuleId getModuleId() noexcept;

  /**
   * @brief Lookup a type by its type ID.
   * @param tid Type ID
   * @return Type* or nullptr if not found
   */
  qxir::Type *lookupType(qxir::TypeID tid) ;

  /**
   * @brief Set the root node of the module.
   * @param root Root node
   */
  void setRoot(qxir_node_t *root) noexcept;

  /**
   * @brief Get the root node of the module.
   * @return qxir_node_t*
   */
  qxir_node_t *getRoot() noexcept;

  /**
   * @brief Make it known that a pass has been applied to the module.
   * @param label Pass label
   */
  void applyPassLabel(const std::string &label)  ;
  bool hasPassBeenRun(const std::string &label) ;

  std::string_view push_string(std::string_view sv);
};

#define QMODULE_SIZE sizeof(qmodule_t)

namespace qxir {
  qmodule_t *getModule(ModuleId mid) ;
  std::unique_ptr<qmodule_t> createModule() ;
}  // namespace qxir

#endif

#endif  // __QUIX_QXIR_MODULE_H__
