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

#ifndef __NITRATE_IR_TRANSFORM_PASS_MANAGER_H__
#define __NITRATE_IR_TRANSFORM_PASS_MANAGER_H__

#include <functional>
#include <nitrate-ir/IR/Fwd.hh>
#include <nitrate-ir/Module.hh>
#include <vector>

namespace ncc::ir::transform {
  using FunctionPass =
      std::function<void(ncc::ir::Function&, ncc::ir::IRModule&, void*)>;

#define NITRATE_IR_PASS(NAME, ...) \
  void IR_Pass_##NAME(ncc::ir::Function& func, ncc::ir::IRModule& M, void* data)

  class IPassManager {
  public:
    virtual ~IPassManager() = default;
    virtual void AddPass(FunctionPass pass) = 0;
    virtual void Apply() = 0;
  };

  class PassManager final : public IPassManager {
    std::vector<FunctionPass> m_passes;
    IRModule& m_module;
    void* m_data;

  public:
    PassManager(IRModule& m, void* data) : m_module(m), m_data(data) {}
    virtual ~PassManager() = default;

    void AddPass(FunctionPass pass) override { m_passes.push_back(pass); }
    void Apply() override {
      for (auto& pass : m_passes) {
        std::ranges::for_each(m_module.GetFunctions(), [&](const auto& func) {
          pass(*func, m_module, m_data);
        });
      }
    }
  };
}  // namespace ncc::ir::transform

#endif
