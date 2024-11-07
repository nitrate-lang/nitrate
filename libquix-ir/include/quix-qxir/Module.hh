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

#include <quix-core/Memory.h>
#include <quix-lexer/Lexer.h>
#include <quix-qxir/TypeDecl.h>

#include <boost/bimap.hpp>
#include <cstdint>
#include <limits>
#include <memory>
#include <quix-core/Classes.hh>
#include <quix-qxir/Report.hh>
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

}  // namespace qxir

namespace qxir {
  class Expr;
  class Type;
  class BinExpr;
  class UnExpr;
  class PostUnExpr;
  class U1Ty;
  class U8Ty;
  class U16Ty;
  class U32Ty;
  class U64Ty;
  class U128Ty;
  class I8Ty;
  class I16Ty;
  class I32Ty;
  class I64Ty;
  class I128Ty;
  class F16Ty;
  class F32Ty;
  class F64Ty;
  class F128Ty;
  class VoidTy;
  class PtrTy;
  class OpaqueTy;
  class StructTy;
  class UnionTy;
  class ArrayTy;
  class FnTy;
  class Int;
  class Float;
  class List;
  class Call;
  class Seq;
  class Index;
  class Ident;
  class Extern;
  class Local;
  class Ret;
  class Brk;
  class Cont;
  class If;
  class While;
  class For;
  class Form;
  class Case;
  class Switch;
  class Fn;
  class Asm;
  class Tmp;
}  // namespace qxir

struct qmodule_t final {
private:
  using FunctionNameBimap = boost::bimap<std::string_view, std::pair<qxir::FnTy *, qxir::Fn *>>;
  using GlobalVariableNameBimap = boost::bimap<std::string_view, qxir::Local *>;
  using FunctionParamMap =
      std::unordered_map<std::string_view,
                         std::vector<std::tuple<std::string, qxir::Type *, qxir::Expr *>>>;
  using TypenameMap = std::unordered_map<std::string_view, qxir::Type *>;
  using CompositeFieldMap =
      std::unordered_map<std::string_view,
                         std::vector<std::tuple<std::string, qxir::Type *, qxir::Expr *>>>;
  using NamedConstMap = std::unordered_map<std::string_view, qxir::Expr *>;

  ///=============================================================================
  qxir::Expr *m_root;                               /* Root node of the module */
  std::unordered_map<uint64_t, uint64_t> m_key_map; /* Place for IRGraph key-value pairs */
  ///=============================================================================

  ///=============================================================================
  /// BEGIN: Data structures requisite for efficient lowering
  FunctionNameBimap functions;          /* Lookup for function names to their nodes */
  GlobalVariableNameBimap variables;    /* Lookup for global variables names to their nodes */
  FunctionParamMap m_parameters;        /* Lookup for function parameters */
  TypenameMap m_typedef_map;            /* Lookup type names to their type nodes */
  CompositeFieldMap m_composite_fields; /* */
  NamedConstMap m_named_constants;      /* Lookup for named constants */
  bool m_failbit;                       /* Set if module lowering fails */

  void reset_module_temporaries(void) {
    functions.clear(), variables.clear(), m_parameters.clear();
    m_typedef_map.clear(), m_composite_fields.clear(), m_named_constants.clear();
    m_failbit = false;
  }
  /// END: Data structures requisite for efficient lowering
  ///=============================================================================

  std::unique_ptr<qxir::diag::DiagnosticManager> m_diag; /* Diagnostic manager instance */
  std::unique_ptr<qxir::TypeManager> m_type_mgr;         /* Type manager instance */
  std::unordered_set<std::string> m_strings;             /* Interned strings */
  std::vector<std::string> m_passes_applied;             /* Module mutation tracking */
  std::vector<std::string> m_checks_applied;             /* Module analysis pass tracking */
  qxir::TargetInfo m_target_info;                        /* Build target information */
  std::string m_module_name;                             /* Not nessesarily unique module name */
  qxir::ModuleId m_id;                                   /* Module ID unique to the
                                                            process during its lifetime */
  bool m_diagnostics_enabled;

  qcore_arena m_node_arena;
  qxir_conf_t *m_conf;
  qlex_t *m_lexer;

public:
  qmodule_t(qxir::ModuleId id, const std::string &name = "?");
  ~qmodule_t();

  qxir::ModuleId getModuleId() noexcept { return m_id; }

  qxir::Type *lookupType(qxir::TypeID tid);

  void setRoot(qxir::Expr *root) noexcept { m_root = root; }
  qxir::Expr *&getRoot() noexcept { return m_root; }

  void setLexer(qlex_t *lexer) noexcept { m_lexer = lexer; }
  qlex_t *getLexer() noexcept { return m_lexer; }

  void setConf(qxir_conf_t *conf) noexcept { m_conf = conf; }
  qxir_conf_t *getConf() noexcept { return m_conf; }

  std::unordered_map<uint64_t, uint64_t> &getKeyMap() noexcept { return m_key_map; }

  void enableDiagnostics(bool is_enabled) noexcept;
  bool isDiagnosticsEnabled() const noexcept { return m_diagnostics_enabled; }

  void applyPassLabel(const std::string &label) { m_passes_applied.push_back(label); }
  const auto &getPassesApplied() const { return m_passes_applied; }
  void applyCheckLabel(const std::string &label) { m_checks_applied.push_back(label); }
  const auto &getChecksApplied() const { return m_checks_applied; }

  bool hasPassBeenRun(const std::string &label) {
    return std::find(m_passes_applied.begin(), m_passes_applied.end(), label) !=
           m_passes_applied.end();
  }

  const std::string getName() const { return m_module_name; }
  void setName(const std::string &name) { m_module_name = name; }

  auto &getFunctions() { return functions; }
  auto &getGlobalVariables() { return variables; }
  auto &getParameterMap() { return m_parameters; }
  auto &getTypeMap() { return m_typedef_map; }
  auto &getCompositeFields() { return m_composite_fields; }
  auto &getNamedConstants() { return m_named_constants; }

  std::string_view internString(std::string_view sv);

  qcore_arena_t &getNodeArena() { return *m_node_arena.get(); }

  qxir::diag::DiagnosticManager &getDiag() { return *m_diag; }

  const qxir::TargetInfo &getTargetInfo() const { return m_target_info; }

  void setFailbit(bool fail) { m_failbit = fail; }
  bool getFailbit() const { return m_failbit; }
};

constexpr size_t QMODULE_SIZE = sizeof(qmodule_t);

namespace qxir {
  qmodule_t *getModule(ModuleId mid);
  qmodule_t *createModule(std::string name = "?");
}  // namespace qxir

#endif
