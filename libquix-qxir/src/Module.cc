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

#include <LibMacro.h>
#include <quix-qxir/Node.h>

static std::vector<std::optional<std::unique_ptr<qxir::Module>>> qxir_modules;
static std::mutex qxir_modules_mutex;

CPP_EXPORT qxir::Module::Module(ModuleId id) { m_id = id; }

CPP_EXPORT qxir::Module::~Module() {
  if (m_arena) {
    qcore_arena_close(&m_arena.value());
  }

  std::lock_guard<std::mutex> lock(qxir_modules_mutex);
  qxir_modules[m_id].reset();
}

CPP_EXPORT std::unique_ptr<qxir::Module> qxir::createModule() noexcept {
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
    qxir_modules.push_back(std::make_unique<Module>(mid));
  } else {
    qxir_modules[mid] = std::make_unique<Module>(mid);
  }

  return std::move(qxir_modules[mid].value());
}

CPP_EXPORT qxir::Module *qxir::getModule(qxir::ModuleId mid) noexcept {
  std::lock_guard<std::mutex> lock(qxir_modules_mutex);

  qcore_assert(mid < qxir_modules.size() && qxir_modules.at(mid).has_value(), "Module not found");
  return qxir_modules.at(mid)->get();
}
