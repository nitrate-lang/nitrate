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

#include <core/Diagnostic.hh>
#include <memory>
#include <mutex>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-ir/IRGraph.hh>
#include <nitrate-ir/Module.hh>

using namespace ncc;
using namespace ncc::ir;

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

qmodule_t::qmodule_t(const std::string &name) {
  m_applied.clear();

  m_offset_resolver = std::make_unique<LexerSourceResolver>();
  m_diagnostics = std::make_unique<DiagnosticManager>();

  m_module_name = name;

  m_root = nullptr;
  m_diagnostics_enabled = true;
}

qmodule_t::~qmodule_t() { m_root = nullptr; }

void qmodule_t::enableDiagnostics(bool is_enabled) {
  m_diagnostics_enabled = is_enabled;
}

CPP_EXPORT void qmodule_t::accept(ir::NRVisitor &visitor) {
  m_root->accept(visitor);
}

///=============================================================================

qmodule_t *ir::createModule(std::string name) { return new qmodule_t(name); }
