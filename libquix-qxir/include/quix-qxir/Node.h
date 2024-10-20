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

#ifndef __QUIX_QXIR_NODE_H__
#define __QUIX_QXIR_NODE_H__

#include <quix-qxir/TypeDecl.h>

#ifdef __cplusplus
extern "C" {
#endif

#define QIR_NODE_COUNT 46

/**
 * @brief Clone a QXIR node. Optionally into a different module.
 *
 * @param dst The destination module context or NULL to clone into the same context.
 * @param node The node to clone.
 *
 * @return qxir_node_t* The cloned node.
 *
 * @note If `dst` NULL, the function will clone into the same module.
 * @note If `node` NULL, the function will return NULL.
 * @note This clone is a deep copy.
 *
 * @note Be nice to this function, it does more than you think.
 */
qxir_node_t *qxir_clone(qmodule_t *dst, const qxir_node_t *node);

#ifdef __cplusplus
}
#endif

///=============================================================================
/// END: ABSTRACT SYNTAX TREE DATA TYPES
///=============================================================================

#if (defined(__cplusplus) && defined(QXIR_USE_CPP_API)) || defined(__QUIX_IMPL__)

#include <quix-core/Arena.h>
#include <quix-core/Error.h>
#include <quix-lexer/Token.h>
#include <quix-qxir/Module.h>
#include <quix-qxir/TypeDecl.h>

#include <boost/uuid/uuid.hpp>
#include <cassert>
#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <optional>
#include <ostream>
#include <quix-core/Classes.hh>
#include <string>
#include <variant>
#include <vector>
#include <charconv>

namespace qxir {
  class ArenaAllocatorImpl {
    qcore_arena m_arena;

  public:
    ArenaAllocatorImpl() = default;

    void *allocate(std::size_t bytes);
    void deallocate(void *ptr) noexcept;

    qcore_arena_t &get() { return *m_arena.get(); }
  };

  extern "C" thread_local ArenaAllocatorImpl qxir_arena;

  template <class T>
  struct Arena {
    typedef T value_type;

    Arena() = default;

    template <class U>
    constexpr Arena(const Arena<U> &) noexcept {}

    [[nodiscard]] T *allocate(std::size_t n) {
      return static_cast<T *>(qxir_arena.allocate(sizeof(T) * n));
    }

    void deallocate(T *p, std::size_t n) noexcept {
      (void)n;
      (void)p;
    }
  };

  template <class T, class U>
  bool operator==(const Arena<T> &, const Arena<U> &) {
    return true;
  }
  template <class T, class U>
  bool operator!=(const Arena<T> &, const Arena<U> &) {
    return false;
  }
};  // namespace qxir

struct qxir_node_t {
public:
  qxir_node_t() = default;
};

namespace qxir {

#ifdef __QXIR_NODE_REFLECT_IMPL__
#define QCLASS_REFLECT() public:
#else
#define QCLASS_REFLECT() private:
#endif

  class Expr : public qxir_node_t {
    QCLASS_REFLECT()

    qxir_ty_t m_node_type : 6;        /* Typecode of this node. */
    qxir::ModuleId m_module_idx : 16; /* The module context index. */
    uint64_t m_constexpr : 1;         /* Is this expression a constant expression? */
    uint64_t m_mutable : 1;           /* Is this expression mutable? */

    qlex_loc_t m_start_loc;
    uint16_t m_loc_size;  // Diagnostics can not span more than 64K bytes.

    Expr(const Expr &) = delete;
    Expr &operator=(const Expr &) = delete;

  public:
    Expr(qxir_ty_t ty)
        : m_node_type(ty),
          m_module_idx(std::numeric_limits<ModuleId>::max()),
          m_constexpr(0),
          m_mutable(1),
          m_start_loc{0},
          m_loc_size(0) {}

    uint32_t getKindSize() const noexcept;
    qxir_ty_t getKind() const noexcept { return m_node_type; }
    const char *getKindName() const noexcept;

    template <typename T>
    constexpr static const char *getKindName() noexcept {
      if constexpr (std::is_same_v<T, BinExpr>) {
        return "BinExpr";
      } else if constexpr (std::is_same_v<T, UnExpr>) {
        return "UnExpr";
      } else if constexpr (std::is_same_v<T, PostUnExpr>) {
        return "PostUnExpr";
      } else if constexpr (std::is_same_v<T, Int>) {
        return "Int";
      } else if constexpr (std::is_same_v<T, Float>) {
        return "Float";
      } else if constexpr (std::is_same_v<T, List>) {
        return "List";
      } else if constexpr (std::is_same_v<T, Call>) {
        return "Call";
      } else if constexpr (std::is_same_v<T, Seq>) {
        return "Seq";
      } else if constexpr (std::is_same_v<T, Index>) {
        return "Index";
      } else if constexpr (std::is_same_v<T, Ident>) {
        return "Ident";
      } else if constexpr (std::is_same_v<T, Extern>) {
        return "Extern";
      } else if constexpr (std::is_same_v<T, Local>) {
        return "Local";
      } else if constexpr (std::is_same_v<T, Ret>) {
        return "Ret";
      } else if constexpr (std::is_same_v<T, Brk>) {
        return "Brk";
      } else if constexpr (std::is_same_v<T, Cont>) {
        return "Cont";
      } else if constexpr (std::is_same_v<T, If>) {
        return "If";
      } else if constexpr (std::is_same_v<T, While>) {
        return "While";
      } else if constexpr (std::is_same_v<T, For>) {
        return "For";
      } else if constexpr (std::is_same_v<T, Form>) {
        return "Form";
      } else if constexpr (std::is_same_v<T, Case>) {
        return "Case";
      } else if constexpr (std::is_same_v<T, Switch>) {
        return "Switch";
      } else if constexpr (std::is_same_v<T, Fn>) {
        return "Fn";
      } else if constexpr (std::is_same_v<T, Asm>) {
        return "Asm";
      } else if constexpr (std::is_same_v<T, U1Ty>) {
        return "U1Ty";
      } else if constexpr (std::is_same_v<T, U8Ty>) {
        return "U8Ty";
      } else if constexpr (std::is_same_v<T, U16Ty>) {
        return "U16Ty";
      } else if constexpr (std::is_same_v<T, U32Ty>) {
        return "U32Ty";
      } else if constexpr (std::is_same_v<T, U64Ty>) {
        return "U64Ty";
      } else if constexpr (std::is_same_v<T, U128Ty>) {
        return "U128Ty";
      } else if constexpr (std::is_same_v<T, I8Ty>) {
        return "I8Ty";
      } else if constexpr (std::is_same_v<T, I16Ty>) {
        return "I16Ty";
      } else if constexpr (std::is_same_v<T, I32Ty>) {
        return "I32Ty";
      } else if constexpr (std::is_same_v<T, I64Ty>) {
        return "I64Ty";
      } else if constexpr (std::is_same_v<T, I128Ty>) {
        return "I128Ty";
      } else if constexpr (std::is_same_v<T, F16Ty>) {
        return "F16Ty";
      } else if constexpr (std::is_same_v<T, F32Ty>) {
        return "F32Ty";
      } else if constexpr (std::is_same_v<T, F64Ty>) {
        return "F64Ty";
      } else if constexpr (std::is_same_v<T, F128Ty>) {
        return "F128Ty";
      } else if constexpr (std::is_same_v<T, VoidTy>) {
        return "VoidTy";
      } else if constexpr (std::is_same_v<T, PtrTy>) {
        return "PtrTy";
      } else if constexpr (std::is_same_v<T, OpaqueTy>) {
        return "OpaqueTy";
      } else if constexpr (std::is_same_v<T, StructTy>) {
        return "StructTy";
      } else if constexpr (std::is_same_v<T, UnionTy>) {
        return "UnionTy";
      } else if constexpr (std::is_same_v<T, ArrayTy>) {
        return "ArrayTy";
      } else if constexpr (std::is_same_v<T, FnTy>) {
        return "FnTy";
      } else if constexpr (std::is_same_v<T, Tmp>) {
        return "Tmp";
      } else {
        static_assert(!std::is_same_v<T, T>,
                      "The requested type target is not supported by this function.");
      }
    }

    bool isType() const noexcept;
    inline bool isConstExpr() const noexcept { return m_constexpr; }
    inline bool isMutable() const noexcept { return m_mutable; }
    inline void setConstExpr(bool is_const) noexcept { m_constexpr = is_const; }
    inline void setMutable(bool is_mut) noexcept { m_mutable = is_mut; }
    inline bool isLiteral() const noexcept {
      return m_node_type == QIR_NODE_INT || m_node_type == QIR_NODE_FLOAT;
    }

    // Returns "" if the construct is not named.
    std::string_view getName() const noexcept;

    std::pair<qlex_loc_t, qlex_loc_t> getLoc() const noexcept;
    qlex_loc_t locBeg() const noexcept;
    qlex_loc_t locEnd() const noexcept;
    void setLoc(std::pair<qlex_loc_t, qlex_loc_t> loc) noexcept;

    qmodule_t *getModule() const noexcept;
    void setModule(qmodule_t *module) noexcept;

    Type *getType() noexcept;

    template <typename T>
    static T *safe_cast_as(Expr *ptr) noexcept {
#ifndef NDEBUG
#define is(_T) (std::is_same_v<T, _T>)
      static_assert(is(BinExpr) || is(UnExpr) || is(PostUnExpr) || is(Int) || is(Float) ||
                        is(List) || is(Call) || is(Seq) || is(Index) || is(Ident) || is(Extern) ||
                        is(Local) || is(Ret) || is(Brk) || is(Cont) || is(If) || is(While) ||
                        is(For) || is(Form) || is(Case) || is(Switch) || is(Fn) || is(Asm) ||
                        is(U1Ty) || is(U8Ty) || is(U16Ty) || is(U32Ty) || is(U64Ty) || is(U128Ty) ||
                        is(I8Ty) || is(I16Ty) || is(I32Ty) || is(I64Ty) || is(I128Ty) ||
                        is(F16Ty) || is(F32Ty) || is(F64Ty) || is(F128Ty) || is(VoidTy) ||
                        is(PtrTy) || is(OpaqueTy) || is(StructTy) || is(UnionTy) || is(ArrayTy) ||
                        is(FnTy) || is(Tmp),
                    "The requested type target is not supported by this function.");
#undef is
#endif

      if (!ptr) {
        return nullptr;
      }

#ifndef NDEBUG
      switch (ptr->getKind()) {
        case QIR_NODE_BINEXPR: {
          if constexpr (!std::is_same_v<T, BinExpr>) goto cast_panic;
          break;
        }
        case QIR_NODE_UNEXPR: {
          if constexpr (!std::is_same_v<T, UnExpr>) goto cast_panic;
          break;
        }
        case QIR_NODE_POST_UNEXPR: {
          if constexpr (!std::is_same_v<T, PostUnExpr>) goto cast_panic;
          break;
        }
        case QIR_NODE_INT: {
          if constexpr (!std::is_same_v<T, Int>) goto cast_panic;
          break;
        }
        case QIR_NODE_FLOAT: {
          if constexpr (!std::is_same_v<T, Float>) goto cast_panic;
          break;
        }
        case QIR_NODE_LIST: {
          if constexpr (!std::is_same_v<T, List>) goto cast_panic;
          break;
        }
        case QIR_NODE_CALL: {
          if constexpr (!std::is_same_v<T, Call>) goto cast_panic;
          break;
        }
        case QIR_NODE_SEQ: {
          if constexpr (!std::is_same_v<T, Seq>) goto cast_panic;
          break;
        }
        case QIR_NODE_INDEX: {
          if constexpr (!std::is_same_v<T, Index>) goto cast_panic;
          break;
        }
        case QIR_NODE_IDENT: {
          if constexpr (!std::is_same_v<T, Ident>) goto cast_panic;
          break;
        }
        case QIR_NODE_EXTERN: {
          if constexpr (!std::is_same_v<T, Extern>) goto cast_panic;
          break;
        }
        case QIR_NODE_LOCAL: {
          if constexpr (!std::is_same_v<T, Local>) goto cast_panic;
          break;
        }
        case QIR_NODE_RET: {
          if constexpr (!std::is_same_v<T, Ret>) goto cast_panic;
          break;
        }
        case QIR_NODE_BRK: {
          if constexpr (!std::is_same_v<T, Brk>) goto cast_panic;
          break;
        }
        case QIR_NODE_CONT: {
          if constexpr (!std::is_same_v<T, Cont>) goto cast_panic;
          break;
        }
        case QIR_NODE_IF: {
          if constexpr (!std::is_same_v<T, If>) goto cast_panic;
          break;
        }
        case QIR_NODE_WHILE: {
          if constexpr (!std::is_same_v<T, While>) goto cast_panic;
          break;
        }
        case QIR_NODE_FOR: {
          if constexpr (!std::is_same_v<T, For>) goto cast_panic;
          break;
        }
        case QIR_NODE_FORM: {
          if constexpr (!std::is_same_v<T, Form>) goto cast_panic;
          break;
        }
        case QIR_NODE_CASE: {
          if constexpr (!std::is_same_v<T, Case>) goto cast_panic;
          break;
        }
        case QIR_NODE_SWITCH: {
          if constexpr (!std::is_same_v<T, Switch>) goto cast_panic;
          break;
        }
        case QIR_NODE_FN: {
          if constexpr (!std::is_same_v<T, Fn>) goto cast_panic;
          break;
        }
        case QIR_NODE_ASM: {
          if constexpr (!std::is_same_v<T, Asm>) goto cast_panic;
          break;
        }
        case QIR_NODE_U1_TY: {
          if constexpr (!std::is_same_v<T, U1Ty>) goto cast_panic;
          break;
        }
        case QIR_NODE_U8_TY: {
          if constexpr (!std::is_same_v<T, U8Ty>) goto cast_panic;
          break;
        }
        case QIR_NODE_U16_TY: {
          if constexpr (!std::is_same_v<T, U16Ty>) goto cast_panic;
          break;
        }
        case QIR_NODE_U32_TY: {
          if constexpr (!std::is_same_v<T, U32Ty>) goto cast_panic;
          break;
        }
        case QIR_NODE_U64_TY: {
          if constexpr (!std::is_same_v<T, U64Ty>) goto cast_panic;
          break;
        }
        case QIR_NODE_U128_TY: {
          if constexpr (!std::is_same_v<T, U128Ty>) goto cast_panic;
          break;
        }
        case QIR_NODE_I8_TY: {
          if constexpr (!std::is_same_v<T, I8Ty>) goto cast_panic;
          break;
        }
        case QIR_NODE_I16_TY: {
          if constexpr (!std::is_same_v<T, I16Ty>) goto cast_panic;
          break;
        }
        case QIR_NODE_I32_TY: {
          if constexpr (!std::is_same_v<T, I32Ty>) goto cast_panic;
          break;
        }
        case QIR_NODE_I64_TY: {
          if constexpr (!std::is_same_v<T, I64Ty>) goto cast_panic;
          break;
        }
        case QIR_NODE_I128_TY: {
          if constexpr (!std::is_same_v<T, I128Ty>) goto cast_panic;
          break;
        }
        case QIR_NODE_F16_TY: {
          if constexpr (!std::is_same_v<T, F16Ty>) goto cast_panic;
          break;
        }
        case QIR_NODE_F32_TY: {
          if constexpr (!std::is_same_v<T, F32Ty>) goto cast_panic;
          break;
        }
        case QIR_NODE_F64_TY: {
          if constexpr (!std::is_same_v<T, F64Ty>) goto cast_panic;
          break;
        }
        case QIR_NODE_F128_TY: {
          if constexpr (!std::is_same_v<T, F128Ty>) goto cast_panic;
          break;
        }
        case QIR_NODE_VOID_TY: {
          if constexpr (!std::is_same_v<T, VoidTy>) goto cast_panic;
          break;
        }
        case QIR_NODE_PTR_TY: {
          if constexpr (!std::is_same_v<T, PtrTy>) goto cast_panic;
          break;
        }
        case QIR_NODE_OPAQUE_TY: {
          if constexpr (!std::is_same_v<T, OpaqueTy>) goto cast_panic;
          break;
        }
        case QIR_NODE_STRUCT_TY: {
          if constexpr (!std::is_same_v<T, StructTy>) goto cast_panic;
          break;
        }
        case QIR_NODE_UNION_TY: {
          if constexpr (!std::is_same_v<T, UnionTy>) goto cast_panic;
          break;
        }
        case QIR_NODE_ARRAY_TY: {
          if constexpr (!std::is_same_v<T, ArrayTy>) goto cast_panic;
          break;
        }
        case QIR_NODE_FN_TY: {
          if constexpr (!std::is_same_v<T, FnTy>) goto cast_panic;
          break;
        }

#ifdef __QUIX_IMPL__
        case QIR_NODE_TMP: {
          if constexpr (!std::is_same_v<T, Tmp>) goto cast_panic;
          break;
        }
#endif
      }
#endif

      return static_cast<T *>(ptr);

#ifndef NDEBUG
    cast_panic:
      qcore_panicf("Invalid cast from %s to %s", ptr->getKindName(), Expr::getKindName<T>());
#endif
    }

    /**
     * @brief Type-safe cast (type check only in debug mode).
     *
     * @tparam T The type to cast to.
     * @return T* The casted pointer. It may be nullptr if the source pointer is nullptr.
     * @warning This function will panic if the cast is invalid.
     */
    template <typename T>
    T *as() noexcept {
      return safe_cast_as<T>(this);
    }

    /**
     * @brief Type-safe cast (type check only in debug mode).
     *
     * @tparam T The type to cast to.
     * @return const T* The casted pointer. It may be nullptr if the source pointer is nullptr.
     * @warning This function will panic if the cast is invalid.
     */
    template <typename T>
    const T *as() const noexcept {
      return safe_cast_as<T>(const_cast<Expr *>(this));
    }

    Expr *asExpr() noexcept { return this; }
    Type *asType() noexcept;

    /**
     * @brief Type check.
     *
     * @param type The type to check.
     * @return true If the type matches.
     * @return false If the type does not match.
     */
    bool is(qxir_ty_t type) const noexcept;

    /**
     * @brief Compare two nodes for equality.
     * @param other The other node.
     * @return true If the nodes are equivalent (deep comparison).
     * @note This compare will be insensitive to metadata like module, source location, etc.
     */
    bool cmp_eq(const Expr *other) const;

    bool is_acyclic() const noexcept;

    /**
     * @brief Print the node to the output stream.
     * @param os The output stream.
     * @param isForDebug Whether to print the node for debugging.
     */
    void dump(std::ostream &os = std::cout, bool isForDebug = false) const;

    /**
     * @brief Compute the hash of the node.
     * @return boost::uuids::uuid The hash.
     * @note This code will be the same on different compiler runs as long as the compiler version
     * is the same.
     */
    boost::uuids::uuid hash() noexcept;

    /**
     * @brief Get a unique identifier for the node.
     * @return std::string The unique identifier.
     * @note Wrapper around hash()
     */
    std::string getUniqueUUID() noexcept;

    /**
     * @brief Get a short code to uniquely identify the node.
     * @return uint64_t The unique identifier.
     * @note This code may be different for different compiler runs.
     */
    uint64_t getUniqId() const;
  } __attribute__((packed)) __attribute__((aligned(8)));

#define EXPR_SIZE sizeof(Expr)

  class Type : public Expr {
    uint64_t getAlignBits();

  public:
    Type(qxir_ty_t ty) : Expr(ty) { setConstExpr(true); }

    bool hasKnownSize() noexcept;
    bool hasKnownAlign() noexcept;
    uint64_t getSizeBits();
    inline uint64_t getSizeBytes() { return std::ceil(getSizeBits() / 8.0); }
    inline uint64_t getAlignBytes() { return std::ceil(getAlignBits() / 8.0); }

    bool is_primitive() const;
    bool is_array() const;
    bool is_pointer() const;
    bool is_function() const;
    bool is_composite() const;
    bool is_union() const;
    bool is_numeric() const;
    bool is_integral() const;
    bool is_floating_point() const;
    bool is_signed() const;
    bool is_unsigned() const;
    bool is_void() const;
    bool is_bool() const;
    bool is_ptr_to(const Type *type) const;
  };

  ///=============================================================================
  /// BEGIN: EXPRESSIONS CATEGORIES
  ///=============================================================================

  enum class Op {
    Plus,      /* '+': Addition operator */
    Minus,     /* '-': Subtraction operator */
    Times,     /* '*': Multiplication operator */
    Slash,     /* '/': Division operator */
    Percent,   /* '%': Modulus operator */
    BitAnd,    /* '&': Bitwise AND operator */
    BitOr,     /* '|': Bitwise OR operator */
    BitXor,    /* '^': Bitwise XOR operator */
    BitNot,    /* '~': Bitwise NOT operator */
    LogicAnd,  /* '&&': Logical AND operator */
    LogicOr,   /* '||': Logical OR operator */
    LogicNot,  /* '!': Logical NOT operator */
    LShift,    /* '<<': Left shift operator */
    RShift,    /* '>>': Right shift operator */
    ROTR,      /* '>>>': Rotate right operator */
    ROTL,      /* '<<<': Rotate left operator */
    Inc,       /* '++': Increment operator */
    Dec,       /* '--': Decrement operator */
    Set,       /* '=': Assignment operator */
    LT,        /* '<': Less than operator */
    GT,        /* '>': Greater than operator */
    LE,        /* '<=': Less than or equal to operator */
    GE,        /* '>=': Greater than or equal to operator */
    Eq,        /* '==': Equal to operator */
    NE,        /* '!=': Not equal to operator */
    Alignof,   /* 'alignof': Alignment of operator */
    Typeof,    /* 'typeof': Type of operator */
    BitcastAs, /* 'bitcast_as': Bitcast operator */
    CastAs,    /* 'cast_as': Common operator */
    Bitsizeof, /* 'bitsizeof': Bit size of operator */
  };

  enum class AbiTag {
    C,
    QUIX,
    Internal,
    Default = QUIX,
  };

  std::ostream &operator<<(std::ostream &os, Op op);

  class BinExpr final : public Expr {
    QCLASS_REFLECT()

    Expr *m_lhs;
    Expr *m_rhs;
    Op m_op;

  public:
    BinExpr(Expr *lhs, Expr *rhs, Op op)
        : Expr(QIR_NODE_BINEXPR), m_lhs(lhs), m_rhs(rhs), m_op(op) {
      if (lhs->isConstExpr() && rhs->isConstExpr()) {
        setConstExpr(true);
      }
    }

    Expr *getLHS() noexcept { return m_lhs; }
    Expr *getRHS() noexcept { return m_rhs; }
    Op getOp() noexcept { return m_op; }

    Expr *setLHS(Expr *lhs) noexcept { return m_lhs = lhs; }
    Expr *setRHS(Expr *rhs) noexcept { return m_rhs = rhs; }
    Op setOp(Op op) noexcept { return m_op = op; }
  };

  class UnExpr final : public Expr {
    QCLASS_REFLECT()

    Expr *m_expr;
    Op m_op;

  public:
    UnExpr(Expr *expr, Op op) : Expr(QIR_NODE_UNEXPR), m_expr(expr), m_op(op) {
      setConstExpr(expr->isConstExpr());
    }

    Expr *getExpr() noexcept { return m_expr; }
    Op getOp() noexcept { return m_op; }

    Expr *setExpr(Expr *expr) noexcept { return m_expr = expr; }
    Op setOp(Op op) noexcept { return m_op = op; }
  };

  class PostUnExpr final : public Expr {
    QCLASS_REFLECT()

    Expr *m_expr;
    Op m_op;

  public:
    PostUnExpr(Expr *expr, Op op) : Expr(QIR_NODE_POST_UNEXPR), m_expr(expr), m_op(op) {
      setConstExpr(expr->isConstExpr());
    }

    Expr *getExpr() noexcept { return m_expr; }
    Op getOp() noexcept { return m_op; }

    Expr *setExpr(Expr *expr) noexcept { return m_expr = expr; }
    Op setOp(Op op) noexcept { return m_op = op; }
  };

  ///=============================================================================
  /// END: EXPRESSIONS CATEGORIES
  ///=============================================================================

  /// ===========================================================================
  /// BEGIN: PRIMITIVE TYPES
  /// ===========================================================================

  class U1Ty final : public Type {
    QCLASS_REFLECT()

  public:
    U1Ty() : Type(QIR_NODE_U1_TY) {}
  };

  class U8Ty final : public Type {
    QCLASS_REFLECT()

  public:
    U8Ty() : Type(QIR_NODE_U8_TY) {}
  };

  class U16Ty final : public Type {
    QCLASS_REFLECT()

  public:
    U16Ty() : Type(QIR_NODE_U16_TY) {}
  };

  class U32Ty final : public Type {
    QCLASS_REFLECT()

  public:
    U32Ty() : Type(QIR_NODE_U32_TY) {}
  };

  class U64Ty final : public Type {
    QCLASS_REFLECT()

  public:
    U64Ty() : Type(QIR_NODE_U64_TY) {}
  };

  class U128Ty final : public Type {
    QCLASS_REFLECT()

  public:
    U128Ty() : Type(QIR_NODE_U128_TY) {}
  };

  class I8Ty final : public Type {
    QCLASS_REFLECT()

  public:
    I8Ty() : Type(QIR_NODE_I8_TY) {}
  };

  class I16Ty final : public Type {
    QCLASS_REFLECT()

  public:
    I16Ty() : Type(QIR_NODE_I16_TY){};
  };

  class I32Ty final : public Type {
    QCLASS_REFLECT()

  public:
    I32Ty() : Type(QIR_NODE_I32_TY) {}
  };

  class I64Ty final : public Type {
    QCLASS_REFLECT()

  public:
    I64Ty() : Type(QIR_NODE_I64_TY) {}
  };

  class I128Ty final : public Type {
    QCLASS_REFLECT()

  public:
    I128Ty() : Type(QIR_NODE_I128_TY) {}
  };

  class F16Ty final : public Type {
    QCLASS_REFLECT()

  public:
    F16Ty() : Type(QIR_NODE_F16_TY) {}
  };

  class F32Ty final : public Type {
    QCLASS_REFLECT()

  public:
    F32Ty() : Type(QIR_NODE_F32_TY) {}
  };

  class F64Ty final : public Type {
    QCLASS_REFLECT()

  public:
    F64Ty() : Type(QIR_NODE_F64_TY) {}
  };

  class F128Ty final : public Type {
    QCLASS_REFLECT()

  public:
    F128Ty() : Type(QIR_NODE_F128_TY) {}
  };

  class VoidTy final : public Type {
    QCLASS_REFLECT()

  public:
    VoidTy() : Type(QIR_NODE_VOID_TY) {}
  };

  /// ===========================================================================
  /// END: PRIMITIVE TYPES
  /// ===========================================================================

  /// ===========================================================================
  /// BEGIN: COMPLEX TYPES
  /// ===========================================================================

  class PtrTy final : public Type {
    QCLASS_REFLECT()

    Type *m_pointee;

  public:
    PtrTy(Type *pointee) : Type(QIR_NODE_PTR_TY), m_pointee(pointee) {}

    Type *getPointee() noexcept { return m_pointee; }
  };

  class OpaqueTy final : public Type {
    QCLASS_REFLECT()

    std::string_view m_name;

  public:
    OpaqueTy(std::string_view name) : Type(QIR_NODE_OPAQUE_TY), m_name(name) {}
  };

  typedef std::vector<Type *, Arena<Type *>> StructFields;

  class StructTy final : public Type {
    QCLASS_REFLECT()

    StructFields m_fields;

  public:
    StructTy(const StructFields &fields) : Type(QIR_NODE_STRUCT_TY), m_fields(fields) {}

    const StructFields &getFields() noexcept { return m_fields; }
  };

  typedef std::vector<Type *, Arena<Type *>> UnionFields;

  class UnionTy final : public Type {
    QCLASS_REFLECT()

    UnionFields m_fields;

  public:
    UnionTy(const UnionFields &fields) : Type(QIR_NODE_UNION_TY), m_fields(fields) {}

    const UnionFields &getFields() noexcept { return m_fields; }
  };

  class ArrayTy final : public Type {
    QCLASS_REFLECT()

    Type *m_element;
    size_t m_size;

  public:
    ArrayTy(Type *element, size_t size)
        : Type(QIR_NODE_ARRAY_TY), m_element(element), m_size(size) {}

    Type *getElement() noexcept { return m_element; }
    size_t getCount() { return m_size; }
  };

  enum class FnAttr {
    Variadic,
  };

  typedef std::vector<Type *, Arena<Type *>> FnParams;
  typedef std::unordered_set<FnAttr, std::hash<FnAttr>, std::equal_to<FnAttr>, Arena<FnAttr>>
      FnAttrs;

  class FnTy final : public Type {
    QCLASS_REFLECT()

    FnParams m_params;
    FnAttrs m_attrs;
    Type *m_return;

  public:
    FnTy(const FnParams &params, Type *ret, const FnAttrs &attrs)
        : Type(QIR_NODE_FN_TY), m_params(params), m_attrs(attrs), m_return(ret) {}

    const FnParams &getParams() noexcept { return m_params; }
    Type *getReturn() noexcept { return m_return; }
    const FnAttrs &getAttrs() noexcept { return m_attrs; }
  };

  ///=============================================================================
  /// END: COMPLEX TYPES
  ///=============================================================================

  ///=============================================================================
  /// BEGIN: LITERALS
  ///=============================================================================

  class Int final : public Expr {
    QCLASS_REFLECT()

    union {
      uint64_t m_u64;    /* bit 63 is 1 always; interpret as if it is 0. */
      const char *m_str; /* bit 63 is always 0, due to addressing on x86_64. */
    } m_data;

    static constexpr uint64_t FLAG_BIT = 1ULL << 63;

  public:
    Int(uint64_t u64) : Expr(QIR_NODE_INT), m_data{.m_u64 = u64 | FLAG_BIT} { setConstExpr(true); }

    Int(std::string_view str) : Expr(QIR_NODE_INT), m_data{.m_str = str.data()} {
      qcore_assert((m_data.m_u64 & FLAG_BIT) == 0,
                   "Optimized code assumed an invariant that does not hold on this architecture.");
      setConstExpr(true);
    }

    bool isNativeRepresentation() const noexcept { return m_data.m_u64 & FLAG_BIT; }

    uint64_t getNativeRepresentation() const noexcept {
      qcore_assert(isNativeRepresentation());
      return m_data.m_u64 & ~FLAG_BIT;
    }

    std::string_view getStringRepresentation() const noexcept {
      qcore_assert(!isNativeRepresentation());
      return m_data.m_str;
    }

    std::string getValue() const noexcept {
      return isNativeRepresentation() ? std::to_string(getNativeRepresentation())
                                      : std::string(getStringRepresentation());
    }

    void setValue(uint64_t u64) noexcept { m_data.m_u64 = u64 | FLAG_BIT; }
    void setValue(std::string_view str) noexcept {
      m_data.m_str = str.data();
      qcore_assert((m_data.m_u64 & FLAG_BIT) == 0,
                   "Optimized code assumed an invariant that does not hold on this architecture.");
    }
  };

  class Float final : public Expr {
    QCLASS_REFLECT()

    std::variant<double, const char *> m_data;

    static_assert(sizeof(double) == 8);

  public:
    Float(double f64) : Expr(QIR_NODE_FLOAT), m_data{f64} { setConstExpr(true); }
    Float(std::string_view str) : Expr(QIR_NODE_FLOAT), m_data{str.data()} { setConstExpr(true); }

    bool isNativeRepresentation() const noexcept { return std::holds_alternative<double>(m_data); }

    double getNativeRepresentation() const noexcept {
      qcore_assert(isNativeRepresentation());
      return std::get<double>(m_data);
    }

    std::string_view getStringRepresentation() const noexcept {
      qcore_assert(!isNativeRepresentation());
      return std::get<const char *>(m_data);
    }

    std::string getValue() const noexcept {
      return isNativeRepresentation() ? std::to_string(getNativeRepresentation())
                                      : std::string(getStringRepresentation());
    }

    void setValue(double f64) noexcept { m_data = f64; }
    void setValue(std::string_view str) noexcept { m_data = str.data(); }
  };

  typedef std::vector<Expr *, Arena<Expr *>> ListItems;

  class List final : public Expr {
    QCLASS_REFLECT()

    ListItems m_items;

  public:
    List(const ListItems &items) : Expr(QIR_NODE_LIST), m_items(items) {
      if (std::all_of(items.begin(), items.end(), [](Expr *item) { return item->isConstExpr(); })) {
        setConstExpr(true);
      }
    }

    const ListItems &getItems() const noexcept { return m_items; }
    ListItems &getItems() noexcept { return m_items; }
    void setItems(const ListItems &items) noexcept { m_items = items; }
    void addItem(Expr *item) noexcept { m_items.push_back(item); }
  };

  ///=============================================================================
  /// END: LITERALS
  ///=============================================================================

  ///=============================================================================
  /// BEGIN: EXPRESSIONS
  ///=============================================================================

  typedef std::vector<Expr *, Arena<Expr *>> CallArgs;

  class Call final : public Expr {
    QCLASS_REFLECT()

    Expr *m_iref; /* Possibly cyclic reference to the target. */
    CallArgs m_args;

  public:
    Call(Expr *ref, const CallArgs &args) : Expr(QIR_NODE_CALL), m_iref(ref), m_args(args) {}

    Expr *getTarget() noexcept { return m_iref; }
    Expr *setTarget(Expr *ref) noexcept { return m_iref = ref; }

    const CallArgs &getArgs() const noexcept { return m_args; }
    CallArgs &getArgs() noexcept { return m_args; }
    void setArgs(const CallArgs &args) noexcept { m_args = args; }

    size_t getNumArgs() noexcept { return m_args.size(); }
  };

  typedef std::vector<Expr *, Arena<Expr *>> SeqItems;

  class Seq final : public Expr {
    QCLASS_REFLECT()

    SeqItems m_items;

  public:
    Seq(const SeqItems &items) : Expr(QIR_NODE_SEQ), m_items(items) {
      if (std::all_of(items.begin(), items.end(), [](Expr *item) { return item->isConstExpr(); })) {
        setConstExpr(true);
      }
    }

    const SeqItems &getItems() const noexcept { return m_items; }
    SeqItems &getItems() noexcept { return m_items; }
    void setItems(const SeqItems &items) noexcept { m_items = items; }
    void addItem(Expr *item) noexcept { m_items.push_back(item); }
  };

  class Index final : public Expr {
    QCLASS_REFLECT()

    Expr *m_expr;
    Expr *m_index;

  public:
    Index(Expr *expr, Expr *index) : Expr(QIR_NODE_INDEX), m_expr(expr), m_index(index) {
      if (expr->isConstExpr() && index->isConstExpr()) {
        setConstExpr(true);
      }
    }

    Expr *getExpr() noexcept { return m_expr; }
    Expr *setExpr(Expr *expr) noexcept { return m_expr = expr; }

    Expr *getIndex() noexcept { return m_index; }
    Expr *setIndex(Expr *index) noexcept { return m_index = index; }
  };

  class Ident final : public Expr {
    QCLASS_REFLECT()

    std::string_view m_name;
    Expr *m_what;

  public:
    Ident(std::string_view name, Expr *what) : Expr(QIR_NODE_IDENT), m_name(name), m_what(what) {}

    Expr *getWhat() noexcept { return m_what; }
    Expr *setWhat(Expr *what) noexcept { return m_what = what; }

    std::string_view setName(std::string_view name) noexcept { return m_name = name; }
  };

  class Extern final : public Expr {
    QCLASS_REFLECT()

    std::string_view m_abi_name;
    Expr *m_value;

  public:
    Extern(Expr *value, std::string_view abi_name)
        : Expr(QIR_NODE_EXTERN), m_abi_name(abi_name), m_value(value) {
      setConstExpr(value->isConstExpr());
    }

    std::string_view getAbiName() const noexcept { return m_abi_name; }
    std::string_view setAbiName(std::string_view abi_name) noexcept {
      return m_abi_name = abi_name;
    }

    Expr *getValue() noexcept { return m_value; }
    Expr *setValue(Expr *value) noexcept { return m_value = value; }
  };

  class Local final : public Expr {
    QCLASS_REFLECT()

    std::string_view m_name;
    Expr *m_value;
    AbiTag m_abi_tag;

  public:
    Local(std::string_view name, Expr *value, AbiTag abi_tag)
        : Expr(QIR_NODE_LOCAL), m_name(name), m_value(value), m_abi_tag(abi_tag) {
      setConstExpr(value->isConstExpr());
    }

    std::string_view setName(std::string_view name) noexcept { return m_name = name; }

    Expr *getValue() noexcept { return m_value; }
    Expr *setValue(Expr *value) noexcept { return m_value = value; }

    AbiTag getAbiTag() const noexcept { return m_abi_tag; }
    AbiTag setAbiTag(AbiTag abi_tag) noexcept { return m_abi_tag = abi_tag; }
  };

  class Ret final : public Expr {
    QCLASS_REFLECT()

    Expr *m_expr;

  public:
    Ret(Expr *expr) : Expr(QIR_NODE_RET), m_expr(expr) { setConstExpr(expr->isConstExpr()); }

    Expr *getExpr() noexcept { return m_expr; }
    Expr *setExpr(Expr *expr) noexcept { return m_expr = expr; }
  };

  class Brk final : public Expr {
    QCLASS_REFLECT()

  public:
    Brk() : Expr(QIR_NODE_BRK) {}
  };

  class Cont final : public Expr {
    QCLASS_REFLECT()

  public:
    Cont() : Expr(QIR_NODE_CONT) {}
  };

  class If final : public Expr {
    QCLASS_REFLECT()

    Expr *m_cond;
    Expr *m_then;
    Expr *m_else;

  public:
    If(Expr *cond, Expr *then, Expr *else_)
        : Expr(QIR_NODE_IF), m_cond(cond), m_then(then), m_else(else_) {
      if (cond->isConstExpr() && then->isConstExpr() && else_->isConstExpr()) {
        setConstExpr(true);
      }
    }

    Expr *getCond() noexcept { return m_cond; }
    Expr *setCond(Expr *cond) noexcept { return m_cond = cond; }

    Expr *getThen() noexcept { return m_then; }
    Expr *setThen(Expr *then) noexcept { return m_then = then; }

    Expr *getElse() noexcept { return m_else; }
    Expr *setElse(Expr *else_) noexcept { return m_else = else_; }
  };

  class While final : public Expr {
    QCLASS_REFLECT()

    Expr *m_cond;
    Seq *m_body;

  public:
    While(Expr *cond, Seq *body) : Expr(QIR_NODE_WHILE), m_cond(cond), m_body(body) {
      if (cond->isConstExpr() && body->isConstExpr()) {
        setConstExpr(true);
      }
    }

    Expr *getCond() noexcept { return m_cond; }
    Expr *setCond(Expr *cond) noexcept { return m_cond = cond; }

    Seq *getBody() noexcept { return m_body; }
    Seq *setBody(Seq *body) noexcept { return m_body = body; }
  };

  class For final : public Expr {
    QCLASS_REFLECT()

    Expr *m_init;
    Expr *m_cond;
    Expr *m_step;
    Expr *m_body;

  public:
    For(Expr *init, Expr *cond, Expr *step, Expr *body)
        : Expr(QIR_NODE_FOR), m_init(init), m_cond(cond), m_step(step), m_body(body) {
      if (init->isConstExpr() && cond->isConstExpr() && step->isConstExpr() &&
          body->isConstExpr()) {
        setConstExpr(true);
      }
    }

    Expr *getInit() noexcept { return m_init; }
    Expr *setInit(Expr *init) noexcept { return m_init = init; }

    Expr *getCond() noexcept { return m_cond; }
    Expr *setCond(Expr *cond) noexcept { return m_cond = cond; }

    Expr *getStep() noexcept { return m_step; }
    Expr *setStep(Expr *step) noexcept { return m_step = step; }

    Expr *getBody() noexcept { return m_body; }
    Expr *setBody(Expr *body) noexcept { return m_body = body; }
  };

  class Form final : public Expr {
    QCLASS_REFLECT()

    std::string_view m_idx_ident;
    std::string_view m_val_ident;
    Expr *m_maxjobs;
    Expr *m_expr;
    Seq *m_body;

  public:
    Form(std::string_view idx_ident, std::string_view val_ident, Expr *maxjobs, Expr *expr,
         Seq *body)
        : Expr(QIR_NODE_FORM),
          m_idx_ident(idx_ident),
          m_val_ident(val_ident),
          m_maxjobs(maxjobs),
          m_expr(expr),
          m_body(body) {
      if (maxjobs->isConstExpr() && expr->isConstExpr() && body->isConstExpr()) {
        setConstExpr(true);
      }
    }

    std::string_view getIdxIdent() noexcept { return m_idx_ident; }
    std::string_view setIdxIdent(std::string_view idx_ident) noexcept {
      return m_idx_ident = idx_ident;
    }

    std::string_view getValIdent() noexcept { return m_val_ident; }
    std::string_view setValIdent(std::string_view val_ident) noexcept {
      return m_val_ident = val_ident;
    }

    Expr *getMaxJobs() noexcept { return m_maxjobs; }
    Expr *setMaxJobs(Expr *maxjobs) noexcept { return m_maxjobs = maxjobs; }

    Expr *getExpr() noexcept { return m_expr; }
    Expr *setExpr(Expr *expr) noexcept { return m_expr = expr; }

    Seq *getBody() noexcept { return m_body; }
    Seq *setBody(Seq *body) noexcept { return m_body = body; }
  };

  class Case final : public Expr {
    QCLASS_REFLECT()

    Expr *m_cond;
    Expr *m_body;

  public:
    Case(Expr *cond, Expr *body) : Expr(QIR_NODE_CASE), m_cond(cond), m_body(body) {
      if (cond->isConstExpr() && body->isConstExpr()) {
        setConstExpr(true);
      }
    }

    Expr *getCond() noexcept { return m_cond; }
    Expr *setCond(Expr *cond) noexcept { return m_cond = cond; }

    Expr *getBody() noexcept { return m_body; }
    Expr *setBody(Expr *body) noexcept { return m_body = body; }
  };

  typedef std::vector<Case *, Arena<Case *>> SwitchCases;

  class Switch final : public Expr {
    QCLASS_REFLECT()

    Expr *m_cond;
    Expr *m_default;
    SwitchCases m_cases;

  public:
    Switch(Expr *cond, const SwitchCases &cases, Expr *default_)
        : Expr(QIR_NODE_SWITCH), m_cond(cond), m_default(default_), m_cases(cases) {
      if (cond->isConstExpr() && default_->isConstExpr() &&
          std::all_of(cases.begin(), cases.end(), [](Case *c) { return c->isConstExpr(); })) {
        setConstExpr(true);
      }
    }

    Expr *getCond() noexcept { return m_cond; }
    Expr *setCond(Expr *cond) noexcept { return m_cond = cond; }

    Expr *getDefault() noexcept { return m_default; }
    Expr *setDefault(Expr *default_) noexcept { return m_default = default_; }

    const SwitchCases &getCases() const noexcept { return m_cases; }
    SwitchCases &getCases() noexcept { return m_cases; }
    void setCases(const SwitchCases &cases) noexcept { m_cases = cases; }
    void addCase(Case *c) noexcept { m_cases.push_back(c); }
  };

  typedef std::vector<std::pair<Type *, std::string_view>,
                      Arena<std::pair<Type *, std::string_view>>>
      Params;

  class Fn final : public Expr {
    QCLASS_REFLECT()

    std::string_view m_name;
    Params m_params;
    Type *m_return;
    Seq *m_body;
    bool m_variadic;
    AbiTag m_abi_tag;

  public:
    Fn(std::string_view name, const Params &params, Type *ret_ty, Seq *body, bool variadic,
       AbiTag abi_tag)
        : Expr(QIR_NODE_FN),
          m_name(name),
          m_params(params),
          m_return(ret_ty),
          m_body(body),
          m_variadic(variadic),
          m_abi_tag(abi_tag) {
      if (std::all_of(params.begin(), params.end(),
                      [](const std::pair<Type *, std::string_view> &p) {
                        return p.first->isConstExpr();
                      }) &&
          body->isConstExpr()) {
        setConstExpr(true);
      }
    }

    std::string_view setName(std::string_view name) noexcept { return m_name = name; }

    const Params &getParams() const noexcept { return m_params; }
    Params &getParams() noexcept { return m_params; }
    void setParams(const Params &params) noexcept { m_params = params; }

    Type *getReturn() noexcept { return m_return; }
    Type *setReturn(Type *ret_ty) noexcept { return m_return = ret_ty; }

    Seq *getBody() noexcept { return m_body; }
    Seq *setBody(Seq *body) noexcept { return m_body = body; }

    bool isVariadic() noexcept { return m_variadic; }
    void setVariadic(bool variadic) noexcept { m_variadic = variadic; }

    AbiTag getAbiTag() const noexcept { return m_abi_tag; }
    AbiTag setAbiTag(AbiTag abi_tag) noexcept { return m_abi_tag = abi_tag; }
  };

  class Asm final : public Expr {
    QCLASS_REFLECT()

  public:
    Asm() : Expr(QIR_NODE_ASM) { qcore_implement(__func__); }
  };

  ///=============================================================================
  /// END: EXPRESSIONS
  ///=============================================================================

#ifdef __QUIX_IMPL__

  enum class TmpType {
    NULL_LITERAL,
    UNDEF_LITERAL,
    CALL,
    ENUM,
    NAMED_TYPE,

    BAD,
  };

  typedef std::tuple<std::string_view, Expr *> LetTmpNodeCradle;

  typedef std::tuple<Expr *, std::vector<std::pair<std::string_view, Expr *>,
                                         Arena<std::pair<std::string_view, Expr *>>>>
      CallArgsTmpNodeCradle;

  typedef std::tuple<Expr *, std::string_view> FieldTmpNodeCradle;

  typedef std::variant<LetTmpNodeCradle, CallArgsTmpNodeCradle, FieldTmpNodeCradle,
                       std::string_view>
      TmpNodeCradle;

  class Tmp final : public Expr {
    QCLASS_REFLECT()

    TmpType m_type;
    TmpNodeCradle m_data;

  public:
    Tmp(TmpType type = TmpType::BAD, const TmpNodeCradle &data = {})
        : Expr(QIR_NODE_TMP), m_type(type), m_data(data) {}

    TmpType getTmpType() noexcept { return m_type; }
    TmpNodeCradle &getData() noexcept { return m_data; }
    const TmpNodeCradle &getData() const noexcept { return m_data; }
  };
#endif

#define TYPE_SIZE sizeof(Expr)
  extern thread_local qmodule_t *current;

  template <typename T>
  constexpr T *getType() {
    Type *type = nullptr;
    if constexpr (std::is_same_v<T, U1Ty>) {
      type = new (Arena<U1Ty>().allocate(1)) U1Ty();
    } else if constexpr (std::is_same_v<T, U8Ty>) {
      type = new (Arena<U8Ty>().allocate(1)) U8Ty();
    } else if constexpr (std::is_same_v<T, U16Ty>) {
      type = new (Arena<U16Ty>().allocate(1)) U16Ty();
    } else if constexpr (std::is_same_v<T, U32Ty>) {
      type = new (Arena<U32Ty>().allocate(1)) U32Ty();
    } else if constexpr (std::is_same_v<T, U64Ty>) {
      type = new (Arena<U64Ty>().allocate(1)) U64Ty();
    } else if constexpr (std::is_same_v<T, U128Ty>) {
      type = new (Arena<U128Ty>().allocate(1)) U128Ty();
    } else if constexpr (std::is_same_v<T, I8Ty>) {
      type = new (Arena<I8Ty>().allocate(1)) I8Ty();
    } else if constexpr (std::is_same_v<T, I16Ty>) {
      type = new (Arena<I16Ty>().allocate(1)) I16Ty();
    } else if constexpr (std::is_same_v<T, I32Ty>) {
      type = new (Arena<I32Ty>().allocate(1)) I32Ty();
    } else if constexpr (std::is_same_v<T, I64Ty>) {
      type = new (Arena<I64Ty>().allocate(1)) I64Ty();
    } else if constexpr (std::is_same_v<T, I128Ty>) {
      type = new (Arena<I128Ty>().allocate(1)) I128Ty();
    } else if constexpr (std::is_same_v<T, F16Ty>) {
      type = new (Arena<F16Ty>().allocate(1)) F16Ty();
    } else if constexpr (std::is_same_v<T, F32Ty>) {
      type = new (Arena<F32Ty>().allocate(1)) F32Ty();
    } else if constexpr (std::is_same_v<T, F64Ty>) {
      type = new (Arena<F64Ty>().allocate(1)) F64Ty();
    } else if constexpr (std::is_same_v<T, F128Ty>) {
      type = new (Arena<F128Ty>().allocate(1)) F128Ty();
    } else if constexpr (std::is_same_v<T, VoidTy>) {
      type = new (Arena<VoidTy>().allocate(1)) VoidTy();
    } else {
      static_assert(!std::is_same_v<T, T>,
                    "qxir::getType<T>(): Can not construct immuntable of this type.");
    }

    type->setModule(current);

    return static_cast<T *>(type);
  }

  template <typename T, typename... Args>
  static T *create(Args &&...args) {
    void *raw = Arena<T>().allocate(1);
    auto ptr = new (raw) T(std::forward<Args>(args)...);
    ptr->setModule(current);
    return ptr;
  }

  enum IterMode {
    dfs_pre,
    dfs_post,
    bfs_pre,
    bfs_post,
    children,
  };

  enum class IterMP {
    none,
    async,
  };

  enum class IterOp {
    Proceed,
    Abort,
    SkipChildren,
  };

  typedef std::function<IterOp(Expr *p, Expr **c)> IterCallback;
  typedef std::function<bool(Expr **a, Expr **b)> ChildSelect;

  namespace detail {
    void dfs_pre_impl(Expr **base, IterCallback cb, ChildSelect cs, bool parallel);
    void dfs_post_impl(Expr **base, IterCallback cb, ChildSelect cs, bool parallel);
    void bfs_pre_impl(Expr **base, IterCallback cb, ChildSelect cs, bool parallel);
    void bfs_post_impl(Expr **base, IterCallback cb, ChildSelect cs, bool parallel);
    void iter_children(Expr **base, IterCallback cb, ChildSelect cs, bool parallel);
  }  // namespace detail

  template <IterMode mode, IterMP mp = IterMP::none>
  void iterate(Expr *&base, IterCallback cb, ChildSelect cs = nullptr) {
    if constexpr (mode == dfs_pre) {
      return detail::dfs_pre_impl(&base, cb, cs, mp == IterMP::async);
    } else if constexpr (mode == dfs_post) {
      return detail::dfs_post_impl(&base, cb, cs, mp == IterMP::async);
    } else if constexpr (mode == bfs_pre) {
      return detail::bfs_pre_impl(&base, cb, cs, mp == IterMP::async);
    } else if constexpr (mode == bfs_post) {
      return detail::bfs_post_impl(&base, cb, cs, mp == IterMP::async);
    } else if constexpr (mode == children) {
      return detail::iter_children(&base, cb, cs, mp == IterMP::async);
    } else {
      static_assert(mode != mode, "Invalid iteration mode.");
    }
  }

  Expr *evaluate_to_literal(Expr *x) noexcept;

  template <typename T>
  std::optional<T> uint_as(const Expr *x) noexcept {
#define IS_T(x) std::is_same_v<T, x>

    qcore_assert(x != nullptr, "qxir::evaluate_as(): x is nullptr.");

    static_assert(IS_T(std::string) || IS_T(uint64_t),
                  "qxir::evaluate_as(): T must be either std::string or uint64_t.");

    Expr *r = evaluate_to_literal(const_cast<Expr *>(x));
    if (r == nullptr) {
      return std::nullopt;
    }

    qxir_ty_t ty = r->getKind();

    if (ty != QIR_NODE_INT) {
      return std::nullopt;
    }

    if constexpr (IS_T(std::string)) {
      return r->as<Int>()->getValue();
    } else if constexpr (IS_T(uint64_t)) {
      uint64_t val;
      auto data = r->as<Int>()->getValue();
      if (std::from_chars(data.data(), data.data() + data.size(), val).ec == std::errc()) {
        return val;
      }

      return std::nullopt;
    }

    return std::nullopt;

#undef IS_T
  }
}  // namespace qxir

#endif

#endif  // __QUIX_QXIR_NODE_H__
