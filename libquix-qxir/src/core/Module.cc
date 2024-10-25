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

#include <mutex>
#include <quix-qxir/Module.hh>

using namespace qxir;

static std::vector<std::optional<qmodule_t *>> qxir_modules;
static std::mutex qxir_modules_mutex;

qmodule_t::qmodule_t(ModuleId id, const std::string &name) {
  m_passes_applied.clear();
  m_strings.clear();
  m_diag = std::make_unique<diag::DiagnosticManager>();
  m_diag->set_ctx(this);
  m_type_mgr = std::make_unique<TypeManager>();
  m_module_name = name;

  m_conf = nullptr;
  m_lexer = nullptr;
  m_root = nullptr;
  m_diagnostics_enabled = true;
  m_failbit = false;

  m_id = id;
}

qmodule_t::~qmodule_t() {
  m_conf = nullptr;
  m_lexer = nullptr;
  m_root = nullptr;
}

Type *qmodule_t::lookupType(TypeID tid) { return m_type_mgr->get(tid); }

void qmodule_t::enableDiagnostics(bool is_enabled) noexcept { m_diagnostics_enabled = is_enabled; }

std::string_view qmodule_t::internString(std::string_view sv) {
  for (const auto &str : m_strings) {
    if (str == sv) {
      return str;
    }
  }

  return m_strings.insert(std::string(sv)).first->c_str();
}

///=============================================================================

qmodule_t *qxir::createModule(std::string name) {
  std::lock_guard<std::mutex> lock(qxir_modules_mutex);

  ModuleId mid;

  for (mid = 0; mid < qxir_modules.size(); mid++) {
    if (!qxir_modules[mid].has_value()) {
      break;
    }
  }

  if (mid >= MAX_MODULE_INSTANCES) {
    return nullptr;
  }

  qxir_modules.insert(qxir_modules.begin() + mid, new qmodule_t(mid, name));

  return qxir_modules[mid].value();
}

CPP_EXPORT qmodule_t *qxir::getModule(ModuleId mid) {
  std::lock_guard<std::mutex> lock(qxir_modules_mutex);

  if (mid >= qxir_modules.size() || !qxir_modules.at(mid).has_value()) {
    return nullptr;
  }

  return qxir_modules.at(mid).value();
}

LIB_EXPORT qmodule_t *qxir_new(qlex_t *lexer, qxir_conf_t *conf, const char *name) {
  try {
    if (!conf) {
      return nullptr;
    }

    if (!name) {
      name = "module";
    }

    qmodule_t *obj = createModule(name);
    if (!obj) {
      return nullptr;
    }

    obj->setConf(conf);
    obj->setLexer(lexer);

    return obj;
  } catch (...) {
    return nullptr;
  }
}

LIB_EXPORT void qxir_free(qmodule_t *mod) {
  try {
    if (!mod) {
      return;
    }

    std::lock_guard<std::mutex> lock(qxir_modules_mutex);

    auto mid = mod->getModuleId();
    delete mod;
    qxir_modules.at(mid).reset();
  } catch (...) {
    qcore_panic("qxir_free failed");
  }
}

LIB_EXPORT size_t qxir_max_modules(void) { return MAX_MODULE_INSTANCES; }

LIB_EXPORT qlex_t *qxir_get_lexer(qmodule_t *mod) { return mod->getLexer(); }

LIB_EXPORT qxir_node_t *qxir_base(qmodule_t *mod) {
  return reinterpret_cast<qxir_node_t *>(mod->getRoot());
}

LIB_EXPORT qxir_conf_t *qxir_get_conf(qmodule_t *mod) { return mod->getConf(); }
