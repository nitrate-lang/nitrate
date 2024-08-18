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

#define __QXIR_IMPL__

#include <core/LibMacro.h>
#include <quix-qxir/Module.h>

static std::vector<std::optional<std::unique_ptr<qmodule_t>>> qxir_modules;
static std::mutex qxir_modules_mutex;

qmodule_t::qmodule_t(qxir::ModuleId id) {
  m_passes_applied.clear();
  m_strings.clear();
  m_diag = std::make_unique<qxir::diag::DiagnosticManager>();
  m_type_mgr = std::make_unique<qxir::TypeManager>();

  qcore_arena_open(&m_arena);

  m_conf = nullptr;
  m_lexer = nullptr;
  m_root = nullptr;

  m_id = id;
}

qmodule_t::~qmodule_t() {
  qcore_arena_close(&m_arena);

  std::lock_guard<std::mutex> lock(qxir_modules_mutex);
  qxir_modules[m_id].reset();
}

qxir::ModuleId qmodule_t::getModuleId() noexcept { return m_id; }

qxir::Type *qmodule_t::lookupType(qxir::TypeID tid)  { return m_type_mgr->get(tid); }

void qmodule_t::setRoot(qxir_node_t *root) noexcept { m_root = root; }

qxir_node_t *qmodule_t::getRoot() noexcept { return m_root; }

void qmodule_t::applyPassLabel(const std::string &label)  {
  m_passes_applied.insert(label);
}

bool qmodule_t::hasPassBeenRun(const std::string &label)  {
  return m_passes_applied.contains(label);
}

std::string_view qmodule_t::push_string(std::string_view sv) {
  for (const auto &str : m_strings) {
    if (str == sv) {
      return str;
    }
  }

  return m_strings.insert(std::string(sv)).first->c_str();
}

std::unique_ptr<qmodule_t> qxir::createModule()  {
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

  if (mid == qxir_modules.size()) {
    qxir_modules.push_back(std::make_unique<qmodule_t>(mid));
  } else {
    qxir_modules[mid] = std::make_unique<qmodule_t>(mid);
  }

  return std::move(qxir_modules[mid].value());
}

CPP_EXPORT qmodule_t *qxir::getModule(qxir::ModuleId mid)  {
  std::lock_guard<std::mutex> lock(qxir_modules_mutex);

  qcore_assert(mid < qxir_modules.size() && qxir_modules.at(mid).has_value(), "Module not found");
  return qxir_modules.at(mid)->get();
}

LIB_EXPORT qmodule_t *qxir_new(qlex_t *lexer, qxir_conf_t *conf) {
  try {
    if (!conf) {
      return nullptr;
    }

    qmodule_t *obj = qxir::createModule().release();

    obj->m_conf = conf;
    obj->m_lexer = lexer;
    // qxir->impl->diag.set_ctx(qxir);
    /// TODO:
    qcore_implement("qxir_new");

    // return qxir;
  } catch (...) {
    return nullptr;
  }
}

LIB_EXPORT void qxir_free(qmodule_t *qxir) {
  try {
    if (!qxir) {
      return;
    }

    // delete qxir->impl;

    // qxir->impl = nullptr;
    // qxir->conf = nullptr;
    // qxir->root = nullptr;
    // qxir->lexer = nullptr;
    /// TODO: Implement qxir_free
    qcore_implement("qxir_free");

    // delete qxir;
  } catch (...) {
    return;
  }
}

LIB_EXPORT size_t qxir_max_modules(void) { return qxir::MAX_MODULE_INSTANCES; }
