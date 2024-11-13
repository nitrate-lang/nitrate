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

#include <core/LibMacro.h>
#include <nitrate-core/Error.h>

#include <memory>
#include <mutex>
#include <nitrate-ir/Module.hh>

#include "nitrate-ir/Report.hh"

using namespace nr;

static std::vector<std::optional<qmodule_t *>> nr_modules;
static std::mutex nr_modules_mutex;

class LexerSourceResolver : public diag::IOffsetResolver {
public:
  virtual std::optional<std::pair<uint32_t, uint32_t>> resolve(uint32_t) noexcept override {
    qcore_implement();
  }
  virtual ~LexerSourceResolver() = default;
};

qmodule_t::qmodule_t(ModuleId id, const std::string &name) {
  m_applied.clear();
  m_strings.clear();

  auto resolver = std::make_shared<LexerSourceResolver>();
  m_diag = std::make_unique<diag::DiagnosticManager>(resolver);

  m_module_name = name;

  m_lexer = nullptr;
  m_root = nullptr;
  m_diagnostics_enabled = true;

  m_id = id;
}

qmodule_t::~qmodule_t() {
  m_lexer = nullptr;
  m_root = nullptr;
}

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

qmodule_t *nr::createModule(std::string name) {
  std::lock_guard<std::mutex> lock(nr_modules_mutex);

  ModuleId mid;

  for (mid = 0; mid < nr_modules.size(); mid++) {
    if (!nr_modules[mid].has_value()) {
      break;
    }
  }

  if (mid >= MAX_MODULE_INSTANCES) {
    return nullptr;
  }

  nr_modules.insert(nr_modules.begin() + mid, new qmodule_t(mid, name));

  return nr_modules[mid].value();
}

CPP_EXPORT qmodule_t *nr::getModule(ModuleId mid) {
  std::lock_guard<std::mutex> lock(nr_modules_mutex);

  if (mid >= nr_modules.size() || !nr_modules.at(mid).has_value()) {
    return nullptr;
  }

  return nr_modules.at(mid).value();
}

LIB_EXPORT qmodule_t *nr_new(qlex_t *lexer, nr_conf_t *conf, const char *name) {
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

  obj->setLexer(lexer);

  return obj;
}

LIB_EXPORT void nr_free(qmodule_t *mod) {
  if (!mod) {
    return;
  }

  std::lock_guard<std::mutex> lock(nr_modules_mutex);

  auto mid = mod->getModuleId();
  delete mod;
  nr_modules.at(mid).reset();
}

LIB_EXPORT size_t nr_max_modules(void) { return MAX_MODULE_INSTANCES; }

LIB_EXPORT qlex_t *nr_get_lexer(qmodule_t *mod) { return mod->getLexer(); }

LIB_EXPORT nr_node_t *nr_base(qmodule_t *mod) {
  return reinterpret_cast<nr_node_t *>(mod->getRoot());
}
