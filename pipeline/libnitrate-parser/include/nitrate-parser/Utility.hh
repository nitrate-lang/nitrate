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

#ifndef __NITRATE_AST_UTILITY_H__
#define __NITRATE_AST_UTILITY_H__

#include <memory>
#include <nitrate-core/FlowPtr.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-parser/ASTFwd.hh>
#include <source_location>

namespace ncc::parse {
  /* This function takes template variadic arguments and forwards them into
   * the constructor of type T. If compiled with debugging, the source location
   * of the original call site is saved for the purposes of data-flow analysis
   * and AST debugging.
   */
  template <typename T, typename... Args>
  constexpr static inline auto CreateNode(Args&&... args) {
    return [&](std::source_location origin = std::source_location::current()) {
      FlowPtr<T> new_obj = MakeFlowPtr<T>(new (Arena<T>().allocate(1)) T(std::forward<Args>(args)...));  // NOLINT

      new_obj.SetTracking(origin);

      return new_obj;
    };
  }

  class NCC_EXPORT ASTRoot final {
    FlowPtr<Base> m_base;
    bool m_failed;
    std::shared_ptr<void> m_allocator;

  public:
    constexpr ASTRoot(auto base, auto allocator, auto failed)
        : m_base(std::move(base)), m_failed(failed), m_allocator(std::move(allocator)) {}

    auto Get() -> FlowPtr<Base>& { return m_base; }
    [[nodiscard]] auto Get() const -> FlowPtr<Base> { return m_base; }

    [[nodiscard]] auto Check() const -> bool;
  };
}  // namespace ncc::parse

#endif
