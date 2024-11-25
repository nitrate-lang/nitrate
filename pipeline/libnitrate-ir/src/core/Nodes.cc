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

#include <limits>
#include <sstream>
#define __NR_NODE_REFLECT_IMPL__  // Make private fields accessible

#include <core/LibMacro.h>
#include <nitrate-core/Error.h>
#include <nitrate-ir/IR.h>
#include <openssl/evp.h>

#include <boost/uuid/name_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <cstdint>
#include <cstring>
#include <nitrate-ir/IRGraph.hh>
#include <nitrate-ir/Module.hh>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <variant>

using namespace nr;

///=============================================================================
namespace nr {
  thread_local ArenaAllocatorImpl nr_arena;

  namespace mem {
    Brk static_QIR_NODE_BRK;
    Cont static_QIR_NODE_CONT;
    Expr static_QIR_NODE_IGN(QIR_NODE_IGN);
    U1Ty static_QIR_NODE_U1_TY;
    U8Ty static_QIR_NODE_U8_TY;
    U16Ty static_QIR_NODE_U16_TY;
    U32Ty static_QIR_NODE_U32_TY;
    U64Ty static_QIR_NODE_U64_TY;
    U128Ty static_QIR_NODE_U128_TY;
    I8Ty static_QIR_NODE_I8_TY;
    I16Ty static_QIR_NODE_I16_TY;
    I32Ty static_QIR_NODE_I32_TY;
    I64Ty static_QIR_NODE_I64_TY;
    I128Ty static_QIR_NODE_I128_TY;
    F16Ty static_QIR_NODE_F16_TY;
    F32Ty static_QIR_NODE_F32_TY;
    F64Ty static_QIR_NODE_F64_TY;
    F128Ty static_QIR_NODE_F128_TY;
    VoidTy static_QIR_NODE_VOID_TY;
  }  // namespace mem
}  // namespace nr

void *ArenaAllocatorImpl::allocate(std::size_t size) {
  const std::size_t alignment = 16;
  return qcore_arena_alloc_ex(m_arena.get(), size, alignment);
}

void ArenaAllocatorImpl::deallocate(void *ptr) noexcept { (void)ptr; }

///=============================================================================

static bool isCyclicUtil(nr::Expr *base,
                         std::unordered_set<nr::Expr *> &visited,
                         std::unordered_set<nr::Expr *> &recStack) {
  bool has_cycle = false;

  if (!visited.contains(base)) {
    // Mark the current node as visited
    // and part of recursion stack
    visited.insert(base);
    recStack.insert(base);

    // Recur for all the vertices adjacent
    // to this vertex
    iterate<IterMode::children>(
        base, [&](nr::Expr *, nr::Expr **cur) -> IterOp {
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

CPP_EXPORT bool nr::Expr::isAcyclic() const noexcept {
  std::unordered_set<Expr *> visited, recStack;
  bool has_cycle = false;

  Expr *ptr = const_cast<Expr *>(this);
  iterate<IterMode::children>(ptr, [&](Expr *, Expr **cur) -> IterOp {
    if (!visited.contains(*cur) && isCyclicUtil(*cur, visited, recStack))
        [[unlikely]] {
      has_cycle = true;
      return IterOp::Abort;
    }

    return IterOp::Proceed;
  });

  return !has_cycle;
}

CPP_EXPORT void nr::Expr::dump(std::ostream &os, bool isForDebug) const {
  (void)isForDebug;

  char *cstr = nullptr;
  size_t len = 0;

  FILE *fmembuf = open_memstream(&cstr, &len);
  if (!nr_write(nullptr, this, NR_SERIAL_CODE, fmembuf, nullptr, 0)) {
    qcore_panic("Failed to dump expression");
  }
  fflush(fmembuf);

  os.write(cstr, len);

  fclose(fmembuf);
  free(cstr);
}

CPP_EXPORT boost::uuids::uuid nr::Expr::hash() noexcept {
  const EVP_MD *md = EVP_sha256();
  std::array<uint8_t, EVP_MAX_MD_SIZE> hash;

  EVP_MD_CTX *ctx = EVP_MD_CTX_new();
  if (!ctx) {
    qcore_panic("Failed to create EVP_MD_CTX");
  }

  if (EVP_DigestInit(ctx, md) != 1) {
    qcore_panic("Failed to initialize EVP_MD_CTX");
  }

  Expr *ptr = this;
  iterate<IterMode::dfs_pre>(ptr, [ctx](Expr *, Expr **_cur) -> IterOp {
    Expr *cur = *_cur;
    uint8_t kind = static_cast<uint8_t>(cur->getKind());

    if (EVP_DigestUpdate(ctx, &kind, sizeof(kind)) != 1) {
      qcore_panic("Failed to update EVP_MD_CTX");
    }

#define MIXIN_PRIMITIVE(x) EVP_DigestUpdate(ctx, &x, sizeof(x))
#define MIXIN_STRING(x) EVP_DigestUpdate(ctx, x.data(), x.size())

    switch (kind) {
      case QIR_NODE_BINEXPR: {
        MIXIN_PRIMITIVE(cur->as<BinExpr>()->m_op);
        break;
      }
      case QIR_NODE_UNEXPR: {
        MIXIN_PRIMITIVE(cur->as<UnExpr>()->m_op);
        break;
      }
      case QIR_NODE_POST_UNEXPR: {
        MIXIN_PRIMITIVE(cur->as<PostUnExpr>()->m_op);
        break;
      }
      case QIR_NODE_INT: {
        uint128_t x = cur->as<Int>()->getValue();
        MIXIN_PRIMITIVE(x);
        break;
      }
      case QIR_NODE_FLOAT: {
        double v = cur->as<Float>()->getValue();
        FloatSize s = cur->as<Float>()->getSize();
        MIXIN_PRIMITIVE(v);
        MIXIN_PRIMITIVE(s);
        break;
      }
      case QIR_NODE_LIST: {
        break;
      }
      case QIR_NODE_CALL: {
        break;
      }
      case QIR_NODE_SEQ: {
        break;
      }
      case QIR_NODE_INDEX: {
        break;
      }
      case QIR_NODE_IDENT: {
        MIXIN_STRING(cur->as<Ident>()->m_name);
        break;
      }
      case QIR_NODE_EXTERN: {
        MIXIN_STRING(cur->as<Extern>()->m_abi_name);
        break;
      }
      case QIR_NODE_LOCAL: {
        MIXIN_STRING(cur->as<Local>()->m_name);
        break;
      }
      case QIR_NODE_RET: {
        break;
      }
      case QIR_NODE_BRK: {
        break;
      }
      case QIR_NODE_CONT: {
        break;
      }
      case QIR_NODE_IF: {
        break;
      }
      case QIR_NODE_WHILE: {
        break;
      }
      case QIR_NODE_FOR: {
        break;
      }
      case QIR_NODE_CASE: {
        break;
      }
      case QIR_NODE_SWITCH: {
        break;
      }
      case QIR_NODE_IGN: {
        break;
      }
      case QIR_NODE_FN: {
        MIXIN_STRING(cur->as<Fn>()->m_name);
        break;
      }
      case QIR_NODE_ASM: {
        qcore_implement();
        break;
      }
      case QIR_NODE_U1_TY: {
        break;
      }
      case QIR_NODE_U8_TY: {
        break;
      }
      case QIR_NODE_U16_TY: {
        break;
      }
      case QIR_NODE_U32_TY: {
        break;
      }
      case QIR_NODE_U64_TY: {
        break;
      }
      case QIR_NODE_U128_TY: {
        break;
      }
      case QIR_NODE_I8_TY: {
        break;
      }
      case QIR_NODE_I16_TY: {
        break;
      }
      case QIR_NODE_I32_TY: {
        break;
      }
      case QIR_NODE_I64_TY: {
        break;
      }
      case QIR_NODE_I128_TY: {
        break;
      }
      case QIR_NODE_F16_TY: {
        break;
      }
      case QIR_NODE_F32_TY: {
        break;
      }
      case QIR_NODE_F64_TY: {
        break;
      }
      case QIR_NODE_F128_TY: {
        break;
      }
      case QIR_NODE_VOID_TY: {
        break;
      }
      case QIR_NODE_PTR_TY: {
        break;
      }
      case QIR_NODE_OPAQUE_TY: {
        MIXIN_STRING(cur->as<OpaqueTy>()->m_name);
        break;
      }
      case QIR_NODE_STRUCT_TY: {
        break;
      }
      case QIR_NODE_UNION_TY: {
        break;
      }
      case QIR_NODE_ARRAY_TY: {
        break;
      }
      case QIR_NODE_FN_TY: {
        std::set<FnAttr> tags;
        for (auto &tag : cur->as<FnTy>()->m_attrs) {
          tags.insert(tag);
        }
        for (auto tag : tags) {
          MIXIN_PRIMITIVE(tag);
        }
        break;
      }
      case QIR_NODE_TMP: {
        MIXIN_PRIMITIVE(cur->as<Tmp>()->m_type);

        if (std::holds_alternative<CallArgsTmpNodeCradle>(
                cur->as<Tmp>()->m_data)) {
          const CallArgsTmpNodeCradle &data =
              std::get<CallArgsTmpNodeCradle>(cur->as<Tmp>()->m_data);
          if (data.base != nullptr) {
            MIXIN_STRING(data.base->getStateUUID());
          }
          for (const auto &arg : data.args) {
            MIXIN_STRING(arg.first);
            MIXIN_STRING(arg.second->getStateUUID());
          }
        } else if (std::holds_alternative<std::string_view>(
                       cur->as<Tmp>()->m_data)) {
          std::string_view &data =
              std::get<std::string_view>(cur->as<Tmp>()->m_data);
          MIXIN_STRING(data);
        } else {
          qcore_panic("Unknown TmpNodeCradle inner type");
        }
        break;
      }
    }

    return IterOp::Proceed;
  });

  if (EVP_DigestFinal_ex(ctx, hash.data(), nullptr) != 1) {
    qcore_panic("Failed to finalize EVP_MD_CTX");
  }

  EVP_MD_CTX_free(ctx);

  boost::uuids::uuid uuid;
  std::memcpy(uuid.data, hash.data(), uuid.size());
  boost::uuids::name_generator gen(uuid);
  return gen("nr");
}

CPP_EXPORT uint64_t Expr::getUniqId() const {
  static thread_local std::unordered_map<const Expr *, uint64_t> id_map;
  static thread_local uint64_t last = 0;

  if (id_map.contains(this)) {
    return id_map.at(this);
  }

  for (auto &[key, value] : id_map) {
    if (key->isSame(this)) {
      return value;
    }
  }

  id_map[this] = last;

  return last++;
}

///=============================================================================

CPP_EXPORT bool Type::is_primitive() const {
  switch (getKind()) {
    case QIR_NODE_U1_TY:
    case QIR_NODE_U8_TY:
    case QIR_NODE_U16_TY:
    case QIR_NODE_U32_TY:
    case QIR_NODE_U64_TY:
    case QIR_NODE_U128_TY:
    case QIR_NODE_I8_TY:
    case QIR_NODE_I16_TY:
    case QIR_NODE_I32_TY:
    case QIR_NODE_I64_TY:
    case QIR_NODE_I128_TY:
    case QIR_NODE_F16_TY:
    case QIR_NODE_F32_TY:
    case QIR_NODE_F64_TY:
    case QIR_NODE_F128_TY:
    case QIR_NODE_VOID_TY:
      return true;
    default:
      return false;
  }
}

CPP_EXPORT bool Type::is_array() const {
  return getKind() == QIR_NODE_ARRAY_TY;
}

CPP_EXPORT bool Type::is_pointer() const {
  return getKind() == QIR_NODE_PTR_TY;
}

CPP_EXPORT bool Type::is_function() const {
  return getKind() == QIR_NODE_FN_TY;
}

CPP_EXPORT bool Type::is_composite() const {
  switch (getKind()) {
    case QIR_NODE_STRUCT_TY:
    case QIR_NODE_UNION_TY:
    case QIR_NODE_ARRAY_TY:
      return true;
    default:
      return false;
  }
}

CPP_EXPORT bool Type::is_union() const {
  return getKind() == QIR_NODE_UNION_TY;
}

CPP_EXPORT bool Type::is_numeric() const {
  switch (getKind()) {
    case QIR_NODE_U1_TY:
    case QIR_NODE_U8_TY:
    case QIR_NODE_U16_TY:
    case QIR_NODE_U32_TY:
    case QIR_NODE_U64_TY:
    case QIR_NODE_U128_TY:
    case QIR_NODE_I8_TY:
    case QIR_NODE_I16_TY:
    case QIR_NODE_I32_TY:
    case QIR_NODE_I64_TY:
    case QIR_NODE_I128_TY:
    case QIR_NODE_F16_TY:
    case QIR_NODE_F32_TY:
    case QIR_NODE_F64_TY:
    case QIR_NODE_F128_TY:
      return true;
    default:
      return false;
  }
}

CPP_EXPORT bool Type::is_integral() const {
  switch (getKind()) {
    case QIR_NODE_U1_TY:
    case QIR_NODE_U8_TY:
    case QIR_NODE_U16_TY:
    case QIR_NODE_U32_TY:
    case QIR_NODE_U64_TY:
    case QIR_NODE_U128_TY:
    case QIR_NODE_I8_TY:
    case QIR_NODE_I16_TY:
    case QIR_NODE_I32_TY:
    case QIR_NODE_I64_TY:
    case QIR_NODE_I128_TY:
      return true;
    default:
      return false;
  }
}

CPP_EXPORT bool Type::is_floating_point() const {
  switch (getKind()) {
    case QIR_NODE_F16_TY:
    case QIR_NODE_F32_TY:
    case QIR_NODE_F64_TY:
    case QIR_NODE_F128_TY:
      return true;
    default:
      return false;
  }
}

CPP_EXPORT bool Type::is_signed() const {
  switch (getKind()) {
    case QIR_NODE_I8_TY:
    case QIR_NODE_I16_TY:
    case QIR_NODE_I32_TY:
    case QIR_NODE_I64_TY:
    case QIR_NODE_I128_TY:
    case QIR_NODE_F16_TY:
    case QIR_NODE_F32_TY:
    case QIR_NODE_F64_TY:
    case QIR_NODE_F128_TY:
      return true;
    default:
      return false;
  }
}

CPP_EXPORT bool Type::is_unsigned() const {
  switch (getKind()) {
    case QIR_NODE_U1_TY:
    case QIR_NODE_U8_TY:
    case QIR_NODE_U16_TY:
    case QIR_NODE_U32_TY:
    case QIR_NODE_U64_TY:
    case QIR_NODE_U128_TY:
      return true;
    default:
      return false;
  }
}

CPP_EXPORT bool Type::is_void() const { return getKind() == QIR_NODE_VOID_TY; }

CPP_EXPORT bool Type::is_bool() const { return getKind() == QIR_NODE_U1_TY; }

CPP_EXPORT bool Type::is_ptr_to(const Type *type) const {
  if (is_pointer()) {
    return as<PtrTy>()->m_pointee->isSame(type);
  }

  return false;
}

///=============================================================================

CPP_EXPORT uint128_t Int::str2u128(std::string_view s) noexcept {
  uint128_t x = 0;

  for (char c : s) {
    if (!std::isdigit(c)) {
      qcore_panicf("Failed to convert string `%s` to uint128_t", s.data());
    }

    // Check for overflow
    if (x > (std::numeric_limits<uint128_t>::max() - (c - '0')) / 10) {
      qcore_panicf("Overflow when converting string `%s` to uint128_t",
                   s.data());
    }

    x = x * 10 + (c - '0');
  }

  return x;
}

CPP_EXPORT std::string Int::getValueString() const noexcept {
  return ((uint128_t)m_value).str();
}

std::unordered_map<uint128_t, Int *> Int::m_cache;
static std::mutex m_cache_mtx;

CPP_EXPORT Int *Int::get(uint128_t val, uint8_t size) noexcept {
  std::lock_guard<std::mutex> lock(m_cache_mtx);

  auto it = m_cache.find(val);
  if (it != m_cache.end()) [[likely]] {
    return it->second;
  }

  return m_cache[val] = new Int(val, size);
}

///=============================================================================

Expr *nr::createIgn() {
  return new (Arena<Expr>().allocate(1)) Expr(QIR_NODE_IGN);
}
