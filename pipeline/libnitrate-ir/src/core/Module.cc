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

#include <nitrate-core/Error.h>
#include <nitrate-core/Macro.h>

#include <core/Diagnostic.hh>
#include <memory>
#include <mutex>
#include <nitrate-ir/IRGraph.hh>
#include <nitrate-ir/Module.hh>

using namespace nr;

static std::vector<std::optional<qmodule_t *>> nr_modules;
static std::mutex nr_modules_mutex;

class LexerSourceResolver : public ISourceView {
public:
  virtual std::optional<std::pair<uint32_t, uint32_t>> off2rc(
      uint32_t) override {
    return std::nullopt;
    /// TODO: Implement source offset resolver
    qcore_implement();
  }

  virtual std::optional<std::vector<std::string_view>> rect(
      uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1) override {
    /// TODO: Implement source offset resolver
    qcore_implement();

    (void)x0;
    (void)y0;
    (void)x1;
    (void)y1;

    return std::nullopt;
  }

  virtual ~LexerSourceResolver() = default;
};

qmodule_t::qmodule_t(ModuleId id, const std::string &name) {
  m_applied.clear();

  m_offset_resolver = std::make_unique<LexerSourceResolver>();
  m_diagnostics = std::make_unique<DiagnosticManager>();

  m_module_name = name;

  m_root = nullptr;
  m_diagnostics_enabled = true;

  m_id = id;
}

qmodule_t::~qmodule_t() { m_root = nullptr; }

void qmodule_t::enableDiagnostics(bool is_enabled) {
  m_diagnostics_enabled = is_enabled;
}

CPP_EXPORT void qmodule_t::accept(nr::NRVisitor &visitor) {
  m_root->accept(visitor);
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

C_EXPORT void nr_free(qmodule_t *mod) {
  if (!mod) {
    return;
  }

  std::lock_guard<std::mutex> lock(nr_modules_mutex);

  auto mid = mod->getModuleId();
  delete mod;
  nr_modules.at(mid).reset();
}

C_EXPORT size_t nr_max_modules(void) { return MAX_MODULE_INSTANCES; }

C_EXPORT nr_node_t *nr_base(qmodule_t *mod) {
  return reinterpret_cast<nr_node_t *>(mod->getRoot());
}
