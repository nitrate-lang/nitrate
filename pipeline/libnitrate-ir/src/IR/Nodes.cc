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

thread_local std::unique_ptr<std::pmr::memory_resource> ncc::ir::NrAllocator = std::make_unique<ncc::DynamicArena>();

struct PtrState {
  FlowPtr<Type> m_pointee;
  uint8_t m_native_size;

  bool operator==(PtrState const& other) const {
    return m_pointee->IsEq(other.m_pointee.get()) && m_native_size == other.m_native_size;
  }
};

struct PtrStateHash {
  size_t operator()(PtrState const& state) const {
    return std::hash<FlowPtr<Type>>{}(state.m_pointee) ^ state.m_native_size;
  }
};

struct ConstState {
  FlowPtr<Type> m_item;

  bool operator==(ConstState const& other) const { return m_item->IsEq(other.m_item.get()); }
};

struct ConstStateHash {
  size_t operator()(ConstState const& state) const { return std::hash<FlowPtr<Type>>{}(state.m_item); }
};

struct OpaqueState {
  string m_name;

  bool operator==(OpaqueState const& other) const { return m_name == other.m_name; }
};

struct OpaqueStateHash {
  size_t operator()(OpaqueState const& state) const { return std::hash<string>{}(state.m_name); }
};

struct StructState {
  std::span<FlowPtr<Type>> m_fields;

  bool operator==(StructState const& other) const {
    if (m_fields.size() != other.m_fields.size()) {
      return false;
    }

    for (size_t i = 0; i < m_fields.size(); i++) {
      if (!m_fields[i]->IsEq(other.m_fields[i].get())) {
        return false;
      }
    }

    return true;
  }
};

struct StructStateHash {
  size_t operator()(StructState const& state) const {
    size_t hash = 0;

    for (auto const& field : state.m_fields) {
      hash ^= std::hash<FlowPtr<Type>>{}(field);
    }

    return hash;
  }
};

struct UnionState {
  std::span<FlowPtr<Type>> m_fields;

  bool operator==(UnionState const& other) const {
    if (m_fields.size() != other.m_fields.size()) {
      return false;
    }

    for (size_t i = 0; i < m_fields.size(); i++) {
      if (!m_fields[i]->IsEq(other.m_fields[i].get())) {
        return false;
      }
    }

    return true;
  }
};

struct UnionStateHash {
  size_t operator()(UnionState const& state) const {
    size_t hash = 0;

    for (auto const& field : state.m_fields) {
      hash ^= std::hash<FlowPtr<Type>>{}(field);
    }

    return hash;
  }
};

struct ArrayState {
  FlowPtr<Type> m_element;
  size_t m_size;

  bool operator==(ArrayState const& other) const {
    return m_element->IsEq(other.m_element.get()) && m_size == other.m_size;
  }
};

struct ArrayStateHash {
  size_t operator()(ArrayState const& state) const {
    return std::hash<FlowPtr<Type>>{}(state.m_element) ^ state.m_size;
  }
};

struct FnState {
  std::span<FlowPtr<Type>> m_params;
  FlowPtr<Type> m_ret;
  bool m_variadic;
  size_t m_native_size;

  bool operator==(FnState const& other) const {
    if (m_params.size() != other.m_params.size()) {
      return false;
    }

    for (size_t i = 0; i < m_params.size(); i++) {
      if (!m_params[i]->IsEq(other.m_params[i].get())) {
        return false;
      }
    }

    return m_ret->IsEq(other.m_ret.get()) && m_variadic == other.m_variadic && m_native_size == other.m_native_size;
  }
};

struct FnStateHash {
  size_t operator()(FnState const& state) const {
    size_t hash = 0;

    for (auto const& param : state.m_params) {
      hash ^= std::hash<FlowPtr<Type>>{}(param);
    }

    return hash ^ std::hash<FlowPtr<Type>>{}(state.m_ret) ^ state.m_variadic ^ state.m_native_size;
  }
};

static std::unordered_map<PtrState, std::unique_ptr<PtrTy>, PtrStateHash> GPtrCache;
static std::unordered_map<ConstState, std::unique_ptr<ConstTy>, ConstStateHash> GConstCache;
static std::unordered_map<OpaqueState, std::unique_ptr<OpaqueTy>, OpaqueStateHash> GOpaqueCache;
static std::unordered_map<StructState, std::pair<std::unique_ptr<StructTy>, std::vector<FlowPtr<Type>>>,
                          StructStateHash>
    GStructCache;
static std::unordered_map<UnionState, std::pair<std::unique_ptr<UnionTy>, std::vector<FlowPtr<Type>>>, UnionStateHash>
    GUnionCache;
static std::unordered_map<ArrayState, std::unique_ptr<ArrayTy>, ArrayStateHash> GArrayCache;
static std::unordered_map<FnState, std::unique_ptr<FnTy>, FnStateHash> GFnCache;

static std::mutex GPtrCacheMutex, GConstCacheMutex, GOpaqueCacheMutex, GStructCacheMutex, GUnionCacheMutex,
    GArrayCacheMutex, GFnCacheMutex;

void IRResetTypeCache() {
  std::lock_guard l0(GPtrCacheMutex), l1(GConstCacheMutex), l2(GOpaqueCacheMutex), l3(GStructCacheMutex),
      l4(GUnionCacheMutex), l5(GArrayCacheMutex), l6(GFnCacheMutex);

  GPtrCache.clear();
  GConstCache.clear();
  GOpaqueCache.clear();
  GStructCache.clear();
  GUnionCache.clear();
  GArrayCache.clear();
  GFnCache.clear();
}

NCC_EXPORT PtrTy* ir::GetPtrTy(FlowPtr<Type> pointee, uint8_t native_size) {
  PtrState state{pointee, native_size};

  std::lock_guard lock(GPtrCacheMutex);
  auto it = GPtrCache.find(state);

  if (it == GPtrCache.end()) [[unlikely]] {
    it = GPtrCache.emplace(std::move(state), std::make_unique<PtrTy>(pointee, native_size)).first;
  }

  return it->second.get();
}

NCC_EXPORT ConstTy* ir::GetConstTy(FlowPtr<Type> item) {
  ConstState state{item};

  std::lock_guard lock(GConstCacheMutex);
  auto it = GConstCache.find(state);

  if (it == GConstCache.end()) [[unlikely]] {
    it = GConstCache.emplace(std::move(state), std::make_unique<ConstTy>(item)).first;
  }

  return it->second.get();
}

NCC_EXPORT OpaqueTy* ir::GetOpaqueTy(string name) {
  OpaqueState state{name};

  std::lock_guard lock(GOpaqueCacheMutex);
  auto it = GOpaqueCache.find(state);

  if (it == GOpaqueCache.end()) [[unlikely]] {
    it = GOpaqueCache.emplace(std::move(state), std::make_unique<OpaqueTy>(name)).first;
  }

  return it->second.get();
}

NCC_EXPORT StructTy* ir::GetStructTy(std::span<FlowPtr<Type>> fields) {
  StructState state{fields};

  std::lock_guard lock(GStructCacheMutex);
  auto it = GStructCache.find(state);

  if (it == GStructCache.end()) [[unlikely]] {
    using Value = std::pair<std::unique_ptr<StructTy>, std::vector<FlowPtr<Type>>>;

    Value v{nullptr, std::vector<FlowPtr<Type>>(fields.begin(), fields.end())};
    v.first = std::make_unique<StructTy>(v.second);

    it = GStructCache.emplace(std::move(state), std::move(v)).first;
  }

  return it->second.first.get();
}

NCC_EXPORT UnionTy* ir::GetUnionTy(std::span<FlowPtr<Type>> fields) {
  UnionState state{fields};

  std::lock_guard lock(GUnionCacheMutex);
  auto it = GUnionCache.find(state);

  if (it == GUnionCache.end()) [[unlikely]] {
    using Value = std::pair<std::unique_ptr<UnionTy>, std::vector<FlowPtr<Type>>>;

    Value v{nullptr, std::vector<FlowPtr<Type>>(fields.begin(), fields.end())};
    v.first = std::make_unique<UnionTy>(v.second);

    it = GUnionCache.emplace(std::move(state), std::move(v)).first;
  }

  return it->second.first.get();
}

NCC_EXPORT ArrayTy* ir::GetArrayTy(FlowPtr<Type> element, size_t size) {
  ArrayState state{element, size};

  std::lock_guard lock(GArrayCacheMutex);
  auto it = GArrayCache.find(state);

  if (it == GArrayCache.end()) [[unlikely]] {
    it = GArrayCache.emplace(std::move(state), std::make_unique<ArrayTy>(element, size)).first;
  }

  return it->second.get();
}

NCC_EXPORT FnTy* ir::GetFnTy(std::span<FlowPtr<Type>> params, FlowPtr<Type> ret, bool variadic, size_t native_size) {
  FnState state{params, ret, variadic, native_size};

  std::lock_guard lock(GFnCacheMutex);
  auto it = GFnCache.find(state);

  if (it == GFnCache.end()) [[unlikely]] {
    it = GFnCache.emplace(std::move(state), std::make_unique<FnTy>(params, ret, variadic, native_size)).first;
  }

  return it->second.get();
}

NCC_EXPORT FlowPtr<Expr> ir::CreateIgn() { return Create<Expr>(IR_eIGN); }

///=============================================================================

static bool IsCyclicUtil(auto base, std::unordered_set<FlowPtr<Expr>>& visited,
                         std::unordered_set<FlowPtr<Expr>>& rec_stack) {
  bool has_cycle = false;

  if (!visited.contains(base)) {
    // Mark the current node as visited
    // and part of recursion stack
    visited.insert(base);
    rec_stack.insert(base);

    // Recurse for all the vertices adjacent
    // to this vertex
    iterate<IterMode::children>(base, [&](auto, auto const* const cur) -> IterOp {
      if (!visited.contains(*cur) && IsCyclicUtil(*cur, visited, rec_stack)) [[unlikely]] {
        has_cycle = true;
        return IterOp::Abort;
      } else if (rec_stack.contains(*cur)) [[unlikely]] {
        has_cycle = true;
        return IterOp::Abort;
      }

      return IterOp::Proceed;
    });
  }

  // Remove the vertex from recursion stack
  rec_stack.erase(base);
  return has_cycle;
}

NCC_EXPORT bool detail::IsAcyclicImpl(FlowPtr<Expr> self) {
  std::unordered_set<FlowPtr<Expr>> visited, rec_stack;
  bool has_cycle = false;

  iterate<IterMode::children>(self, [&](auto, auto c) -> IterOp {
    if (!visited.contains(*c) && IsCyclicUtil(*c, visited, rec_stack)) [[unlikely]] {
      has_cycle = true;
      return IterOp::Abort;
    }

    return IterOp::Proceed;
  });

  return !has_cycle;
}
