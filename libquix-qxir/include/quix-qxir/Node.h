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

#define QIR_NODE_COUNT 53

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
 */
qxir_node_t *qxir_clone(qmodule_t *dst, const qxir_node_t *node);

#ifdef __cplusplus
}
#endif

///=============================================================================
/// END: ABSTRACT SYNTAX TREE DATA TYPES
///=============================================================================

#if (defined(__cplusplus) && defined(QXIR_USE_CPP_API)) || defined(__QXIR_IMPL__)

#include <quix-core/Arena.h>
#include <quix-core/Error.h>
#include <quix-lexer/Token.h>
#include <quix-qxir/Module.h>
#include <quix-qxir/TypeDecl.h>

#include <any>
#include <boost/uuid/uuid.hpp>
#include <cassert>
#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <ostream>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

namespace qxir {
  class ArenaAllocatorImpl {
    /// WARNING: This must be the first member; C bindings use
    /// qxir_arena as of type `qcore_arena_t`.
    qcore_arena_t m_arena;

  public:
    ArenaAllocatorImpl();
    ~ArenaAllocatorImpl();

    void *allocate(std::size_t bytes);
    void deallocate(void *ptr) noexcept;

    qcore_arena_t &get() { return m_arena; }
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

class qxir_node_t {
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
    uint64_t m_volatile : 1;          /* Is this expression volatile? */

    qlex_loc_t m_start_loc;
    uint16_t m_loc_size;  // Diagnostics can not span more than 64K bytes.

    Expr(const Expr &) = delete;
    Expr &operator=(const Expr &) = delete;

  public:
    Expr(qxir_ty_t ty = QIR_NODE_BAD)
        : m_node_type(ty),
          m_module_idx(0),
          m_constexpr(0),
          m_volatile(0),
          m_start_loc{},
          m_loc_size(0) {}

    uint32_t thisSizeOf() const noexcept;
    qxir_ty_t getKind() const noexcept;
    const char *thisTypeName() const noexcept;

    bool isType() const noexcept;
    inline bool isConst() const noexcept { return m_constexpr; }
    inline bool isVolatile() const noexcept { return m_volatile; }
    inline void setConst(bool is_const) noexcept { m_constexpr = is_const; }
    inline void setVolatile(bool is_volatile) noexcept { m_volatile = is_volatile; }

    std::pair<qlex_loc_t, qlex_loc_t> getLoc() const noexcept;
    void setLoc(std::pair<qlex_loc_t, qlex_loc_t> loc) noexcept;

    qmodule_t *getModule() const noexcept;
    void setModule(qmodule_t *module) noexcept;

    Type *getType() noexcept;

    /**
     * @brief Type-safe cast (type check only in debug mode).
     *
     * @tparam T The type to cast to.
     * @return const T* The casted pointer. It may be nullptr if the source pointer is nullptr.
     * @warning This function will panic if the cast is invalid.
     */
    template <typename T>
    const T *as() const noexcept {
      return reinterpret_cast<const T *>(this);
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
      return reinterpret_cast<T *>(this);
    }

    Expr *asExpr() noexcept { return this; }
    Type *asType() noexcept { return as<Type>(); }

    /**
     * @brief Type check.
     *
     * @tparam T The type to check.
     * @return true If the type matches.
     * @return false If the type does not match.
     */
    template <typename T>
    bool is() const noexcept {
      return typeid(*this) == typeid(T);
    }

    /**
     * @brief Type check.
     *
     * @param type The type to check.
     * @return true If the type matches.
     * @return false If the type does not match.
     */
    bool is(qxir_ty_t type) const noexcept;

    /**
     * @brief Print the node to the output stream.
     * @param os The output stream.
     * @param isForDebug Whether to print the node for debugging.
     */
    void dump(std::ostream &os, bool isForDebug = false) const;

    /**
     * @brief Compute the hash of the node.
     * @return boost::uuids::uuid The hash.
     */
    boost::uuids::uuid hash() noexcept;

    /**
     * @brief Get a unique identifier for the node.
     * @return std::string The unique identifier.
     */
    std::string getUniqueId() noexcept;
  } __attribute__((packed)) __attribute__((aligned(8)));

#define EXPR_SIZE sizeof(Expr)

  class Type : public Expr {
  public:
    Type(qxir_ty_t ty) : Expr(ty) {}

    bool isSizeKnown() const noexcept;
    size_t getSizeBits();
    inline size_t getSizeBytes() { return std::ceil(getSizeBits() / 8.0); }
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
    Offsetof,  /* 'offsetof': Offset of operator */
    BitcastAs, /* 'bitcast_as': Bitcast operator */
    CastAs,    /* 'cast_as': Common operator */
    Bitsizeof, /* 'bitsizeof': Bit size of operator */
  };

  std::ostream &operator<<(std::ostream &os, Op op);

  class BinExpr final : public Expr {
    QCLASS_REFLECT()

    Expr *m_lhs;
    Expr *m_rhs;
    Op m_op;

  public:
    BinExpr(Expr *lhs, Expr *rhs, Op op)
        : Expr(QIR_NODE_BINEXPR), m_lhs(lhs), m_rhs(rhs), m_op(op) {}

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
    UnExpr(Expr *expr, Op op) : Expr(QIR_NODE_UNEXPR), m_expr(expr), m_op(op) {}

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
    PostUnExpr(Expr *expr, Op op) : Expr(QIR_NODE_POST_UNEXPR), m_expr(expr), m_op(op) {}

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
    I16Ty() : Type(QIR_NODE_I16_TY) {};
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

    std::string_view getName() noexcept { return m_name; }
  };

  class StringTy final : public Type {
    QCLASS_REFLECT()

  public:
    StringTy() : Type(QIR_NODE_STRING_TY) {}
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
    Expr *m_size;

  public:
    ArrayTy(Type *element, Expr *size)
        : Type(QIR_NODE_ARRAY_TY), m_element(element), m_size(size) {}

    Type *getElement() noexcept { return m_element; }
    Expr *getCount() { return m_size; }
  };

  class ListTy final : public Type {
    QCLASS_REFLECT()

    Type *m_element;

  public:
    ListTy(Type *element) : Type(QIR_NODE_LIST_TY), m_element(element) {}

    Type *getElement() noexcept { return m_element; }
  };

  class IntrinTy final : public Type {
    QCLASS_REFLECT()

    std::string_view m_name;

  public:
    IntrinTy(std::string_view name) : Type(QIR_NODE_INTRIN_TY), m_name(name) {}

    std::string_view getName() noexcept { return m_name; }
  };

  enum class FnTag {
    NONE,
  };

  typedef std::vector<Type *, Arena<Type *>> FnParams;
  typedef std::unordered_set<FnTag, std::hash<FnTag>, std::equal_to<FnTag>, Arena<FnTag>> FnAttrs;

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
    Int(uint64_t u64) : Expr(QIR_NODE_INT), m_data{.m_u64 = u64 | FLAG_BIT} {}
    Int(std::string_view str) : Expr(QIR_NODE_INT), m_data{.m_str = str.data()} {
      qcore_assert((m_data.m_u64 & FLAG_BIT) == 0,
                   "Optimized code assumed an invariant that does not hold on this architecture.");
    }

    bool isNativeRepresentation() const noexcept { return m_data.m_u64 & FLAG_BIT; }

    uint64_t getNativeRepresentation() noexcept {
      qcore_assert(isNativeRepresentation());
      return m_data.m_u64 & ~FLAG_BIT;
    }

    std::string_view getStringRepresentation() noexcept {
      qcore_assert(!isNativeRepresentation());
      return m_data.m_str;
    }

    std::string getValue() noexcept {
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
    Float(double f64) : Expr(QIR_NODE_FLOAT), m_data{f64} {}
    Float(std::string_view str) : Expr(QIR_NODE_FLOAT), m_data{str.data()} {}

    bool isNativeRepresentation() const noexcept { return std::holds_alternative<double>(m_data); }

    double getNativeRepresentation() noexcept {
      qcore_assert(isNativeRepresentation());
      return std::get<double>(m_data);
    }

    std::string_view getStringRepresentation() noexcept {
      qcore_assert(!isNativeRepresentation());
      return std::get<const char *>(m_data);
    }

    std::string getValue() noexcept {
      return isNativeRepresentation() ? std::to_string(getNativeRepresentation())
                                      : std::string(getStringRepresentation());
    }

    void setValue(double f64) noexcept { m_data = f64; }
    void setValue(std::string_view str) noexcept { m_data = str.data(); }
  };

  class String final : public Expr {
    QCLASS_REFLECT()

    std::string_view m_data;

  public:
    String(std::string_view data) : Expr(QIR_NODE_STRING), m_data(data) {}

    std::string_view getValue() noexcept { return m_data; }
    std::string_view setValue(std::string_view data) noexcept { return m_data = data; }
  };

  typedef std::vector<Expr *, Arena<Expr *>> ListItems;

  class List final : public Expr {
    QCLASS_REFLECT()

    ListItems m_items;

  public:
    List(const ListItems &items) : Expr(QIR_NODE_LIST), m_items(items) {}

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

  class Alloc : public Expr {
    QCLASS_REFLECT()

    Type *m_type;

  public:
    Alloc(Type *type) : Expr(QIR_NODE_ALLOC), m_type(type) {}

    Type *getAllocType() noexcept { return m_type; }
    Type *setAllocType(Type *type) noexcept { return m_type = type; }

    size_t getSizeBytes() noexcept { return m_type->getSizeBytes(); }
  };

  typedef std::vector<Expr *, Arena<Expr *>> CallArgs;

  class Call final : public Expr {
    QCLASS_REFLECT()

    Expr *m_fn;
    CallArgs m_args;

  public:
    Call(Expr *fn, const CallArgs &args) : Expr(QIR_NODE_CALL), m_fn(fn), m_args(args) {}

    Expr *getFn() noexcept { return m_fn; }
    Expr *setFn(Expr *fn) noexcept { return m_fn = fn; }

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
    Seq(const SeqItems &items) : Expr(QIR_NODE_SEQ), m_items(items) {}

    const SeqItems &getItems() const noexcept { return m_items; }
    SeqItems &getItems() noexcept { return m_items; }
    void setItems(const SeqItems &items) noexcept { m_items = items; }
    void addItem(Expr *item) noexcept { m_items.push_back(item); }
  };

  typedef std::vector<Expr *, Arena<Expr *>> AsyncItems;

  class Async final : public Expr {
    QCLASS_REFLECT()

    AsyncItems m_items;

  public:
    Async(const AsyncItems &items) : Expr(QIR_NODE_ASYNC), m_items(items) {}

    const AsyncItems &getItems() const noexcept { return m_items; }
    AsyncItems &getItems() noexcept { return m_items; }
    void setItems(const AsyncItems &items) noexcept { m_items = items; }
    void addItem(Expr *item) noexcept { m_items.push_back(item); }
  };

  class Index final : public Expr {
    QCLASS_REFLECT()

    Expr *m_expr;
    Expr *m_index;

  public:
    Index(Expr *expr, Expr *index) : Expr(QIR_NODE_INDEX), m_expr(expr), m_index(index) {}

    Expr *getExpr() noexcept { return m_expr; }
    Expr *setExpr(Expr *expr) noexcept { return m_expr = expr; }

    Expr *getIndex() noexcept { return m_index; }
    Expr *setIndex(Expr *index) noexcept { return m_index = index; }
  };

  class Ident final : public Expr {
    QCLASS_REFLECT()

    std::string_view m_name;

  public:
    Ident(std::string_view name) : Expr(QIR_NODE_IDENT), m_name(name) {}

    std::string_view getName() noexcept { return m_name; }
    std::string_view setName(std::string_view name) noexcept { return m_name = name; }
  };

  class Export final : public Expr {
    QCLASS_REFLECT()

    std::string_view m_name;
    std::string_view m_abi_name;
    Expr *m_value;

  public:
    Export(std::string_view name, Expr *value, std::string_view abi_name = "")
        : Expr(QIR_NODE_EXPORT), m_name(name), m_abi_name(abi_name), m_value(value) {}

    std::string_view getName() noexcept { return m_name; }
    std::string_view setName(std::string_view name) noexcept { return m_name = name; }

    std::string_view getAbiName() noexcept { return m_abi_name; }
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

  public:
    Local(std::string_view name, Expr *value) : Expr(QIR_NODE_LOCAL), m_name(name), m_value(value) {}

    std::string_view getName() noexcept { return m_name; }
    std::string_view setName(std::string_view name) noexcept { return m_name = name; }

    Expr *getValue() noexcept { return m_value; }
    Expr *setValue(Expr *value) noexcept { return m_value = value; }
  };
  
  class Ret final : public Expr {
    QCLASS_REFLECT()

    Expr *m_expr;

  public:
    Ret(Expr *expr) : Expr(QIR_NODE_RET), m_expr(expr) {}

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
        : Expr(QIR_NODE_IF), m_cond(cond), m_then(then), m_else(else_) {}

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
    Expr *m_body;

  public:
    While(Expr *cond, Expr *body) : Expr(QIR_NODE_WHILE), m_cond(cond), m_body(body) {}

    Expr *getCond() noexcept { return m_cond; }
    Expr *setCond(Expr *cond) noexcept { return m_cond = cond; }

    Expr *getBody() noexcept { return m_body; }
    Expr *setBody(Expr *body) noexcept { return m_body = body; }
  };

  class For final : public Expr {
    QCLASS_REFLECT()

    Expr *m_init;
    Expr *m_cond;
    Expr *m_step;
    Expr *m_body;

  public:
    For(Expr *init, Expr *cond, Expr *step, Expr *body)
        : Expr(QIR_NODE_FOR), m_init(init), m_cond(cond), m_step(step), m_body(body) {}

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
          m_body(body) {}

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

  class Foreach final : public Expr {
    QCLASS_REFLECT()

    std::string_view m_idx_ident;
    std::string_view m_val_ident;
    Expr *m_expr;
    Seq *m_body;

  public:
    Foreach(std::string_view idx_ident, std::string_view val_ident, Expr *expr, Seq *body)
        : Expr(QIR_NODE_FOREACH),
          m_idx_ident(idx_ident),
          m_val_ident(val_ident),
          m_expr(expr),
          m_body(body) {}

    std::string_view getIdxIdent() noexcept { return m_idx_ident; }
    std::string_view setIdxIdent(std::string_view idx_ident) noexcept {
      return m_idx_ident = idx_ident;
    }

    std::string_view getValIdent() noexcept { return m_val_ident; }
    std::string_view setValIdent(std::string_view val_ident) noexcept {
      return m_val_ident = val_ident;
    }

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
    Case(Expr *cond, Expr *body) : Expr(QIR_NODE_CASE), m_cond(cond), m_body(body) {}

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
        : Expr(QIR_NODE_SWITCH), m_cond(cond), m_default(default_), m_cases(cases) {}

    Expr *getCond() noexcept { return m_cond; }
    Expr *setCond(Expr *cond) noexcept { return m_cond = cond; }

    Expr *getDefault() noexcept { return m_default; }
    Expr *setDefault(Expr *default_) noexcept { return m_default = default_; }

    const SwitchCases &getCases() const noexcept { return m_cases; }
    SwitchCases &getCases() noexcept { return m_cases; }
    void setCases(const SwitchCases &cases) noexcept { m_cases = cases; }
    void addCase(Case *c) noexcept { m_cases.push_back(c); }
  };

  typedef std::vector<Expr *, Arena<Expr *>> Params;

  class Fn final : public Expr {
    QCLASS_REFLECT()

    std::string_view m_name;
    Params m_params;
    Seq *m_body;

  public:
    Fn(std::string_view name, const Params &params, Seq *body)
        : Expr(QIR_NODE_FN), m_name(name), m_params(params), m_body(body) {}

    std::string_view getName() noexcept { return m_name; }
    std::string_view setName(std::string_view name) noexcept { return m_name = name; }

    const Params &getParams() const noexcept { return m_params; }
    Params &getParams() noexcept { return m_params; }
    void setParams(const Params &params) noexcept { m_params = params; }
    void addParam(Expr *param) noexcept { m_params.push_back(param); }

    Seq *getBody() noexcept { return m_body; }
    Seq *setBody(Seq *body) noexcept { return m_body = body; }
  };

  class Asm final : public Expr {
    QCLASS_REFLECT()

  public:
    Asm() : Expr(QIR_NODE_ASM) { qcore_implement(__func__); }
  };

  ///=============================================================================
  /// END: EXPRESSIONS
  ///=============================================================================

  enum class TmpType {
    NULL_LITERAL,
    UNDEF_LITERAL,
    CALL,
    ENUM,
    LET,
    VAR,
    CONST,
    FIELD,
    NAMED_TYPE,

    BAD,
  };

  class Tmp final : public Expr {
    QCLASS_REFLECT()

    TmpType m_type;
    std::any m_data;

  public:
    Tmp(TmpType type = TmpType::BAD, const std::any &data = {})
        : Expr(QIR_NODE_TMP), m_type(type), m_data(data) {}

    TmpType getTmpType() noexcept { return m_type; }
    std::any &getData() noexcept { return m_data; }
    const std::any &getData() const noexcept { return m_data; }
  };

  typedef std::tuple<std::string_view, Expr *> LetTmpNodeCradle;

  typedef std::tuple<Expr *, std::vector<std::pair<std::string_view, Expr *>,
                                         Arena<std::pair<std::string_view, Expr *>>>>
      CallArgsTmpNodeCradle;

  typedef std::tuple<Expr *, std::string_view> FieldTmpNodeCradle;

#define TYPE_SIZE sizeof(Expr)

  template <typename T>
  constexpr T *getType() {
    /// TODO: Optimize this via deduplication.

    if constexpr (std::is_same_v<T, U1Ty>) {
      return new (Arena<U1Ty>().allocate(1)) U1Ty();
    } else if constexpr (std::is_same_v<T, U8Ty>) {
      return new (Arena<U8Ty>().allocate(1)) U8Ty();
    } else if constexpr (std::is_same_v<T, U16Ty>) {
      return new (Arena<U16Ty>().allocate(1)) U16Ty();
    } else if constexpr (std::is_same_v<T, U32Ty>) {
      return new (Arena<U32Ty>().allocate(1)) U32Ty();
    } else if constexpr (std::is_same_v<T, U64Ty>) {
      return new (Arena<U64Ty>().allocate(1)) U64Ty();
    } else if constexpr (std::is_same_v<T, U128Ty>) {
      return new (Arena<U128Ty>().allocate(1)) U128Ty();
    } else if constexpr (std::is_same_v<T, I8Ty>) {
      return new (Arena<I8Ty>().allocate(1)) I8Ty();
    } else if constexpr (std::is_same_v<T, I16Ty>) {
      return new (Arena<I16Ty>().allocate(1)) I16Ty();
    } else if constexpr (std::is_same_v<T, I32Ty>) {
      return new (Arena<I32Ty>().allocate(1)) I32Ty();
    } else if constexpr (std::is_same_v<T, I64Ty>) {
      return new (Arena<I64Ty>().allocate(1)) I64Ty();
    } else if constexpr (std::is_same_v<T, I128Ty>) {
      return new (Arena<I128Ty>().allocate(1)) I128Ty();
    } else if constexpr (std::is_same_v<T, F16Ty>) {
      return new (Arena<F16Ty>().allocate(1)) F16Ty();
    } else if constexpr (std::is_same_v<T, F32Ty>) {
      return new (Arena<F32Ty>().allocate(1)) F32Ty();
    } else if constexpr (std::is_same_v<T, F64Ty>) {
      return new (Arena<F64Ty>().allocate(1)) F64Ty();
    } else if constexpr (std::is_same_v<T, F128Ty>) {
      return new (Arena<F128Ty>().allocate(1)) F128Ty();
    } else if constexpr (std::is_same_v<T, VoidTy>) {
      return new (Arena<VoidTy>().allocate(1)) VoidTy();
    } else if constexpr (std::is_same_v<T, StringTy>) {
      return new (Arena<StringTy>().allocate(1)) StringTy();
    } else {
      static_assert(!std::is_same_v<T, T>,
                    "qxir::getType<T>(): Can not construct immuntable of this type.");
    }
  }

  template <typename T, typename... Args>
  static T *create(Args &&...args) {
    void *ptr = Arena<T>().allocate(1);
    return new (ptr) T(std::forward<Args>(args)...);
  }

  enum IterMode {
    dfs_pre,
    dfs_post,
    bfs_pre,
    bfs_post,
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

  typedef std::function<IterOp(Expr *p, Expr *c)> IterCallback;
  typedef std::function<bool(Expr **a, Expr **b)> ChildSelect;

  namespace detail {
    void dfs_pre_impl(Expr *base, IterCallback cb, ChildSelect cs, bool parallel);
    void dfs_post_impl(Expr *base, IterCallback cb, ChildSelect cs, bool parallel);
    void bfs_pre_impl(Expr *base, IterCallback cb, ChildSelect cs, bool parallel);
    void bfs_post_impl(Expr *base, IterCallback cb, ChildSelect cs, bool parallel);
  }  // namespace detail

  template <IterMode mode, IterMP mp = IterMP::none>
  void iterate(Expr *base, IterCallback cb, ChildSelect cs = nullptr) {
    if constexpr (mode == dfs_pre) {
      return detail::dfs_pre_impl(base, cb, cs, mp == IterMP::async);
    } else if constexpr (mode == dfs_post) {
      return detail::dfs_post_impl(base, cb, cs, mp == IterMP::async);
    } else if constexpr (mode == bfs_pre) {
      return detail::bfs_pre_impl(base, cb, cs, mp == IterMP::async);
    } else if constexpr (mode == bfs_post) {
      return detail::bfs_post_impl(base, cb, cs, mp == IterMP::async);
    } else {
      static_assert(mode != mode, "Invalid iteration mode.");
    }
  }
}  // namespace qxir

#endif

#endif  // __QUIX_QXIR_NODE_H__
