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

#ifndef __NITRATE_IR_MODULE_H__
#define __NITRATE_IR_MODULE_H__

#include <boost/bimap.hpp>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <nitrate-core/Allocate.hh>
#include <nitrate-core/FlowPtr.hh>
#include <nitrate-core/NullableFlowPtr.hh>
#include <nitrate-core/String.hh>
#include <nitrate-ir/IR/Fwd.hh>
#include <nitrate-ir/IR/Visitor.hh>
#include <ranges>
#include <string>
#include <vector>

namespace ncc::ir {
  struct TargetInfo {
    uint16_t m_PointerSizeBytes = 8;
    std::optional<string> m_TargetTriple, m_CPU, m_CPUFeatures;
  };

  class NRBuilder;

  class IRModule final {
    friend Expr;
    friend class NRBuilder;

    using FunctionNameBimap = boost::bimap<std::string, std::pair<FnTy *, Function *>>;

    NullableFlowPtr<Seq> m_root;
    FunctionNameBimap m_functions{};

    std::vector<string> m_applied{};
    TargetInfo m_target_info{};
    string m_module_name{};
    bool m_diagnostics_enabled{};

    std::unique_ptr<std::pmr::memory_resource> m_ir_data;

  public:
    IRModule(string module_name = "module");
    ~IRModule();

    [[nodiscard]] auto GetRoot() const { return m_root; }

    [[nodiscard]] auto GetTransformHistory() const -> std::span<const string> { return m_applied; }
    auto Diagnostics(std::optional<bool> state = std::nullopt) -> bool;
    auto Name(std::optional<string> name = std::nullopt) -> string;
    auto GetNodeArena() -> auto & { return m_ir_data; }
    [[nodiscard]] auto GetTargetInfo() const { return m_target_info; }
    [[nodiscard]] auto GetFunctions() const {
      return m_functions.left | std::views::transform([](auto &pair) { return pair.second.second; });
    }

    void Accept(IRVisitor<void> &visitor);
  };

  constexpr size_t kQmoduleSize = sizeof(IRModule);
}  // namespace ncc::ir

#endif
