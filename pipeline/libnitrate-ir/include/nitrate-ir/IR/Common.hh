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

#ifndef __NITRATE_IR_GRAPH_DATA_H__
#define __NITRATE_IR_GRAPH_DATA_H__

#include <boost/multiprecision/cpp_int.hpp>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <memory>
#include <nitrate-core/Allocate.hh>
#include <nitrate-core/FlowPtr.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-ir/IR/Fwd.hh>
#include <nitrate-lexer/Token.hh>
#include <span>
#include <variant>
#include <vector>

namespace ncc::ir {
  using uint128_t = boost::multiprecision::uint128_t;

  extern thread_local std::unique_ptr<ncc::IMemory> NrAllocator;

  template <class T>
  struct Arena {
    typedef T ValueType;

    Arena() = default;

    template <class U>
    constexpr Arena(const Arena<U> &) {}

    [[nodiscard]] T *Allocate(std::size_t n) {
      return static_cast<T *>(NrAllocator->Alloc(sizeof(T) * n));
    }

    void Deallocate(T *, std::size_t) {}
  };

  template <class T, class U>
  bool operator==(const Arena<T> &, const Arena<U> &) {
    return true;
  }
  template <class T, class U>
  bool operator!=(const Arena<T> &, const Arena<U> &) {
    return false;
  }

  enum class Purity : uint8_t {
    Impure = 0,
    Pure = 1,
    Quasi = 2,
    Retro = 3,
  };

  enum class Vis : uint8_t {
    Sec = 0,
    Pub = 1,
    Pro = 2,
  };

  enum class StorageClass : uint8_t {
    /* Automatic storeage duration */
    LLVM_StackAlloa,

    /* Static storage duration */
    LLVM_Static,

    /* Thread-local storage duration */
    LLVM_ThreadLocal,

    /* Dynamic allocation */
    Managed,
  };

  template <class A>
  using IR_Vertex_CallArguments =
      std::vector<std::pair<string, FlowPtr<GenericExpr<A>>>,
                  Arena<std::pair<string, FlowPtr<GenericExpr<A>>>>>;

  template <class A>
  using FnParams =
      std::vector<FlowPtr<GenericType<A>>, Arena<FlowPtr<GenericType<A>>>>;

  enum class TmpType {
    CALL,
    NAMED_TYPE,
    DEFAULT_VALUE,
  };

  template <class A>
  struct GenericCallArgsTmpNodeCradle {
    FlowPtr<GenericExpr<A>> m_base;
    std::span<std::pair<string, FlowPtr<GenericExpr<A>>>> m_args;

    bool operator==(const GenericCallArgsTmpNodeCradle<A> &rhs) const {
      return m_base == rhs.m_base && m_args == rhs.m_args;
    }
  };

  template <class A>
  using TmpNodeCradle = std::variant<GenericCallArgsTmpNodeCradle<A>, string>;

  template <class A>
  using IR_Vertex_ListItems =
      std::vector<FlowPtr<GenericExpr<A>>, Arena<FlowPtr<GenericExpr<A>>>>;

  enum class AbiTag {
    C,
    Nitrate,
    Internal,
    Default = Nitrate,
  };

  template <class A>
  using IR_Vertex_CallArgs =
      std::vector<FlowPtr<GenericExpr<A>>, Arena<FlowPtr<GenericExpr<A>>>>;

  template <class A>
  using IR_Vertex_SeqItems =
      std::vector<FlowPtr<GenericExpr<A>>, Arena<FlowPtr<GenericExpr<A>>>>;

  template <class A>
  using IR_Vertex_Params =
      std::vector<std::pair<FlowPtr<GenericType<A>>, string>,
                  Arena<std::pair<FlowPtr<GenericType<A>>, string>>>;

  template <class A>
  using IR_Vertex_SwitchCases =
      std::vector<FlowPtr<GenericCase<A>>, Arena<FlowPtr<GenericCase<A>>>>;

}  // namespace ncc::ir

#endif