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

#ifndef __NITRATE_NR_MODULE_H__
#define __NITRATE_NR_MODULE_H__

#include <nitrate-ir/TypeDecl.h>

#include <boost/bimap.hpp>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <nitrate-core/Allocate.hh>
#include <nitrate-ir/Report.hh>
#include <nitrate-ir/Visitor.hh>
#include <string>
#include <vector>

namespace nr {
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

    TypeID add(Type *type) {
      m_types.push_back(type);
      return TypeID(m_types.size() - 1);
    }

    Type *get(TypeID tid) { return m_types.at(tid.m_id); }
  };

  constexpr size_t MAX_MODULE_INSTANCES = std::numeric_limits<ModuleId>::max();

  struct TargetInfo {
    uint16_t PointerSizeBytes = 8;
    std::optional<std::string> TargetTriple, CPU, CPUFeatures;
  };

  class Expr;

  enum class ModulePassType {
    Transform,
    Check,
  };

  class NRBuilder;
}  // namespace nr

struct qmodule_t final {
private:
  friend class nr::Expr;
  friend class nr::NRBuilder;

  using FunctionNameBimap =
      boost::bimap<std::string_view, std::pair<nr::FnTy *, nr::Fn *>>;
  using GlobalVariableNameBimap = boost::bimap<std::string_view, nr::Local *>;
  using FunctionParamMap = std::unordered_map<
      std::string_view,
      std::vector<std::tuple<std::string, nr::Type *, nr::Expr *>>>;
  using TypenameMap = std::unordered_map<std::string_view, nr::Type *>;
  using StructFieldMap = std::unordered_map<
      std::string_view,
      std::vector<std::tuple<std::string, nr::Type *, nr::Expr *>>>;
  using NamedConstMap = std::unordered_map<std::string_view, nr::Expr *>;
  using ModulePasses = std::vector<std::pair<std::string, nr::ModulePassType>>;

  ///=============================================================================
  nr::Expr *m_root{}; /* Root node of the module */
  std::unordered_map<uint64_t, uint64_t>
      m_key_map{}; /* Place for IRGraph key-value pairs */
  uint64_t m_extension_data_ctr = 1;

  ///=============================================================================

  ///=============================================================================
  /// BEGIN: Data structures requisite for efficient lowering
  FunctionNameBimap functions{}; /* Lookup for function names to their nodes */
  GlobalVariableNameBimap
      variables{}; /* Lookup for global variables names to their nodes */
  FunctionParamMap m_parameters{}; /* Lookup for function parameters */
  TypenameMap m_typedef_map{};     /* Lookup type names to their type nodes */
  StructFieldMap m_composite_fields{}; /* */
  NamedConstMap m_named_constants{};   /* Lookup for named constants */

  void reset_module_temporaries(void) {
    functions.clear(), variables.clear(), m_parameters.clear();
    m_typedef_map.clear(), m_composite_fields.clear(),
        m_named_constants.clear();
  }
  /// END: Data structures requisite for efficient lowering
  ///=============================================================================

  std::unique_ptr<nr::IReport> m_diagnostics;
  std::unique_ptr<nr::ISourceView> m_offset_resolver;
  ModulePasses m_applied{};       /* Module pass tracking */
  nr::TargetInfo m_target_info{}; /* Build target information */
  std::string m_module_name{};    /* Not nessesarily unique module name */
  nr::ModuleId m_id{};            /* Module ID unique to the
                                     process during its lifetime */
  bool m_diagnostics_enabled{};

  qcore_arena m_node_arena{};

public:
  qmodule_t(nr::ModuleId id, const std::string &name = "?");
  ~qmodule_t();

  nr::ModuleId getModuleId() { return m_id; }

  void setRoot(nr::Expr *root) { m_root = root; }
  nr::Expr *&getRoot() { return m_root; }
  nr::Expr *getRoot() const { return m_root; }

  std::unordered_map<uint64_t, uint64_t> &getKeyMap() { return m_key_map; }

  void enableDiagnostics(bool is_enabled);
  bool isDiagnosticsEnabled() const { return m_diagnostics_enabled; }

  const auto &getPassesApplied() const { return m_applied; }
  void applyPassLabel(const std::string &label, nr::ModulePassType type) {
    m_applied.push_back({label, type});
  }

  const std::string getName() const { return m_module_name; }
  void setName(const std::string &name) { m_module_name = name; }

  auto &getFunctions() { return functions; }
  auto &getGlobalVariables() { return variables; }
  auto &getParameterMap() { return m_parameters; }
  auto &getTypeMap() { return m_typedef_map; }
  auto &getStructFields() { return m_composite_fields; }
  auto &getNamedConstants() { return m_named_constants; }

  qcore_arena_t &getNodeArena() { return *m_node_arena.get(); }

  std::unique_ptr<nr::IReport> &getDiag() { return m_diagnostics; }
  std::unique_ptr<nr::ISourceView> &getOffsetResolver() {
    return m_offset_resolver;
  }

  const nr::TargetInfo &getTargetInfo() const { return m_target_info; }

  void accept(nr::NRVisitor &visitor);
};

constexpr size_t QMODULE_SIZE = sizeof(qmodule_t);

namespace nr {
  qmodule_t *getModule(ModuleId mid);
  qmodule_t *createModule(std::string name = "?");
}  // namespace nr

#endif
