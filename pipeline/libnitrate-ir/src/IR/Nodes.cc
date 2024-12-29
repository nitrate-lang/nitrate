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

#include <boost/uuid/name_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <cstring>
#include <nitrate-core/Allocate.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-ir/IR.hh>
#include <nitrate-ir/IR/Nodes.hh>
#include <nitrate-ir/Module.hh>
#include <unordered_set>

using namespace ncc;
using namespace ncc::ir;

thread_local std::unique_ptr<ncc::IMemory> ncc::ir::nr_allocator =
    std::make_unique<ncc::dyn_arena>();

struct PtrState {
  FlowPtr<Type> pointee;
  uint8_t native_size;

  bool operator==(PtrState const& other) const {
    return pointee->isSame(other.pointee.get()) &&
           native_size == other.native_size;
  }
};

struct PtrStateHash {
  size_t operator()(PtrState const& state) const {
    return std::hash<FlowPtr<Type>>{}(state.pointee) ^ state.native_size;
  }
};

struct ConstState {
  FlowPtr<Type> item;

  bool operator==(ConstState const& other) const {
    return item->isSame(other.item.get());
  }
};

struct ConstStateHash {
  size_t operator()(ConstState const& state) const {
    return std::hash<FlowPtr<Type>>{}(state.item);
  }
};

struct OpaqueState {
  string name;

  bool operator==(OpaqueState const& other) const { return name == other.name; }
};

struct OpaqueStateHash {
  size_t operator()(OpaqueState const& state) const {
    return std::hash<string>{}(state.name);
  }
};

struct StructState {
  std::span<FlowPtr<Type>> fields;

  bool operator==(StructState const& other) const {
    if (fields.size() != other.fields.size()) {
      return false;
    }

    for (size_t i = 0; i < fields.size(); i++) {
      if (!fields[i]->isSame(other.fields[i].get())) {
        return false;
      }
    }

    return true;
  }
};

struct StructStateHash {
  size_t operator()(StructState const& state) const {
    size_t hash = 0;

    for (auto const& field : state.fields) {
      hash ^= std::hash<FlowPtr<Type>>{}(field);
    }

    return hash;
  }
};

struct UnionState {
  std::span<FlowPtr<Type>> fields;

  bool operator==(UnionState const& other) const {
    if (fields.size() != other.fields.size()) {
      return false;
    }

    for (size_t i = 0; i < fields.size(); i++) {
      if (!fields[i]->isSame(other.fields[i].get())) {
        return false;
      }
    }

    return true;
  }
};

struct UnionStateHash {
  size_t operator()(UnionState const& state) const {
    size_t hash = 0;

    for (auto const& field : state.fields) {
      hash ^= std::hash<FlowPtr<Type>>{}(field);
    }

    return hash;
  }
};

struct ArrayState {
  FlowPtr<Type> element;
  size_t size;

  bool operator==(ArrayState const& other) const {
    return element->isSame(other.element.get()) && size == other.size;
  }
};

struct ArrayStateHash {
  size_t operator()(ArrayState const& state) const {
    return std::hash<FlowPtr<Type>>{}(state.element) ^ state.size;
  }
};

struct FnState {
  std::span<FlowPtr<Type>> params;
  FlowPtr<Type> ret;
  bool variadic;
  size_t native_size;

  bool operator==(FnState const& other) const {
    if (params.size() != other.params.size()) {
      return false;
    }

    for (size_t i = 0; i < params.size(); i++) {
      if (!params[i]->isSame(other.params[i].get())) {
        return false;
      }
    }

    return ret->isSame(other.ret.get()) && variadic == other.variadic &&
           native_size == other.native_size;
  }
};

struct FnStateHash {
  size_t operator()(FnState const& state) const {
    size_t hash = 0;

    for (auto const& param : state.params) {
      hash ^= std::hash<FlowPtr<Type>>{}(param);
    }

    return hash ^ std::hash<FlowPtr<Type>>{}(state.ret) ^ state.variadic ^
           state.native_size;
  }
};

static std::unordered_map<PtrState, std::unique_ptr<PtrTy>, PtrStateHash>
    g_ptr_cache;
static std::unordered_map<ConstState, std::unique_ptr<ConstTy>, ConstStateHash>
    g_const_cache;
static std::unordered_map<OpaqueState, std::unique_ptr<OpaqueTy>,
                          OpaqueStateHash>
    g_opaque_cache;
static std::unordered_map<
    StructState,
    std::pair<std::unique_ptr<StructTy>, std::vector<FlowPtr<Type>>>,
    StructStateHash>
    g_struct_cache;
static std::unordered_map<
    UnionState, std::pair<std::unique_ptr<UnionTy>, std::vector<FlowPtr<Type>>>,
    UnionStateHash>
    g_union_cache;
static std::unordered_map<ArrayState, std::unique_ptr<ArrayTy>, ArrayStateHash>
    g_array_cache;
static std::unordered_map<FnState, std::unique_ptr<FnTy>, FnStateHash>
    g_fn_cache;

static std::mutex g_ptr_cache_mutex, g_const_cache_mutex, g_opaque_cache_mutex,
    g_struct_cache_mutex, g_union_cache_mutex, g_array_cache_mutex,
    g_fn_cache_mutex;

void IR_resetTypeCache(void) {
  std::lock_guard l0(g_ptr_cache_mutex), l1(g_const_cache_mutex),
      l2(g_opaque_cache_mutex), l3(g_struct_cache_mutex),
      l4(g_union_cache_mutex), l5(g_array_cache_mutex), l6(g_fn_cache_mutex);

  g_ptr_cache.clear();
  g_const_cache.clear();
  g_opaque_cache.clear();
  g_struct_cache.clear();
  g_union_cache.clear();
  g_array_cache.clear();
  g_fn_cache.clear();
}

CPP_EXPORT PtrTy* ir::getPtrTy(FlowPtr<Type> pointee, uint8_t native_size) {
  PtrState state{pointee, native_size};

  std::lock_guard lock(g_ptr_cache_mutex);
  auto it = g_ptr_cache.find(state);

  if (it == g_ptr_cache.end()) [[unlikely]] {
    it = g_ptr_cache
             .emplace(std::move(state),
                      std::make_unique<PtrTy>(pointee, native_size))
             .first;
  }

  return it->second.get();
}

CPP_EXPORT ConstTy* ir::getConstTy(FlowPtr<Type> item) {
  ConstState state{item};

  std::lock_guard lock(g_const_cache_mutex);
  auto it = g_const_cache.find(state);

  if (it == g_const_cache.end()) [[unlikely]] {
    it =
        g_const_cache.emplace(std::move(state), std::make_unique<ConstTy>(item))
            .first;
  }

  return it->second.get();
}

CPP_EXPORT OpaqueTy* ir::getOpaqueTy(string name) {
  OpaqueState state{name};

  std::lock_guard lock(g_opaque_cache_mutex);
  auto it = g_opaque_cache.find(state);

  if (it == g_opaque_cache.end()) [[unlikely]] {
    it = g_opaque_cache
             .emplace(std::move(state), std::make_unique<OpaqueTy>(name))
             .first;
  }

  return it->second.get();
}

CPP_EXPORT StructTy* ir::getStructTy(std::span<FlowPtr<Type>> fields) {
  StructState state{fields};

  std::lock_guard lock(g_struct_cache_mutex);
  auto it = g_struct_cache.find(state);

  if (it == g_struct_cache.end()) [[unlikely]] {
    using Value =
        std::pair<std::unique_ptr<StructTy>, std::vector<FlowPtr<Type>>>;

    Value v{nullptr, std::vector<FlowPtr<Type>>(fields.begin(), fields.end())};
    v.first = std::make_unique<StructTy>(v.second);

    it = g_struct_cache.emplace(std::move(state), std::move(v)).first;
  }

  return it->second.first.get();
}

CPP_EXPORT UnionTy* ir::getUnionTy(std::span<FlowPtr<Type>> fields) {
  UnionState state{fields};

  std::lock_guard lock(g_union_cache_mutex);
  auto it = g_union_cache.find(state);

  if (it == g_union_cache.end()) [[unlikely]] {
    using Value =
        std::pair<std::unique_ptr<UnionTy>, std::vector<FlowPtr<Type>>>;

    Value v{nullptr, std::vector<FlowPtr<Type>>(fields.begin(), fields.end())};
    v.first = std::make_unique<UnionTy>(v.second);

    it = g_union_cache.emplace(std::move(state), std::move(v)).first;
  }

  return it->second.first.get();
}

CPP_EXPORT ArrayTy* ir::getArrayTy(FlowPtr<Type> element, size_t size) {
  ArrayState state{element, size};

  std::lock_guard lock(g_array_cache_mutex);
  auto it = g_array_cache.find(state);

  if (it == g_array_cache.end()) [[unlikely]] {
    it =
        g_array_cache
            .emplace(std::move(state), std::make_unique<ArrayTy>(element, size))
            .first;
  }

  return it->second.get();
}

CPP_EXPORT FnTy* ir::getFnTy(std::span<FlowPtr<Type>> params, FlowPtr<Type> ret,
                             bool variadic, size_t native_size) {
  FnState state{params, ret, variadic, native_size};

  std::lock_guard lock(g_fn_cache_mutex);
  auto it = g_fn_cache.find(state);

  if (it == g_fn_cache.end()) [[unlikely]] {
    it = g_fn_cache
             .emplace(std::move(state), std::make_unique<FnTy>(
                                            params, ret, variadic, native_size))
             .first;
  }

  return it->second.get();
}

CPP_EXPORT FlowPtr<Expr> ir::createIgn() { return create<Expr>(IR_eIGN); }

///=============================================================================

static bool isCyclicUtil(auto base, std::unordered_set<FlowPtr<Expr>>& visited,
                         std::unordered_set<FlowPtr<Expr>>& recStack) {
  bool has_cycle = false;

  if (!visited.contains(base)) {
    // Mark the current node as visited
    // and part of recursion stack
    visited.insert(base);
    recStack.insert(base);

    // Recurse for all the vertices adjacent
    // to this vertex
    iterate<IterMode::children>(
        base, [&](auto, auto const* const cur) -> IterOp {
          if (!visited.contains(*cur) && isCyclicUtil(*cur, visited, recStack))
              [[unlikely]] {
            has_cycle = true;
            return IterOp::Abort;
          } else if (recStack.contains(*cur)) [[unlikely]] {
            has_cycle = true;
            return IterOp::Abort;
          }

          return IterOp::Proceed;
        });
  }

  // Remove the vertex from recursion stack
  recStack.erase(base);
  return has_cycle;
}

CPP_EXPORT bool detail::IsAcyclicImpl(FlowPtr<Expr> self) {
  std::unordered_set<FlowPtr<Expr>> visited, recStack;
  bool has_cycle = false;

  iterate<IterMode::children>(self, [&](auto, auto C) -> IterOp {
    if (!visited.contains(*C) && isCyclicUtil(*C, visited, recStack))
        [[unlikely]] {
      has_cycle = true;
      return IterOp::Abort;
    }

    return IterOp::Proceed;
  });

  return !has_cycle;
}
