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

#include <quix-lexer/Token.h>

/**
 * @brief Quixcc abstract syntax tree node.
 */
typedef struct qxir_node_t qxir_node_t;

/**
 * @brief Quixcc abstract syntax tree node type.
 */
typedef enum qxir_ty_t {
  QIR_NODE_BINEXPR,
  QIR_NODE_UNEXPR,
  QIR_NODE_POST_UNEXPR,

  QIR_NODE_INT,
  QIR_NODE_FLOAT,
  QIR_NODE_STRING,
  QIR_NODE_LIST,

  QIR_NODE_ALLOC,
  QIR_NODE_DEALLOC,
  QIR_NODE_CALL,
  QIR_NODE_SEQ,
  QIR_NODE_ASYNC,
  QIR_NODE_INDEX,
  QIR_NODE_IDENT,
  QIR_NODE_GLOBAL,
  QIR_NODE_RET,
  QIR_NODE_BRK,
  QIR_NODE_CONT,
  QIR_NODE_IF,
  QIR_NODE_WHILE,
  QIR_NODE_FOR,
  QIR_NODE_FORM,
  QIR_NODE_FOREACH,
  QIR_NODE_CASE,
  QIR_NODE_SWITCH,
  QIR_NODE_FN,
  QIR_NODE_ASM,

  QIR_NODE_U1_TY,
  QIR_NODE_U8_TY,
  QIR_NODE_U16_TY,
  QIR_NODE_U32_TY,
  QIR_NODE_U64_TY,
  QIR_NODE_U128_TY,
  QIR_NODE_I8_TY,
  QIR_NODE_I16_TY,
  QIR_NODE_I32_TY,
  QIR_NODE_I64_TY,
  QIR_NODE_I128_TY,
  QIR_NODE_F16_TY,
  QIR_NODE_F32_TY,
  QIR_NODE_F64_TY,
  QIR_NODE_F128_TY,
  QIR_NODE_VOID_TY,
  QIR_NODE_PTR_TY,
  QIR_NODE_OPAQUE_TY,
  QIR_NODE_STRING_TY,
  QIR_NODE_STRUCT_TY,
  QIR_NODE_UNION_TY,
  QIR_NODE_ARRAY_TY,
  QIR_NODE_LIST_TY,
  QIR_NODE_INTRIN_TY,
  QIR_NODE_FN_TY,
} qxir_ty_t;

#define QIR_NODE_COUNT 52

typedef struct qxir_node_t qxir_node_t;

///=============================================================================
/// END: ABSTRACT SYNTAX TREE DATA TYPES
///=============================================================================

#if (defined(__cplusplus) && defined(QXIR_USE_CPP_API)) || defined(__QXIR_IMPL__) || 1

#include <quix-core/Arena.h>
#include <quix-core/Error.h>

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

    void swap(qcore_arena_t &arena) { std::swap(m_arena, arena); }

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
  enum class ExportLang {};

  // class String : public std::basic_string<char, std::char_traits<char>, Arena<char>> {
  // public:
  //   String() = default;
  //   String(const char *str) : std::basic_string<char, std::char_traits<char>, Arena<char>>(str)
  //   {} String(const std::string &str)
  //       : std::basic_string<char, std::char_traits<char>, Arena<char>>(str.c_str(), str.size())
  //       {}
  // };

  typedef uint16_t ModuleId;

  class Module {
    ModuleId m_id;

  public:
    Module() = default;

    ModuleId getId() noexcept { return m_id; }
  };

  class Type;
  class Expr : public qxir_node_t {
#ifdef __QXIR_NODE_REFLECT_IMPL__
  public:
#endif
    uint64_t m_type_idx : 40;    /* Typecode of this expression. */
    ModuleId m_module_idx : 16;  /* The module context index. */
    uint64_t m_constexpr : 1;    /* Is this expression a constant expression? */
    uint64_t m_minimal : 1;      /* Is this expression already in simplified form? */
    uint64_t m_volatile : 1;     /* Is this expression volatile? */
    uint64_t m_higher_order : 1; /* Is higher-order construct? */
    uint64_t m_padding : 4;

  public:
    Expr() : m_type_idx(0), m_constexpr(0), m_minimal(0), m_volatile(0), m_higher_order(0) {}

    uint32_t thisSizeOf() const noexcept;
    qxir_ty_t thisTypeId() const noexcept;
    const char *thisTypeName() const noexcept;

    bool isType() const noexcept;
    inline bool isConst() const noexcept { return m_constexpr; }
    inline bool isReduced() const noexcept { return m_minimal; }
    inline bool isVolatile() const noexcept { return m_volatile; }
    inline bool isHigherOrder() const noexcept { return m_higher_order; }

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
#if !defined(NDEBUG)
      auto p = dynamic_cast<const T *>(this);

      if (!p) {
        const char *this_str = typeid(*this).name();
        const char *other_str = typeid(T).name();

        qcore_panicf(
            "qxir_node_t::as(const %s *this): Invalid cast from `%s` to "
            "`%s`.",
            this_str, this_str, other_str);
        __builtin_unreachable();
      }
      return p;
#else
      return reinterpret_cast<const T *>(this);
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
#if !defined(NDEBUG)
      auto p = dynamic_cast<T *>(this);

      if (!p) {
        const char *this_str = typeid(*this).name();
        const char *other_str = typeid(T).name();

        qcore_panicf("qxir_node_t::as(%s *this): Invalid cast from `%s` to `%s`.", this_str,
                     this_str, other_str);
        __builtin_unreachable();
      }
      return p;
#else
      return reinterpret_cast<T *>(this);
#endif
    }

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
    void dump(std::ostream &os, bool isForDebug = false) const noexcept;

    /**
     * @brief Compute the hash of the node.
     * @return boost::uuids::uuid The hash.
     */
    boost::uuids::uuid hash() noexcept;

    /**
     * @brief Compute the hash of the node.
     * @return boost::uuids::uuid The hash.
     */
    boost::uuids::uuid hash() const noexcept;

    Module *getModule() noexcept;
  } __attribute__((packed));

#define EXPR_SIZE sizeof(Expr)

  class Type : public Expr {
  public:
    Type() = default;

    bool isSizeKnown() const noexcept;
    size_t getSizeBits();
    inline size_t getSizeBytes() { return std::ceil(getSizeBits() / 8.0); }
  };

  ///=============================================================================
  /// BEGIN: EXPRESSIONS CATEGORIES
  ///=============================================================================

  enum class Op { TODO };

  class BinExpr final : public Expr {
    Expr *m_lhs;
    Expr *m_rhs;
    Op m_op;

  public:
    BinExpr(Expr *lhs, Expr *rhs, Op op) : m_lhs(lhs), m_rhs(rhs), m_op(op) {}

    Expr *getLHS() noexcept { return m_lhs; }
    Expr *getRHS() noexcept { return m_rhs; }
    Op getOp() noexcept { return m_op; }

    Expr *setLHS(Expr *lhs) noexcept { return m_lhs = lhs; }
    Expr *setRHS(Expr *rhs) noexcept { return m_rhs = rhs; }
    Op setOp(Op op) noexcept { return m_op = op; }
  };

  class UnExpr final : public Expr {
    Expr *m_expr;
    Op m_op;

  public:
    UnExpr(Expr *expr, Op op) : m_expr(expr), m_op(op) {}

    Expr *getExpr() noexcept { return m_expr; }
    Op getOp() noexcept { return m_op; }

    Expr *setExpr(Expr *expr) noexcept { return m_expr = expr; }
    Op setOp(Op op) noexcept { return m_op = op; }
  };

  class PostUnExpr final : public Expr {
    Expr *m_expr;
    Op m_op;

  public:
    PostUnExpr(Expr *expr, Op op) : m_expr(expr), m_op(op) {}

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
  public:
    U1Ty() = default;
  };

  class U8Ty final : public Type {
  public:
    U8Ty() = default;
  };

  class U16Ty final : public Type {
  public:
    U16Ty() = default;
  };

  class U32Ty final : public Type {
  public:
    U32Ty() = default;
  };

  class U64Ty final : public Type {
  public:
    U64Ty() = default;
  };

  class U128Ty final : public Type {
  public:
    U128Ty() = default;
  };

  class I8Ty final : public Type {
  public:
    I8Ty() = default;
  };

  class I16Ty final : public Type {
  public:
    I16Ty() = default;
  };

  class I32Ty final : public Type {
  public:
    I32Ty() = default;
  };

  class I64Ty final : public Type {
  public:
    I64Ty() = default;
  };

  class I128Ty final : public Type {
  public:
    I128Ty() = default;
  };

  class F16Ty final : public Type {
  public:
    F16Ty() = default;
  };

  class F32Ty final : public Type {
  public:
    F32Ty() = default;
  };

  class F64Ty final : public Type {
  public:
    F64Ty() = default;
  };

  class F128Ty final : public Type {
  public:
    F128Ty() = default;
  };

  class VoidTy final : public Type {
  public:
    VoidTy() = default;
  };

  /// ===========================================================================
  /// END: PRIMITIVE TYPES
  /// ===========================================================================

  /// ===========================================================================
  /// BEGIN: COMPLEX TYPES
  /// ===========================================================================

  class PtrTy final : public Type {
    Type *m_pointee;

  public:
    PtrTy(Type *pointee) : m_pointee(pointee) {}

    Type *getPointee() noexcept { return m_pointee; }
  };

  class OpaqueTy final : public Type {
    std::string_view m_name;

  public:
    OpaqueTy(std::string_view name) : m_name(name) {}

    std::string_view getName() noexcept { return m_name; }
  };

  class StringTy final : public Type {
  public:
    StringTy() = default;
  };

  typedef std::vector<Type *, Arena<Type *>> StructFields;

  class StructTy final : public Type {
    StructFields m_fields;

  public:
    StructTy(const StructFields &fields) : m_fields(fields) {}

    const StructFields &getFields() noexcept { return m_fields; }
  };

  typedef std::unordered_set<Type *, std::hash<Type *>, std::equal_to<Type *>, Arena<Type *>>
      UnionFields;

  class UnionTy final : public Type {
    UnionFields m_fields;

  public:
    UnionTy(const UnionFields &fields) : m_fields(fields) {}

    const UnionFields &getFields() noexcept { return m_fields; }
  };

  class ArrayTy final : public Type {
    Type *m_element;
    uint64_t m_size;

  public:
    ArrayTy(Type *element, uint64_t size) : m_element(element), m_size(size) {}

    Type *getElement() noexcept { return m_element; }
    uint64_t getCount() { return m_size; }
  };

  class ListTy final : public Type {
    Type *m_element;

  public:
    ListTy(Type *element) : m_element(element) {}

    Type *getElement() noexcept { return m_element; }
  };

  class IntrinTy final : public Type {
    std::string_view m_name;

  public:
    IntrinTy(std::string_view name) : m_name(name) {}

    std::string_view getName() noexcept { return m_name; }
  };

  enum class FnTag {
    NONE,
  };

  typedef std::vector<Type *, Arena<Type *>> FnParams;
  typedef std::unordered_set<FnTag, std::hash<FnTag>, std::equal_to<FnTag>, Arena<FnTag>> FnAttrs;

  class FnTy final : public Type {
    FnParams m_params;
    FnAttrs m_attrs;
    Type *m_return;

  public:
    FnTy(const FnParams &params, Type *ret, const FnAttrs &attrs)
        : m_params(params), m_attrs(attrs), m_return(ret) {}

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
    union {
      uint64_t m_u64;    /* bit 63 is 1 always; interpret as if it is 0. */
      const char *m_str; /* bit 63 is always 0, due to addressing on x86_64. */
    } m_data;

    static constexpr uint64_t FLAG_BIT = 1ULL << 63;

  public:
    Int(uint64_t u64) : m_data{.m_u64 = u64 | FLAG_BIT} {}
    Int(std::string_view str) : m_data{.m_str = str.data()} {
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
    std::variant<double, const char *> m_data;

    static_assert(sizeof(double) == 8);

  public:
    Float(double f64) : m_data{f64} {}
    Float(std::string_view str) : m_data{str.data()} {}

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
    std::string_view m_data;

  public:
    String(std::string_view data) : m_data(data) {}

    std::string_view getValue() noexcept { return m_data; }
    std::string_view setValue(std::string_view data) noexcept { return m_data = data; }
  };

  typedef std::vector<Expr *, Arena<Expr *>> ListItems;

  class List final : public Expr {
    ListItems m_items;

  public:
    List(const ListItems &items) : m_items(items) {}

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
    Type *m_type;

  public:
    Alloc(Type *type) : m_type(type) {}

    Type *getType() noexcept { return m_type; }
    Type *setType(Type *type) noexcept { return m_type = type; }

    size_t getSizeBytes() noexcept { return m_type->getSizeBytes(); }
  };

  class Dealloc : public Expr {
    Expr *m_expr;

  public:
    Dealloc(Expr *expr) : m_expr(expr) {}

    Expr *getExpr() noexcept { return m_expr; }
    Expr *setExpr(Expr *expr) noexcept { return m_expr = expr; }
  };

  typedef std::vector<Expr *, Arena<Expr *>> CallArgs;

  class Call final : public Expr {
    Expr *m_fn;
    CallArgs m_args;

  public:
    Call(Expr *fn, const CallArgs &args) : m_fn(fn), m_args(args) {}

    Expr *getFn() noexcept { return m_fn; }
    Expr *setFn(Expr *fn) noexcept { return m_fn = fn; }

    const CallArgs &getArgs() const noexcept { return m_args; }
    CallArgs &getArgs() noexcept { return m_args; }
    void setArgs(const CallArgs &args) noexcept { m_args = args; }

    size_t getNumArgs() noexcept { return m_args.size(); }
  };

  typedef std::vector<Expr *, Arena<Expr *>> SeqItems;

  class Seq final : public Expr {
    SeqItems m_items;

  public:
    Seq(const SeqItems &items) : m_items(items) {}

    const SeqItems &getItems() const noexcept { return m_items; }
    SeqItems &getItems() noexcept { return m_items; }
    void setItems(const SeqItems &items) noexcept { m_items = items; }
    void addItem(Expr *item) noexcept { m_items.push_back(item); }
  };

  typedef std::vector<Expr *, Arena<Expr *>> AsyncItems;

  class Async final : public Expr {
    AsyncItems m_items;

  public:
    Async(const AsyncItems &items) : m_items(items) {}

    const AsyncItems &getItems() const noexcept { return m_items; }
    AsyncItems &getItems() noexcept { return m_items; }
    void setItems(const AsyncItems &items) noexcept { m_items = items; }
    void addItem(Expr *item) noexcept { m_items.push_back(item); }
  };

  class Index final : public Expr {
    Expr *m_expr;
    Expr *m_index;

  public:
    Index(Expr *expr, Expr *index) : m_expr(expr), m_index(index) {}

    Expr *getExpr() noexcept { return m_expr; }
    Expr *setExpr(Expr *expr) noexcept { return m_expr = expr; }

    Expr *getIndex() noexcept { return m_index; }
    Expr *setIndex(Expr *index) noexcept { return m_index = index; }
  };

  class Ident final : public Expr {
    std::string_view m_name;

  public:
    Ident(std::string_view name) : m_name(name) {}

    std::string_view getName() noexcept { return m_name; }
    std::string_view setName(std::string_view name) noexcept { return m_name = name; }
  };

  class Global final : public Expr {
    std::string_view m_name;
    Expr *m_value;

  public:
    Global(std::string_view name, Expr *value) : m_name(name), m_value(value) {}

    std::string_view getName() noexcept { return m_name; }
    std::string_view setName(std::string_view name) noexcept { return m_name = name; }

    Expr *getValue() noexcept { return m_value; }
    Expr *setValue(Expr *value) noexcept { return m_value = value; }
  };

  class Ret final : public Expr {
    Expr *m_expr;

  public:
    Ret(Expr *expr) : m_expr(expr) {}

    Expr *getExpr() noexcept { return m_expr; }
    Expr *setExpr(Expr *expr) noexcept { return m_expr = expr; }
  };

  class Brk final : public Expr {
  public:
    Brk() {}
  };

  class Cont final : public Expr {
  public:
    Cont() {}
  };

  class If final : public Expr {
    Expr *m_cond;
    Expr *m_then;
    Expr *m_else;

  public:
    If(Expr *cond, Expr *then, Expr *else_) : m_cond(cond), m_then(then), m_else(else_) {}

    Expr *getCond() noexcept { return m_cond; }
    Expr *setCond(Expr *cond) noexcept { return m_cond = cond; }

    Expr *getThen() noexcept { return m_then; }
    Expr *setThen(Expr *then) noexcept { return m_then = then; }

    Expr *getElse() noexcept { return m_else; }
    Expr *setElse(Expr *else_) noexcept { return m_else = else_; }
  };

  class While final : public Expr {
    Expr *m_cond;
    Expr *m_body;

  public:
    While(Expr *cond, Expr *body) : m_cond(cond), m_body(body) {}

    Expr *getCond() noexcept { return m_cond; }
    Expr *setCond(Expr *cond) noexcept { return m_cond = cond; }

    Expr *getBody() noexcept { return m_body; }
    Expr *setBody(Expr *body) noexcept { return m_body = body; }
  };

  class For final : public Expr {
    Expr *m_init;
    Expr *m_cond;
    Expr *m_step;
    Expr *m_body;

  public:
    For(Expr *init, Expr *cond, Expr *step, Expr *body)
        : m_init(init), m_cond(cond), m_step(step), m_body(body) {}

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
    std::string_view m_idx_ident;
    std::string_view m_val_ident;
    Expr *m_maxjobs;
    Expr *m_expr;
    Expr *m_body;

  public:
    Form(std::string_view idx_ident, std::string_view val_ident, Expr *maxjobs, Expr *expr,
         Expr *body)
        : m_idx_ident(idx_ident),
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

    Expr *getBody() noexcept { return m_body; }
    Expr *setBody(Expr *body) noexcept { return m_body = body; }
  };

  class Foreach final : public Expr {
    std::string_view m_idx_ident;
    std::string_view m_val_ident;
    Expr *m_expr;
    Seq *m_body;

  public:
    Foreach(std::string_view idx_ident, std::string_view val_ident, Expr *expr, Seq *body)
        : m_idx_ident(idx_ident), m_val_ident(val_ident), m_expr(expr), m_body(body) {}

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
    Expr *m_cond;
    Expr *m_body;

  public:
    Case(Expr *cond, Expr *body) : m_cond(cond), m_body(body) {}

    Expr *getCond() noexcept { return m_cond; }
    Expr *setCond(Expr *cond) noexcept { return m_cond = cond; }

    Expr *getBody() noexcept { return m_body; }
    Expr *setBody(Expr *body) noexcept { return m_body = body; }
  };

  typedef std::vector<Case *, Arena<Case *>> SwitchCases;

  class Switch final : public Expr {
    Expr *m_cond;
    SwitchCases m_cases;

  public:
    Switch(Expr *cond, const SwitchCases &cases) : m_cond(cond), m_cases(cases) {}

    Expr *getCond() noexcept { return m_cond; }
    Expr *setCond(Expr *cond) noexcept { return m_cond = cond; }

    const SwitchCases &getCases() const noexcept { return m_cases; }
    SwitchCases &getCases() noexcept { return m_cases; }
    void setCases(const SwitchCases &cases) noexcept { m_cases = cases; }
    void addCase(Case *c) noexcept { m_cases.push_back(c); }
  };

  typedef std::vector<Expr, Arena<Expr>> Params;

  class Fn final : public Expr {
    std::string_view m_name;
    Params m_params;
    Seq *m_body;

  public:
    Fn(std::string_view name, const Params &params, Seq *body)
        : m_name(name), m_params(params), m_body(body) {}

    std::string_view getName() noexcept { return m_name; }
    std::string_view setName(std::string_view name) noexcept { return m_name = name; }

    const Params &getParams() const noexcept { return m_params; }
    Params &getParams() noexcept { return m_params; }
    void setParams(const Params &params) noexcept { m_params = params; }
    void addParam(Expr param) noexcept { m_params.push_back(param); }

    Seq *getBody() noexcept { return m_body; }
    Seq *setBody(Seq *body) noexcept { return m_body = body; }
  };

  class Asm final : public Expr {
  public:
    Asm() {
      /// TODO:
      qcore_panic("Not implemented yet.");
    }
  };

  ///=============================================================================
  /// END: EXPRESSIONS
  ///=============================================================================

#define TYPE_SIZE sizeof(StructTy)

  template <typename T>
  constexpr T *getType() {
    /// TODO: Optimize this.

    if constexpr (std::is_same_v<T, U1Ty>) {
      return Arena<U1Ty>().allocate(1);
    } else if constexpr (std::is_same_v<T, U8Ty>) {
      return Arena<U8Ty>().allocate(1);
    } else if constexpr (std::is_same_v<T, U16Ty>) {
      return Arena<U16Ty>().allocate(1);
    } else if constexpr (std::is_same_v<T, U32Ty>) {
      return Arena<U32Ty>().allocate(1);
    } else if constexpr (std::is_same_v<T, U64Ty>) {
      return Arena<U64Ty>().allocate(1);
    } else if constexpr (std::is_same_v<T, U128Ty>) {
      return Arena<U128Ty>().allocate(1);
    } else if constexpr (std::is_same_v<T, I8Ty>) {
      return Arena<I8Ty>().allocate(1);
    } else if constexpr (std::is_same_v<T, I16Ty>) {
      return Arena<I16Ty>().allocate(1);
    } else if constexpr (std::is_same_v<T, I32Ty>) {
      return Arena<I32Ty>().allocate(1);
    } else if constexpr (std::is_same_v<T, I64Ty>) {
      return Arena<I64Ty>().allocate(1);
    } else if constexpr (std::is_same_v<T, I128Ty>) {
      return Arena<I128Ty>().allocate(1);
    } else if constexpr (std::is_same_v<T, F16Ty>) {
      return Arena<F16Ty>().allocate(1);
    } else if constexpr (std::is_same_v<T, F32Ty>) {
      return Arena<F32Ty>().allocate(1);
    } else if constexpr (std::is_same_v<T, F64Ty>) {
      return Arena<F64Ty>().allocate(1);
    } else if constexpr (std::is_same_v<T, F128Ty>) {
      return Arena<F128Ty>().allocate(1);
    } else if constexpr (std::is_same_v<T, VoidTy>) {
      return Arena<VoidTy>().allocate(1);
    } else {
      static_assert(!std::is_same_v<T, T>,
                    "qxir::getType<T>(): Can not construct immuntable of this type.");
    }
  }
}  // namespace qxir

#endif

#endif  // __QUIX_QXIR_NODE_H__
