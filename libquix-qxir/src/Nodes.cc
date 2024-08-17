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
#define __QXIR_NODE_REFLECT_IMPL__  // Make private fields accessible

#include <LibMacro.h>
#include <openssl/evp.h>
#include <quix-core/Error.h>
#include <quix-qxir/Node.h>
#include <quix-qxir/QXIR.h>

#include <boost/uuid/name_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <cstring>

using namespace qxir;

///=============================================================================
namespace qxir {
  thread_local ArenaAllocatorImpl qxir_arena;
}

ArenaAllocatorImpl::ArenaAllocatorImpl() { qcore_arena_open(&m_arena); }

ArenaAllocatorImpl::~ArenaAllocatorImpl() { qcore_arena_close(&m_arena); }

void *ArenaAllocatorImpl::allocate(std::size_t size) {
  const std::size_t alignment = 16;
  return qcore_arena_alloc_ex(&m_arena, size, alignment);
}

void ArenaAllocatorImpl::deallocate(void *ptr) noexcept { (void)ptr; }

///=============================================================================

CPP_EXPORT uint32_t Expr::thisSizeOf() const noexcept {
#define SIZEOF_ROW(__type) \
  { typeid(__type).hash_code(), sizeof(__type) }

  static const std::unordered_map<size_t, uint32_t> sizes = {
      SIZEOF_ROW(BinExpr),  SIZEOF_ROW(UnExpr),   SIZEOF_ROW(PostUnExpr), SIZEOF_ROW(Int),
      SIZEOF_ROW(Float),    SIZEOF_ROW(String),   SIZEOF_ROW(List),       SIZEOF_ROW(Alloc),
      SIZEOF_ROW(Call),     SIZEOF_ROW(Seq),      SIZEOF_ROW(Async),      SIZEOF_ROW(Index),
      SIZEOF_ROW(Ident),    SIZEOF_ROW(Global),   SIZEOF_ROW(Ret),        SIZEOF_ROW(Brk),
      SIZEOF_ROW(Cont),     SIZEOF_ROW(If),       SIZEOF_ROW(While),      SIZEOF_ROW(For),
      SIZEOF_ROW(Form),     SIZEOF_ROW(Foreach),  SIZEOF_ROW(Case),       SIZEOF_ROW(Switch),
      SIZEOF_ROW(Fn),       SIZEOF_ROW(Asm),      SIZEOF_ROW(U1Ty),       SIZEOF_ROW(U8Ty),
      SIZEOF_ROW(U16Ty),    SIZEOF_ROW(U32Ty),    SIZEOF_ROW(U64Ty),      SIZEOF_ROW(U128Ty),
      SIZEOF_ROW(I8Ty),     SIZEOF_ROW(I16Ty),    SIZEOF_ROW(I32Ty),      SIZEOF_ROW(I64Ty),
      SIZEOF_ROW(I128Ty),   SIZEOF_ROW(F16Ty),    SIZEOF_ROW(F32Ty),      SIZEOF_ROW(F64Ty),
      SIZEOF_ROW(F128Ty),   SIZEOF_ROW(VoidTy),   SIZEOF_ROW(PtrTy),      SIZEOF_ROW(OpaqueTy),
      SIZEOF_ROW(StringTy), SIZEOF_ROW(StructTy), SIZEOF_ROW(UnionTy),    SIZEOF_ROW(ArrayTy),
      SIZEOF_ROW(ListTy),   SIZEOF_ROW(IntrinTy), SIZEOF_ROW(FnTy),       SIZEOF_ROW(Tmp),
  };

  qcore_assert(sizes.size() == QIR_NODE_COUNT, "Polymorphic type size lookup table is incomplete");

  size_t id = typeid(*this).hash_code();
  qcore_assert(sizes.contains(id));

  return sizes.at(id);
}

CPP_EXPORT qxir_ty_t Expr::getKind() const noexcept { return m_node_type; }

CPP_EXPORT const char *Expr::thisTypeName() const noexcept {
#define NAMEOF_ROW(__name) \
  { QIR_NODE_##__name, "QIR_NODE_" #__name }

  static const std::unordered_map<qxir_ty_t, const char *> names = {
      NAMEOF_ROW(BINEXPR),   NAMEOF_ROW(UNEXPR),    NAMEOF_ROW(POST_UNEXPR), NAMEOF_ROW(INT),
      NAMEOF_ROW(FLOAT),     NAMEOF_ROW(STRING),    NAMEOF_ROW(LIST),        NAMEOF_ROW(ALLOC),
      NAMEOF_ROW(CALL),      NAMEOF_ROW(SEQ),       NAMEOF_ROW(ASYNC),       NAMEOF_ROW(INDEX),
      NAMEOF_ROW(IDENT),     NAMEOF_ROW(GLOBAL),    NAMEOF_ROW(RET),         NAMEOF_ROW(BRK),
      NAMEOF_ROW(CONT),      NAMEOF_ROW(IF),        NAMEOF_ROW(WHILE),       NAMEOF_ROW(FOR),
      NAMEOF_ROW(FORM),      NAMEOF_ROW(FOREACH),   NAMEOF_ROW(CASE),        NAMEOF_ROW(SWITCH),
      NAMEOF_ROW(FN),        NAMEOF_ROW(ASM),       NAMEOF_ROW(U1_TY),       NAMEOF_ROW(U8_TY),
      NAMEOF_ROW(U16_TY),    NAMEOF_ROW(U32_TY),    NAMEOF_ROW(U64_TY),      NAMEOF_ROW(U128_TY),
      NAMEOF_ROW(I8_TY),     NAMEOF_ROW(I16_TY),    NAMEOF_ROW(I32_TY),      NAMEOF_ROW(I64_TY),
      NAMEOF_ROW(I128_TY),   NAMEOF_ROW(F16_TY),    NAMEOF_ROW(F32_TY),      NAMEOF_ROW(F64_TY),
      NAMEOF_ROW(F128_TY),   NAMEOF_ROW(VOID_TY),   NAMEOF_ROW(PTR_TY),      NAMEOF_ROW(OPAQUE_TY),
      NAMEOF_ROW(STRING_TY), NAMEOF_ROW(STRUCT_TY), NAMEOF_ROW(UNION_TY),    NAMEOF_ROW(ARRAY_TY),
      NAMEOF_ROW(LIST_TY),   NAMEOF_ROW(INTRIN_TY), NAMEOF_ROW(FN_TY),       NAMEOF_ROW(TMP),
  };

  qxir_ty_t type = getKind();

  qcore_assert(names.size() == QIR_NODE_COUNT, "Polymorphic type size lookup table is incomplete");
  qcore_assert(names.contains(type));

  return names.at(type);
}

CPP_EXPORT bool Expr::isType() const noexcept {
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
    case QIR_NODE_PTR_TY:
    case QIR_NODE_OPAQUE_TY:
    case QIR_NODE_STRING_TY:
    case QIR_NODE_STRUCT_TY:
    case QIR_NODE_UNION_TY:
    case QIR_NODE_ARRAY_TY:
    case QIR_NODE_LIST_TY:
    case QIR_NODE_INTRIN_TY:
    case QIR_NODE_FN_TY:
      return true;
    default:
      return false;
  }
}

CPP_EXPORT qxir::Type *qxir::Expr::getType() noexcept {
  return getModule()->lookupType(m_type_idx);
}

CPP_EXPORT bool Expr::is(qxir_ty_t type) const noexcept { return type == getKind(); }

CPP_EXPORT void qxir::Expr::dump(std::ostream &os, bool isForDebug) const {
  (void)isForDebug;

  size_t outlen;
  char *repr = qxir_repr(this, false, 2, nullptr, &outlen);
  if (!repr) {
    qcore_panic("Failed to dump expression");
  }

  os << repr;

  free(repr);
}

CPP_EXPORT boost::uuids::uuid qxir::Expr::hash() noexcept {
  const EVP_MD *md = EVP_sha256();
  std::array<uint8_t, EVP_MAX_MD_SIZE> hash;

  EVP_MD_CTX *ctx = EVP_MD_CTX_new();
  if (!ctx) {
    qcore_panic("Failed to create EVP_MD_CTX");
  }

  if (EVP_DigestInit(ctx, md) != 1) {
    qcore_panic("Failed to initialize EVP_MD_CTX");
  }

  iterate<IterMode::dfs_pre>(this, [ctx](Expr *, Expr *cur) -> IterOp {
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
        MIXIN_STRING(cur->as<Int>()->getValue());
        break;
      }
      case QIR_NODE_FLOAT: {
        MIXIN_STRING(cur->as<Float>()->getValue());
        break;
      }
      case QIR_NODE_STRING: {
        MIXIN_STRING(cur->as<String>()->m_data);
        break;
      }
      case QIR_NODE_LIST: {
        break;
      }
      case QIR_NODE_ALLOC: {
        break;
      }
      case QIR_NODE_CALL: {
        break;
      }
      case QIR_NODE_SEQ: {
        break;
      }
      case QIR_NODE_ASYNC: {
        break;
      }
      case QIR_NODE_INDEX: {
        break;
      }
      case QIR_NODE_IDENT: {
        MIXIN_STRING(cur->as<Ident>()->m_name);
        break;
      }
      case QIR_NODE_GLOBAL: {
        MIXIN_STRING(cur->as<Global>()->m_name);
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
      case QIR_NODE_FORM: {
        MIXIN_STRING(cur->as<Form>()->m_idx_ident);
        MIXIN_STRING(cur->as<Form>()->m_val_ident);
        break;
      }
      case QIR_NODE_FOREACH: {
        MIXIN_STRING(cur->as<Foreach>()->m_idx_ident);
        MIXIN_STRING(cur->as<Foreach>()->m_val_ident);
        break;
      }
      case QIR_NODE_CASE: {
        break;
      }
      case QIR_NODE_SWITCH: {
        break;
      }
      case QIR_NODE_FN: {
        MIXIN_STRING(cur->as<Fn>()->m_name);
        break;
      }
      case QIR_NODE_ASM: {
        qcore_implement("QIR_NODE_ASM node hashing");
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
      case QIR_NODE_STRING_TY: {
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
      case QIR_NODE_LIST_TY: {
        break;
      }
      case QIR_NODE_INTRIN_TY: {
        MIXIN_STRING(cur->as<IntrinTy>()->m_name);
        break;
      }
      case QIR_NODE_FN_TY: {
        std::set<FnTag> tags;
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
        /// TODO: Mixin arbitrary data in std::any
        break;
      }
      case QIR_NODE_BAD: {
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
  return gen("qxir");
}

CPP_EXPORT std::string qxir::Expr::getUniqueId() noexcept {
  return boost::uuids::to_string(hash());
}

CPP_EXPORT qxir::Module *qxir::Expr::getModule() noexcept { return ::getModule(m_module_idx); }
