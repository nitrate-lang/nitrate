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

#ifndef __QUIX_PARSER_NODE_H__
#define __QUIX_PARSER_NODE_H__

/**
 * @brief Quixcc abstract syntax tree node.
 */
typedef struct qparse_node_t qparse_node_t;

/**
 * @brief Quixcc abstract syntax tree node type.
 */
typedef enum qparse_ty_t {
  QAST_NODE_STMT = 001,
  QAST_NODE_TYPE = 002,
  QAST_NODE_DECL = 003,
  QAST_NODE_EXPR = 004,
  QAST_NODE_CEXPR = 010,

  QAST_NODE_BINEXPR = 100,
  QAST_NODE_UNEXPR = 101,
  QAST_NODE_TEREXPR = 102,
  QAST_NODE_INT = 400,
  QAST_NODE_FLOAT = 401,
  QAST_NODE_STRING = 402,
  QAST_NODE_CHAR = 403,
  QAST_NODE_BOOL = 404,
  QAST_NODE_NULL = 405,
  QAST_NODE_UNDEF = 406,
  QAST_NODE_CALL = 116,
  QAST_NODE_LIST = 117,
  QAST_NODE_ASSOC = 118,
  QAST_NODE_FIELD = 119,
  QAST_NODE_INDEX = 120,
  QAST_NODE_SLICE = 121,
  QAST_NODE_FSTRING = 122,
  QAST_NODE_IDENT = 123,
  QAST_NODE_SEQ_POINT = 124,
  QAST_NODE_POST_UNEXPR = 125,
  QAST_NODE_STMT_EXPR = 126,
  QAST_NODE_TYPE_EXPR = 127,
  QAST_NODE_TEMPL_CALL = 128,

  QAST_NODE_MUT_TY = 300,
  QAST_NODE_U1_TY = 301,
  QAST_NODE_U8_TY = 302,
  QAST_NODE_U16_TY = 303,
  QAST_NODE_U32_TY = 304,
  QAST_NODE_U64_TY = 305,
  QAST_NODE_U128_TY = 306,
  QAST_NODE_I8_TY = 307,
  QAST_NODE_I16_TY = 308,
  QAST_NODE_I32_TY = 309,
  QAST_NODE_I64_TY = 310,
  QAST_NODE_I128_TY = 311,
  QAST_NODE_F32_TY = 312,
  QAST_NODE_F64_TY = 313,
  QAST_NODE_VOID_TY = 314,
  QAST_NODE_PTR_TY = 315,
  QAST_NODE_OPAQUE_TY = 316,
  QAST_NODE_STRING_TY = 317,
  QAST_NODE_ENUM_TY = 318,
  QAST_NODE_STRUCT_TY = 319,
  QAST_NODE_GROUP_TY = 320,
  QAST_NODE_REGION_TY = 321,
  QAST_NODE_UNION_TY = 322,
  QAST_NODE_ARRAY_TY = 323,
  QAST_NODE_VECTOR_TY = 324,
  QAST_NODE_MAP_TY = 325,
  QAST_NODE_TUPLE_TY = 326,
  QAST_NODE_SET_TY = 327,
  QAST_NODE_RESULT_TY = 328,
  QAST_NODE_FN_TY = 329,
  QAST_NODE_UNRES_TY = 330,

  QAST_NODE_TYPEDEF = 500,
  QAST_NODE_FNDECL = 501,
  QAST_NODE_STRUCT = 502,
  QAST_NODE_REGION = 504,
  QAST_NODE_GROUP = 506,
  QAST_NODE_UNION = 508,
  QAST_NODE_ENUM = 510,
  QAST_NODE_FN = 512,
  QAST_NODE_SUBSYSTEM = 514,
  QAST_NODE_EXPORT = 515,
  QAST_NODE_COMPOSITE_FIELD = 516,

  QAST_NODE_BLOCK = 700,
  QAST_NODE_CONST = 701,
  QAST_NODE_VAR = 702,
  QAST_NODE_LET = 703,
  QAST_NODE_INLINE_ASM = 704,
  QAST_NODE_RETURN = 705,
  QAST_NODE_RETIF = 706,
  QAST_NODE_RETZ = 707,
  QAST_NODE_RETV = 708,
  QAST_NODE_BREAK = 709,
  QAST_NODE_CONTINUE = 710,
  QAST_NODE_IF = 711,
  QAST_NODE_WHILE = 712,
  QAST_NODE_FOR = 713,
  QAST_NODE_FORM = 714,
  QAST_NODE_FOREACH = 715,
  QAST_NODE_CASE = 716,
  QAST_NODE_SWITCH = 717,
  QAST_NODE_SLIST = 718,
  QAST_NODE_EXPR_STMT = 719,
  QAST_NODE_VOLSTMT = 720,
} qparse_ty_t;

#define QAST_NODE_COUNT 91

typedef enum qparse_ast_ftype_t {
  QAST_FIELD_BOOL = 0,
  QAST_FIELD_CHAR = 1,
  QAST_FIELD_INT8 = 2,
  QAST_FIELD_INT16 = 3,
  QAST_FIELD_INT32 = 4,
  QAST_FIELD_INT64 = 5,
  QAST_FIELD_UINT8 = 6,
  QAST_FIELD_UINT16 = 7,
  QAST_FIELD_UINT32 = 8,
  QAST_FIELD_UINT64 = 9,
  QAST_FIELD_FLOAT = 10,
  QAST_FIELD_DOUBLE = 11,
  QAST_FIELD_QSTRING = 12,
  QAST_FIELD_QVEC = 13
} qparse_ast_ftype_t;

typedef struct qparse_node_t qparse_node_t;

///=============================================================================
/// END: ABSTRACT SYNTAX TREE DATA TYPES
///=============================================================================

#if (defined(__cplusplus) && defined(QPARSE_USE_CPP_API)) || defined(__QPARSE_IMPL__)

#include <quix-core/Arena.h>
#include <quix-core/Error.h>
#include <quix-lexer/Token.h>

#include <cassert>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <ostream>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>
#include <variant>
#include <vector>

namespace qparse {
  class ArenaAllocatorImpl {
    /// WARNING: This must be the first member; C bindings use
    /// qparse_ast_arena as of type `qcore_arena_t`.
    qcore_arena_t m_arena;

  public:
    ArenaAllocatorImpl();
    ~ArenaAllocatorImpl();

    void *allocate(std::size_t bytes);
    void deallocate(void *ptr) noexcept;

    void swap(qcore_arena_t &arena) { std::swap(m_arena, arena); }

    qcore_arena_t &get() { return m_arena; }
  };

  extern "C" thread_local ArenaAllocatorImpl qparse_ast_arena;

  template <class T>
  struct Arena {
    typedef T value_type;

    Arena() = default;

    template <class U>
    constexpr Arena(const Arena<U> &) noexcept {}

    [[nodiscard]] T *allocate(std::size_t n) {
      return static_cast<T *>(qparse_ast_arena.allocate(sizeof(T) * n));
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

  class AstError : public std::runtime_error {
  public:
    AstError(const std::string &msg) : std::runtime_error("QAST Error: " + msg) {}
  };

  class AstIllegalOperation : public AstError {
  public:
    AstIllegalOperation(const std::string &msg) : AstError("QAST Illegal Operation: " + msg) {}
  };

  class InvariantViolation : public AstError {
  public:
    InvariantViolation(const std::string &msg) : AstError("QAST Invariant Violation: " + msg) {}
  };

  class EFac final {
    EFac() = delete;

  public:
    static AstError error(const std::string &msg) { return AstError(msg); }
    static AstIllegalOperation illegal(const std::string &msg) { return AstIllegalOperation(msg); }
    static InvariantViolation never(const std::string &msg) { return InvariantViolation(msg); }
  };
};  // namespace qparse

#define NODE_IMPL_CORE(__typename)                                                         \
protected:                                                                                 \
  virtual bool verify_impl(std::ostream &os) const override;                               \
                                                                                           \
protected:                                                                                 \
  virtual void canonicalize_impl() override;                                               \
                                                                                           \
protected:                                                                                 \
  virtual void print_impl(std::ostream &os, bool debug) const override;                    \
                                                                                           \
public:                                                                                    \
  virtual __typename *clone_impl() const override;                                         \
                                                                                           \
public:                                                                                    \
public:                                                                                    \
  template <typename T = __typename, typename... Args>                                     \
  static __typename *get(Args &&...args) {                                                 \
    void *ptr = Arena<__typename>().allocate(1);                                           \
    return new (ptr) __typename(std::forward<Args>(args)...);                              \
  }                                                                                        \
                                                                                           \
public:                                                                                    \
  virtual __typename *clone(ArenaAllocatorImpl &arena = qparse_ast_arena) const override { \
    ArenaAllocatorImpl old = qparse_ast_arena;                                             \
    qparse_ast_arena = arena;                                                              \
    __typename *node = clone_impl();                                                       \
    qparse_ast_arena = old;                                                                \
    return node;                                                                           \
  }

class qparse_node_t {
public:
  qparse_node_t() = default;
};

namespace qparse {
  enum class Visibility {
    PUBLIC,
    PRIVATE,
    PROTECTED,
  };

  enum class ExportLang {
    Default,
    C,
    CXX,
    DLang,
    None, /* Internal */
  };

  class String : public std::basic_string<char, std::char_traits<char>, Arena<char>> {
  public:
    String() = default;
    String(const char *str) : std::basic_string<char, std::char_traits<char>, Arena<char>>(str) {}
    String(const std::string &str)
        : std::basic_string<char, std::char_traits<char>, Arena<char>>(str.c_str(), str.size()) {}
  };

  class Node : public qparse_node_t {
  protected:
    virtual bool verify_impl(std::ostream &os) const = 0;
    virtual void canonicalize_impl() = 0;
    virtual void print_impl(std::ostream &os, bool debug) const = 0;
    virtual Node *clone_impl() const = 0;

  public:
    Node() = default;

    uint32_t this_sizeof() const;
    qparse_ty_t this_typeid() const;
    const char *this_nameof() const;

    bool is_type() const;
    bool is_stmt() const;
    bool is_decl() const;
    bool is_expr() const;
    bool is_const_expr() const;

    std::string to_string(bool minify = false, bool binary_repr = false) const;

    template <typename T>
    const T *as() const {
#if !defined(NDEBUG)
      auto p = dynamic_cast<const T *>(this);

      if (!p) {
        const char *this_str = typeid(*this).name();
        const char *other_str = typeid(T).name();

        qcore_panicf(
            "qparse_node_t::as(const %s *this): Invalid cast from `%s` to "
            "`%s`.",
            this_str, this_str, other_str);
        __builtin_unreachable();
      }
      return p;
#else
      return reinterpret_cast<const T *>(this);
#endif
    }

    template <typename T>
    T *as() {
#if !defined(NDEBUG)
      auto p = dynamic_cast<T *>(this);

      if (!p) {
        const char *this_str = typeid(*this).name();
        const char *other_str = typeid(T).name();

        qcore_panicf("qparse_node_t::as(%s *this): Invalid cast from `%s` to `%s`.", this_str,
                     this_str, other_str);
        __builtin_unreachable();
      }
      return p;
#else
      return reinterpret_cast<T *>(this);
#endif
    }

    template <typename T>
    bool is() const {
      return typeid(*this) == typeid(T);
    }

    bool is(const qparse_ty_t type) const;
    bool verify(std::ostream &os = std::cerr) const;
    void canonicalize();
    virtual Node *clone(ArenaAllocatorImpl &arena = qparse_ast_arena) const = 0;

    static const char *type_name(qparse_ty_t type);

    void dump(bool isForDebug = false) const { print_impl(std::cerr, isForDebug); }

    void print(std::ostream &os, bool isForDebug = false) const { print_impl(os, isForDebug); }
  };

  class Stmt : public Node {
  public:
    Stmt() = default;

    virtual Stmt *clone(ArenaAllocatorImpl &arena = qparse_ast_arena) const = 0;
  };

  class Type : public Node {
  protected:
    bool m_volatile;

  public:
    Type(bool is_volatile = false) : m_volatile(is_volatile) {}

    bool is_primitive() const;
    bool is_array() const;
    bool is_vector() const;
    bool is_tuple() const;
    bool is_set() const;
    bool is_map() const;
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
    bool is_mutable() const;
    bool is_const() const;
    bool is_volatile() const;
    bool is_ptr_to(const Type *type) const;
    bool is_mut_ptr_to(const Type *type) const;
    bool is_const_ptr_to(const Type *type) const;
    bool is_ptr_to_mut(const Type *type) const;
    bool is_ptr_to_const(const Type *type) const;
    bool is_mut_ptr_to_mut(const Type *type) const;
    bool is_mut_ptr_to_const(const Type *type) const;
    bool is_const_ptr_to_mut(const Type *type) const;
    bool is_const_ptr_to_const(const Type *type) const;
    bool is_string() const;

    virtual Type *clone(ArenaAllocatorImpl &arena = qparse_ast_arena) const = 0;
  };

  typedef std::set<String, std::less<String>, Arena<String>> DeclTags;

  class Decl : public Stmt {
  protected:
    DeclTags m_tags;
    String m_name;
    Type *m_type;
    Visibility m_visibility;

  public:
    Decl(String name = "", Type *type = nullptr, std::initializer_list<String> tags = {},
         Visibility visibility = Visibility::PRIVATE)
        : m_tags(tags), m_name(name), m_type(type), m_visibility(visibility) {}

    String get_name() const;
    void set_name(String name);

    virtual Type *get_type() const;
    void set_type(Type *type);

    DeclTags &get_tags();
    void add_tag(String tag);
    void add_tags(std::set<std::string> tags);
    void clear_tags();
    void remove_tag(String tag);

    Visibility get_visibility() const;
    void set_visibility(Visibility visibility);

    virtual Decl *clone(ArenaAllocatorImpl &arena = qparse_ast_arena) const = 0;
  };

  class Expr : public Node {
  protected:
    Type *m_type;

  public:
    Expr() : m_type(nullptr) {}

    bool is_binexpr() const;
    bool is_unaryexpr() const;
    bool is_ternaryexpr() const;

    virtual Expr *clone(ArenaAllocatorImpl &arena = qparse_ast_arena) const = 0;
  };

  class ExprStmt : public Stmt {
  protected:
    Expr *m_expr;

  public:
    ExprStmt(Expr *expr = nullptr) : m_expr(expr) {}

    Expr *get_expr() const;
    void set_expr(Expr *expr);

    NODE_IMPL_CORE(ExprStmt)
  };

  class StmtExpr : public Expr {
  protected:
    Stmt *m_stmt;

  public:
    StmtExpr(Stmt *stmt = nullptr) : m_stmt(stmt) {}

    Stmt *get_stmt() const;
    void set_stmt(Stmt *stmt);

    NODE_IMPL_CORE(StmtExpr)
  };

  class TypeExpr : public Expr {
  protected:
    Type *m_type;

  public:
    TypeExpr(Type *type = nullptr) : m_type(type) {}

    Type *get_type() const;
    void set_type(Type *type);

    NODE_IMPL_CORE(TypeExpr)
  };

  class ConstExpr : public Expr {
  protected:
    Expr *m_value;

  public:
    ConstExpr(Expr *value = nullptr) : m_value(value) {}

    Expr *get_value() const;
    void set_value(Expr *value);

    NODE_IMPL_CORE(ConstExpr)
  };

  class LitExpr : public ConstExpr {
  public:
    LitExpr() = default;

    virtual LitExpr *clone(ArenaAllocatorImpl &arena = qparse_ast_arena) const = 0;
  };

  class FlowStmt : public Stmt {
  public:
    FlowStmt() = default;

    virtual FlowStmt *clone(ArenaAllocatorImpl &arena = qparse_ast_arena) const = 0;
  };

  class DeclStmt : public Stmt {
  public:
    DeclStmt() = default;

    virtual DeclStmt *clone(ArenaAllocatorImpl &arena = qparse_ast_arena) const = 0;
  };

  class TypeBuiltin : public Type {
  public:
    TypeBuiltin() = default;

    virtual TypeBuiltin *clone(ArenaAllocatorImpl &arena = qparse_ast_arena) const = 0;
  };

  class TypeComplex : public Type {
  public:
    TypeComplex() = default;

    virtual TypeComplex *clone(ArenaAllocatorImpl &arena = qparse_ast_arena) const = 0;
  };

  class TypeComposite : public Type {
  public:
    TypeComposite() = default;

    virtual TypeComposite *clone(ArenaAllocatorImpl &arena = qparse_ast_arena) const = 0;
  };

  class UnresolvedType : public Type {
    String m_name;

  public:
    UnresolvedType(String name = "") : m_name(name) {}

    String get_name() const;
    void set_name(String name);

    NODE_IMPL_CORE(UnresolvedType)
  };

  class U1 : public TypeBuiltin {
  public:
    U1() = default;

    NODE_IMPL_CORE(U1)
  };

  class U8 : public TypeBuiltin {
  public:
    U8() = default;

    NODE_IMPL_CORE(U8)
  };

  class U16 : public TypeBuiltin {
  public:
    U16() = default;

    NODE_IMPL_CORE(U16)
  };

  class U32 : public TypeBuiltin {
  public:
    U32() = default;

    NODE_IMPL_CORE(U32)
  };

  class U64 : public TypeBuiltin {
  public:
    U64() = default;

    NODE_IMPL_CORE(U64)
  };

  class U128 : public TypeBuiltin {
  public:
    U128() = default;

    NODE_IMPL_CORE(U128)
  };

  class I8 : public TypeBuiltin {
  public:
    I8() = default;

    NODE_IMPL_CORE(I8)
  };

  class I16 : public TypeBuiltin {
  public:
    I16() = default;

    NODE_IMPL_CORE(I16)
  };

  class I32 : public TypeBuiltin {
  public:
    I32() = default;

    NODE_IMPL_CORE(I32)
  };

  class I64 : public TypeBuiltin {
  public:
    I64() = default;

    NODE_IMPL_CORE(I64)
  };

  class I128 : public TypeBuiltin {
  public:
    I128() = default;

    NODE_IMPL_CORE(I128)
  };

  class F32 : public TypeBuiltin {
  public:
    F32() = default;

    NODE_IMPL_CORE(F32)
  };

  class F64 : public TypeBuiltin {
  public:
    F64() = default;

    NODE_IMPL_CORE(F64)
  };

  class VoidTy : public TypeBuiltin {
  public:
    VoidTy() = default;

    NODE_IMPL_CORE(VoidTy)
  };

  class StringTy : public TypeBuiltin {
  public:
    StringTy() = default;

    NODE_IMPL_CORE(StringTy)
  };

  class PtrTy : public TypeComplex {
    Type *m_item;
    bool m_is_volatile;

  public:
    PtrTy(Type *item = nullptr, bool is_volatile = false)
        : m_item(item), m_is_volatile(is_volatile) {}

    Type *get_item() const;
    void set_item(Type *item);

    bool is_volatile() const;
    void set_volatile(bool is_volatile);

    NODE_IMPL_CORE(PtrTy)
  };

  class OpaqueTy : public TypeComplex {
    String m_name;

  public:
    OpaqueTy(String name = "") : m_name(name) {}

    String get_name() const;
    void set_name(String name);

    NODE_IMPL_CORE(OpaqueTy)
  };

  class VectorTy : public TypeComposite {
    Type *m_item;

  public:
    VectorTy(Type *item = nullptr) : m_item(item) {}

    Type *get_item() const;
    void set_item(Type *item);

    NODE_IMPL_CORE(VectorTy)
  };

  class SetTy : public TypeComposite {
    Type *m_item;

  public:
    SetTy(Type *item = nullptr) : m_item(item) {}

    Type *get_item() const;
    void set_item(Type *item);

    NODE_IMPL_CORE(SetTy)
  };

  class MapTy : public TypeComposite {
    Type *m_key;
    Type *m_value;

  public:
    MapTy(Type *key = nullptr, Type *value = nullptr) : m_key(key), m_value(value) {}

    Type *get_key() const;
    void set_key(Type *key);

    Type *get_value() const;
    void set_value(Type *value);

    NODE_IMPL_CORE(MapTy)
  };

  typedef std::vector<Type *, Arena<Type *>> TupleTyItems;
  class TupleTy : public TypeComposite {
    TupleTyItems m_items;

  public:
    TupleTy(std::initializer_list<Type *> items = {}) : m_items(items) {}
    TupleTy(const TupleTyItems &items) : m_items(items) {}

    TupleTyItems &get_items();
    void add_item(Type *item);
    void add_items(std::initializer_list<Type *> items);
    void clear_items();
    void remove_item(Type *item);

    NODE_IMPL_CORE(TupleTy)
  };

  class OptionalTy : public TypeComposite {
    Type *m_item;

  public:
    OptionalTy(Type *item = nullptr) : m_item(item) {}

    Type *get_item() const;
    void set_item(Type *item);

    NODE_IMPL_CORE(OptionalTy)
  };

  class ArrayTy : public TypeComposite {
    Type *m_item;
    ConstExpr *m_size;

  public:
    ArrayTy(Type *item = nullptr, ConstExpr *size = nullptr) : m_item(item), m_size(size) {}

    Type *get_item() const;
    void set_item(Type *item);

    ConstExpr *get_size() const;
    void set_size(ConstExpr *size);

    NODE_IMPL_CORE(ArrayTy)
  };

  class EnumTy : public TypeComplex {
    String m_name;
    Type *m_memtype;

  public:
    EnumTy(String name = "", Type *memtype = nullptr) : m_name(name), m_memtype(memtype) {}

    String get_name() const;
    void set_name(String name);

    Type *get_memtype() const;
    void set_memtype(Type *memtype);

    NODE_IMPL_CORE(EnumTy)
  };

  class MutTy : public TypeComplex {
    Type *m_item;

  public:
    MutTy(Type *item = nullptr) : m_item(item) {}

    Type *get_item() const;
    void set_item(Type *item);

    NODE_IMPL_CORE(MutTy)
  };

  typedef std::pair<String, Type *> StructItem;
  typedef std::vector<StructItem, Arena<StructItem>> StructItems;

  class StructTy : public TypeComposite {
    StructItems m_items;

  public:
    StructTy(std::initializer_list<StructItem> items = {}) : m_items(items) {}
    StructTy(const StructItems &items) : m_items(items) {}

    StructItems &get_items();
    void add_item(String name, Type *type);
    void add_items(std::initializer_list<StructItem> items);
    void clear_items();
    void remove_item(String name);

    NODE_IMPL_CORE(StructTy)
  };

  typedef std::vector<Type *, Arena<Type *>> GroupTyItems;

  class GroupTy : public TypeComposite {
    GroupTyItems m_items;

  public:
    GroupTy(std::initializer_list<Type *> items = {}) : m_items(items) {}
    GroupTy(const GroupTyItems &items) : m_items(items) {}

    GroupTyItems &get_items();
    void add_item(Type *item);
    void add_items(std::initializer_list<Type *> items);
    void clear_items();
    void remove_item(Type *item);

    NODE_IMPL_CORE(GroupTy)
  };

  typedef std::vector<Type *, Arena<Type *>> RegionTyItems;

  class RegionTy : public TypeComposite {
    RegionTyItems m_items;

  public:
    RegionTy(std::initializer_list<Type *> items = {}) : m_items(items) {}
    RegionTy(const RegionTyItems &items) : m_items(items) {}

    RegionTyItems &get_items();
    void add_item(Type *item);
    void add_items(std::initializer_list<Type *> items);
    void clear_items();
    void remove_item(Type *item);

    NODE_IMPL_CORE(RegionTy)
  };

  typedef std::vector<Type *, Arena<Type *>> UnionTyItems;

  class UnionTy : public TypeComposite {
    UnionTyItems m_items;

  public:
    UnionTy(std::initializer_list<Type *> items = {}) : m_items(items) {}
    UnionTy(const UnionTyItems &items) : m_items(items) {}

    UnionTyItems &get_items();
    void add_item(Type *item);
    void add_items(std::initializer_list<Type *> items);
    void clear_items();
    void remove_item(Type *item);

    NODE_IMPL_CORE(UnionTy)
  };

  enum class FuncPurity {
    IMPURE_THREAD_UNSAFE,
    IMPURE_THREAD_SAFE,
    PURE,
    QUASIPURE,
    RETROPURE,
  };

  typedef std::tuple<String, Type *, Expr *> FuncParam;
  typedef std::vector<FuncParam, Arena<FuncParam>> FuncParams;

  class FuncTy : public TypeComplex {
    FuncParams m_params;
    Type *m_return;
    FuncPurity m_purity;
    bool m_variadic;
    bool m_is_foreign;
    bool m_crashpoint;
    bool m_noexcept;
    bool m_noreturn;

  public:
    FuncTy()
        : m_return(nullptr),
          m_purity(FuncPurity::IMPURE_THREAD_UNSAFE),
          m_variadic(false),
          m_is_foreign(false),
          m_crashpoint(false),
          m_noexcept(false),
          m_noreturn(false) {}
    FuncTy(Type *return_type, FuncParams parameters, bool variadic = false,
           FuncPurity purity = FuncPurity::IMPURE_THREAD_UNSAFE, bool is_foreign = false,
           bool crashpoint = false, bool noexcept_ = false, bool noreturn = false)
        : m_params(parameters),
          m_return(return_type),
          m_purity(purity),
          m_variadic(variadic),
          m_is_foreign(is_foreign),
          m_crashpoint(crashpoint),
          m_noexcept(noexcept_),
          m_noreturn(noreturn) {
      assert(!noreturn || (purity == FuncPurity::IMPURE_THREAD_UNSAFE ||
                           purity == FuncPurity::IMPURE_THREAD_SAFE));
    }
    FuncTy(Type *return_type, std::vector<Type *, Arena<Type *>> parameters, bool variadic = false,
           FuncPurity purity = FuncPurity::IMPURE_THREAD_UNSAFE, bool is_foreign = false,
           bool crashpoint = false, bool noexcept_ = false, bool noreturn = false)
        : m_return(return_type),
          m_purity(purity),
          m_variadic(variadic),
          m_is_foreign(is_foreign),
          m_crashpoint(crashpoint),
          m_noexcept(noexcept_),
          m_noreturn(noreturn) {
      assert(!noreturn || (purity == FuncPurity::IMPURE_THREAD_UNSAFE ||
                           purity == FuncPurity::IMPURE_THREAD_SAFE));

      for (size_t i = 0; i < parameters.size(); i++) {
        m_params.push_back(FuncParam("_" + std::to_string(i), parameters[i], nullptr));
      }
    }

    bool is_noreturn() const;
    void set_noreturn(bool noreturn);

    Type *get_return_ty() const;
    void set_return_ty(Type *return_ty);

    FuncParams &get_params();
    void add_param(String name, Type *type, Expr *default_val = nullptr);
    void add_params(std::initializer_list<FuncParam> params);
    void clear_params();
    void remove_param(String name);

    FuncPurity get_purity() const;
    void set_purity(FuncPurity purity);

    bool is_variadic() const;
    void set_variadic(bool variadic);

    bool is_foreign() const;
    void set_foreign(bool is_foreign);

    bool is_crashpoint() const;
    void set_crashpoint(bool crashpoint);

    bool is_noexcept() const;
    void set_noexcept(bool noexcept_);

    NODE_IMPL_CORE(FuncTy)
  };

  ///=============================================================================

  class UnaryExpr : public Expr {
  protected:
    Expr *m_rhs;
    qlex_op_t m_op;

  public:
    UnaryExpr(qlex_op_t op = qOpUnknown, Expr *rhs = nullptr) : m_rhs(rhs), m_op(op) {}

    Expr *get_rhs() const;
    void set_rhs(Expr *rhs);

    qlex_op_t get_op() const;
    void set_op(qlex_op_t op);

    NODE_IMPL_CORE(UnaryExpr)
  };

  class BinExpr : public Expr {
  protected:
    Expr *m_lhs;
    Expr *m_rhs;
    qlex_op_t m_op;

  public:
    BinExpr(Expr *lhs = nullptr, qlex_op_t op = qOpUnknown, Expr *rhs = nullptr)
        : m_lhs(lhs), m_rhs(rhs), m_op(op) {}

    Expr *get_lhs() const;
    void set_lhs(Expr *lhs);

    Expr *get_rhs() const;
    void set_rhs(Expr *rhs);

    qlex_op_t get_op() const;
    void set_op(qlex_op_t op);

    NODE_IMPL_CORE(BinExpr)
  };

  class PostUnaryExpr : public Expr {
  protected:
    Expr *m_lhs;
    qlex_op_t m_op;

  public:
    PostUnaryExpr(Expr *lhs = nullptr, qlex_op_t op = qOpUnknown) : m_lhs(lhs), m_op(op) {}

    Expr *get_lhs() const;
    void set_lhs(Expr *lhs);

    qlex_op_t get_op() const;
    void set_op(qlex_op_t op);

    NODE_IMPL_CORE(PostUnaryExpr)
  };

  class TernaryExpr : public Expr {
  protected:
    Expr *m_cond;
    Expr *m_lhs;
    Expr *m_rhs;

  public:
    TernaryExpr(Expr *cond = nullptr, Expr *lhs = nullptr, Expr *rhs = nullptr)
        : m_cond(cond), m_lhs(lhs), m_rhs(rhs) {}

    Expr *get_cond() const;
    void set_cond(Expr *cond);

    Expr *get_lhs() const;
    void set_lhs(Expr *lhs);

    Expr *get_rhs() const;
    void set_rhs(Expr *rhs);

    NODE_IMPL_CORE(TernaryExpr)
  };

  ///=============================================================================

  class ConstInt : public LitExpr {
    String m_value;

  public:
    ConstInt(String value = "") : m_value(value) {}
    ConstInt(uint64_t value) : m_value(std::to_string(value)) {}

    String get_value() const;

    NODE_IMPL_CORE(ConstInt)
  };

  class ConstFloat : public LitExpr {
    String m_value;

  public:
    ConstFloat(String value = "") : m_value(value) {}
    ConstFloat(double value) : m_value(std::to_string(value)) {}

    String get_value() const;

    NODE_IMPL_CORE(ConstFloat)
  };

  class ConstBool : public LitExpr {
    bool m_value;

  public:
    ConstBool(bool value = false) : m_value(value) {}

    bool get_value() const;

    NODE_IMPL_CORE(ConstBool)
  };

  class ConstString : public LitExpr {
    String m_value;

  public:
    ConstString(String value = "") : m_value(value) {}

    String get_value() const;

    NODE_IMPL_CORE(ConstString)
  };

  class ConstChar : public LitExpr {
    String m_value;

  public:
    ConstChar(String value = "") : m_value(value) {}

    String get_value() const;

    NODE_IMPL_CORE(ConstChar)
  };

  class ConstNull : public LitExpr {
  public:
    ConstNull() = default;

    NODE_IMPL_CORE(ConstNull)
  };

  class ConstUndef : public LitExpr {
  public:
    ConstUndef() = default;

    NODE_IMPL_CORE(ConstUndef)
  };

  ///=============================================================================

  typedef std::pair<String, Expr *> CallArg;
  typedef std::vector<CallArg, Arena<CallArg>> CallArgs;

  class Call : public Expr {
  protected:
    Expr *m_func;
    CallArgs m_args;

  public:
    Call(Expr *func = nullptr, CallArgs args = {}) : m_func(func), m_args(args) {}

    Expr *get_func() const;
    void set_func(Expr *func);

    CallArgs &get_args();
    void add_arg(CallArg arg);
    void add_args(std::initializer_list<CallArg> args);
    void clear_args();
    void remove_arg(String name);

    NODE_IMPL_CORE(Call)
  };

  typedef std::map<String, ConstExpr *, std::less<String>,
                   Arena<std::pair<const String, ConstExpr *>>>
      TemplateArgs;

  class TemplCall : public Call {
  protected:
    TemplateArgs m_template_args;
    Expr *m_func;
    CallArgs m_args;

  public:
    TemplCall(Expr *func = nullptr, CallArgs args = {}, TemplateArgs template_args = {})
        : m_template_args(template_args), m_func(func), m_args(args) {}

    Expr *get_func() const;
    void set_func(Expr *func);

    TemplateArgs &get_template_args();
    void add_template_arg(String name, ConstExpr *arg);
    void add_template_args(std::map<String, ConstExpr *> args);
    void clear_template_args();
    void remove_template_arg(String name);

    NODE_IMPL_CORE(TemplCall)
  };

  typedef std::vector<Expr *, Arena<Expr *>> ListData;

  class List : public Expr {
  protected:
    ListData m_items;

  public:
    List(std::initializer_list<Expr *> items = {}) : m_items(items) {}
    List(const ListData &items) : m_items(items) {}

    ListData &get_items();
    void add_item(Expr *item);
    void add_items(std::initializer_list<Expr *> items);
    void clear_items();
    void remove_item(Expr *item);

    NODE_IMPL_CORE(List)
  };

  class Assoc : public Expr {
  protected:
    Expr *m_key;
    Expr *m_value;

  public:
    Assoc(Expr *key = nullptr, Expr *value = nullptr) : m_key(key), m_value(value) {}

    Expr *get_key() const;
    void set_key(Expr *key);

    Expr *get_value() const;
    void set_value(Expr *value);

    NODE_IMPL_CORE(Assoc)
  };

  class Field : public Expr {
  protected:
    Expr *m_base;
    String m_field;

  public:
    Field(Expr *base = nullptr, String field = "") : m_base(base), m_field(field) {}

    Expr *get_base() const;
    void set_base(Expr *base);

    String get_field() const;
    void set_field(String field);

    NODE_IMPL_CORE(Field)
  };

  class Index : public Expr {
  protected:
    Expr *m_base;
    Expr *m_index;

  public:
    Index(Expr *base = nullptr, Expr *index = nullptr) : m_base(base), m_index(index) {}

    Expr *get_base() const;
    void set_base(Expr *base);

    Expr *get_index() const;
    void set_index(Expr *index);

    NODE_IMPL_CORE(Index)
  };

  class Slice : public Expr {
  protected:
    Expr *m_base;
    Expr *m_start;
    Expr *m_end;

  public:
    Slice(Expr *base = nullptr, Expr *start = nullptr, Expr *end = nullptr)
        : m_base(base), m_start(start), m_end(end) {}

    Expr *get_base() const;
    void set_base(Expr *base);

    Expr *get_start() const;
    void set_start(Expr *start);

    Expr *get_end() const;
    void set_end(Expr *end);

    NODE_IMPL_CORE(Slice)
  };

  typedef std::vector<Expr *, Arena<Expr *>> FStringArgs;

  class FString : public Expr {
  protected:
    String m_value;
    FStringArgs m_items;

  public:
    FString(String value = "", std::initializer_list<Expr *> items = {})
        : m_value(value), m_items(items) {}
    FString(String value, const FStringArgs &items) : m_value(value), m_items(items) {}

    String get_value() const;
    void set_value(String value);

    FStringArgs &get_items();
    void add_item(Expr *item);
    void add_items(std::initializer_list<Expr *> items);
    void clear_items();
    void remove_item(Expr *item);

    NODE_IMPL_CORE(FString)
  };

  class Ident : public Expr {
    String m_name;

  public:
    Ident(String name = "") : m_name(name) {}

    String get_name() const;
    void set_name(String name);

    NODE_IMPL_CORE(Ident)
  };

  typedef std::vector<Expr *, Arena<Expr *>> SeqPointItems;
  class SeqPoint : public Expr {
  protected:
    SeqPointItems m_items;

  public:
    SeqPoint(std::initializer_list<Expr *> items = {}) : m_items(items) {}
    SeqPoint(const SeqPointItems &items) : m_items(items) {}

    SeqPointItems &get_items();
    void add_item(Expr *item);
    void add_items(std::initializer_list<Expr *> items);
    void clear_items();
    void remove_item(Expr *item);

    NODE_IMPL_CORE(SeqPoint)
  };

  ///=============================================================================

  typedef std::vector<Stmt *, Arena<Stmt *>> BlockItems;
  class Block : public Stmt {
  protected:
    BlockItems m_items;
    bool m_unsafe;

  public:
    Block(std::initializer_list<Stmt *> items = {}) : m_items(items), m_unsafe(false) {}
    Block(const BlockItems &items) : m_items(items), m_unsafe(false) {}

    BlockItems &get_items();
    void add_item(Stmt *item);
    void add_items(std::initializer_list<Stmt *> items);
    void clear_items();
    void remove_item(Stmt *item);

    bool is_unsafe() const;
    void set_unsafe(bool unsafe);

    NODE_IMPL_CORE(Block)
  };

  class VolStmt : public Stmt {
  protected:
    Stmt *m_stmt;

  public:
    VolStmt(Stmt *stmt = nullptr) : m_stmt(stmt) {}

    Stmt *get_stmt() const;
    void set_stmt(Stmt *stmt);

    NODE_IMPL_CORE(VolStmt)
  };

  typedef std::vector<Stmt *, Arena<Stmt *>> StmtListItems;
  class StmtList : public Stmt {
  protected:
    StmtListItems m_items;

  public:
    StmtList(std::initializer_list<Stmt *> items = {}) : m_items(items) {}
    StmtList(const StmtListItems &items) : m_items(items) {}

    StmtListItems &get_items();
    void add_item(Stmt *item);
    void add_items(std::initializer_list<Stmt *> items);
    void clear_items();
    void remove_item(Stmt *item);

    NODE_IMPL_CORE(StmtList)
  };

  class ConstDecl : public Decl {
  protected:
    Expr *m_value;

  public:
    ConstDecl(String name = "", Type *type = nullptr, Expr *value = nullptr)
        : Decl(name, type), m_value(value) {}

    Expr *get_value() const;
    void set_value(Expr *value);

    NODE_IMPL_CORE(ConstDecl)
  };

  class VarDecl : public Decl {
  protected:
    Expr *m_value;

  public:
    VarDecl(String name = "", Type *type = nullptr, Expr *value = nullptr)
        : Decl(name, type), m_value(value) {}

    Expr *get_value() const;
    void set_value(Expr *value);

    NODE_IMPL_CORE(VarDecl)
  };

  class LetDecl : public Decl {
  protected:
    Expr *m_value;

  public:
    LetDecl(String name = "", Type *type = nullptr, Expr *value = nullptr)
        : Decl(name, type), m_value(value) {}

    Expr *get_value() const;
    void set_value(Expr *value);

    NODE_IMPL_CORE(LetDecl)
  };

  typedef std::vector<Expr *, Arena<Expr *>> InlineAsmArgs;

  class InlineAsm : public Stmt {
  protected:
    String m_code;
    InlineAsmArgs m_args;

  public:
    InlineAsm(String code = "", std::initializer_list<Expr *> args = {})
        : m_code(code), m_args(args) {}
    InlineAsm(String code, const InlineAsmArgs &args) : m_code(code), m_args(args) {}

    String get_code() const;
    void set_code(String code);

    InlineAsmArgs &get_args();
    void add_arg(Expr *arg);
    void add_args(std::initializer_list<Expr *> args);
    void clear_args();
    void remove_arg(Expr *arg);

    NODE_IMPL_CORE(InlineAsm)
  };

  class IfStmt : public FlowStmt {
  protected:
    Expr *m_cond;
    Stmt *m_then;
    Stmt *m_else;

  public:
    IfStmt(Expr *cond = nullptr, Stmt *then = nullptr, Stmt *else_ = nullptr)
        : m_cond(cond), m_then(then), m_else(else_) {}

    Expr *get_cond() const;
    void set_cond(Expr *cond);

    Stmt *get_then() const;
    void set_then(Stmt *then);

    Stmt *get_else() const;
    void set_else(Stmt *else_);

    NODE_IMPL_CORE(IfStmt)
  };

  class WhileStmt : public FlowStmt {
  protected:
    Expr *m_cond;
    Stmt *m_body;

  public:
    WhileStmt(Expr *cond = nullptr, Stmt *body = nullptr) : m_cond(cond), m_body(body) {}

    Expr *get_cond() const;
    void set_cond(Expr *cond);

    Stmt *get_body() const;
    void set_body(Stmt *body);

    NODE_IMPL_CORE(WhileStmt)
  };

  class ForStmt : public FlowStmt {
  protected:
    Stmt *m_init;
    Expr *m_cond;
    Expr *m_step;
    Block *m_body;

  public:
    ForStmt(Stmt *init = nullptr, Expr *cond = nullptr, Expr *step = nullptr, Block *body = nullptr)
        : m_init(init), m_cond(cond), m_step(step), m_body(body) {}

    Stmt *get_init() const;
    void set_init(Stmt *init);

    Expr *get_cond() const;
    void set_cond(Expr *cond);

    Expr *get_step() const;
    void set_step(Expr *step);

    Block *get_body() const;
    void set_body(Block *body);

    NODE_IMPL_CORE(ForStmt)
  };

  class FormStmt : public FlowStmt {
  protected:
    String m_idx_ident;
    String m_val_ident;
    Expr *m_expr;
    Expr *m_maxjobs;
    Stmt *m_body;

  public:
    FormStmt(String idx_ident = "", String val_ident = "", Expr *expr = nullptr,
             Expr *maxjobs = nullptr, Stmt *body = nullptr)
        : m_idx_ident(idx_ident),
          m_val_ident(val_ident),
          m_expr(expr),
          m_maxjobs(maxjobs),
          m_body(body) {}

    String get_idx_ident() const;
    void set_idx_ident(String idx_ident);

    String get_val_ident() const;
    void set_val_ident(String val_ident);

    Expr *get_expr() const;
    void set_expr(Expr *expr);

    Expr *get_maxjobs() const;
    void set_maxjobs(Expr *maxjobs);

    Stmt *get_body() const;
    void set_body(Stmt *body);

    NODE_IMPL_CORE(FormStmt)
  };

  class ForeachStmt : public FlowStmt {
  protected:
    String m_idx_ident;
    String m_val_ident;
    Expr *m_expr;
    Stmt *m_body;

  public:
    ForeachStmt(String idx_ident = "", String val_ident = "", Expr *expr = nullptr,
                Stmt *body = nullptr)
        : m_idx_ident(idx_ident), m_val_ident(val_ident), m_expr(expr), m_body(body) {}

    String get_idx_ident() const;
    void set_idx_ident(String idx_ident);

    String get_val_ident() const;
    void set_val_ident(String val_ident);

    Expr *get_expr() const;
    void set_expr(Expr *expr);

    Stmt *get_body() const;
    void set_body(Stmt *body);

    NODE_IMPL_CORE(ForeachStmt)
  };

  class BreakStmt : public FlowStmt {
  public:
    BreakStmt() = default;

    NODE_IMPL_CORE(BreakStmt)
  };

  class ContinueStmt : public FlowStmt {
  public:
    ContinueStmt() = default;

    NODE_IMPL_CORE(ContinueStmt)
  };

  class ReturnStmt : public FlowStmt {
  protected:
    Expr *m_value;

  public:
    ReturnStmt(Expr *value = nullptr) : m_value(value) {}

    Expr *get_value() const;
    void set_value(Expr *value);

    NODE_IMPL_CORE(ReturnStmt)
  };

  class ReturnIfStmt : public FlowStmt {
  protected:
    Expr *m_cond;
    Expr *m_value;

  public:
    ReturnIfStmt(Expr *cond = nullptr, Expr *value = nullptr) : m_cond(cond), m_value(value) {}

    Expr *get_cond() const;
    void set_cond(Expr *cond);

    Expr *get_value() const;
    void set_value(Expr *value);

    NODE_IMPL_CORE(ReturnIfStmt)
  };

  class RetZStmt : public FlowStmt {
  protected:
    Expr *m_cond;
    Expr *m_value;

  public:
    RetZStmt(Expr *cond = nullptr, Expr *value = nullptr) : m_cond(cond), m_value(value) {}

    Expr *get_cond() const;
    void set_cond(Expr *cond);

    Expr *get_value() const;
    void set_value(Expr *value);

    NODE_IMPL_CORE(RetZStmt)
  };

  class RetVStmt : public FlowStmt {
  protected:
    Expr *m_cond;

  public:
    RetVStmt(Expr *cond = nullptr) : m_cond(cond) {}

    Expr *get_cond() const;
    void set_cond(Expr *cond);

    NODE_IMPL_CORE(RetVStmt)
  };

  class CaseStmt : public FlowStmt {
  protected:
    Expr *m_cond;
    Stmt *m_body;

  public:
    CaseStmt(Expr *cond = nullptr, Stmt *body = nullptr) : m_cond(cond), m_body(body) {}

    Expr *get_cond() const;
    void set_cond(Expr *cond);

    Stmt *get_body() const;
    void set_body(Stmt *body);

    NODE_IMPL_CORE(CaseStmt)
  };

  typedef std::vector<CaseStmt *, Arena<CaseStmt *>> SwitchCases;
  class SwitchStmt : public FlowStmt {
  protected:
    Expr *m_cond;
    SwitchCases m_cases;
    Stmt *m_default;

  public:
    SwitchStmt(Expr *cond = nullptr, std::initializer_list<CaseStmt *> cases = {},
               Stmt *default_ = nullptr)
        : m_cond(cond), m_cases(cases), m_default(default_) {}
    SwitchStmt(Expr *cond, const SwitchCases &cases, Stmt *default_)
        : m_cond(cond), m_cases(cases), m_default(default_) {}

    Expr *get_cond() const;
    void set_cond(Expr *cond);

    SwitchCases &get_cases();
    void add_case(CaseStmt *case_);
    void add_cases(std::initializer_list<CaseStmt *> cases);
    void clear_cases();
    void remove_case(CaseStmt *case_);

    Stmt *get_default() const;
    void set_default(Stmt *default_);

    NODE_IMPL_CORE(SwitchStmt)
  };

  ///=============================================================================

  class TypedefDecl : public Decl {
  protected:
  public:
    TypedefDecl(String name = "", Type *type = nullptr) : Decl(name, type) {}

    NODE_IMPL_CORE(TypedefDecl)
  };

  class FnDecl : public Decl {
  protected:
  public:
    FnDecl(String name = "", FuncTy *type = nullptr) : Decl(name, type) {}

    virtual FuncTy *get_type() const override;

    NODE_IMPL_CORE(FnDecl)
  };

  class FnDef : public FnDecl {
  protected:
    Stmt *m_body;
    Expr *m_precond;
    Expr *m_postcond;

  public:
    FnDef(FnDecl *decl = nullptr, Stmt *body = nullptr, Expr *precond = nullptr,
          Expr *postcond = nullptr)
        : FnDecl(decl->get_name(), decl->get_type()),
          m_body(body),
          m_precond(precond),
          m_postcond(postcond) {}

    Stmt *get_body() const;
    void set_body(Stmt *body);

    Expr *get_precond() const;
    void set_precond(Expr *precond);

    Expr *get_postcond() const;
    void set_postcond(Expr *postcond);

    NODE_IMPL_CORE(FnDef)
  };

  class CompositeField : public Decl {
  protected:
    Expr *m_value;

  public:
    CompositeField(String name = "", Type *type = nullptr, Expr *value = nullptr)
        : Decl(name, type), m_value(value) {}

    Expr *get_value() const;
    void set_value(Expr *value);

    NODE_IMPL_CORE(CompositeField)
  };

  typedef std::vector<CompositeField *, Arena<CompositeField *>> StructDefFields;
  typedef std::vector<FnDecl *, Arena<FnDecl *>> StructDefMethods;
  typedef std::vector<FnDecl *, Arena<FnDecl *>> StructDefStaticMethods;

  class StructDef : public Decl {
  protected:
    StructDefMethods m_methods;
    StructDefStaticMethods m_static_methods;
    StructDefFields m_fields;

  public:
    StructDef(String name = "", StructTy *type = nullptr,
              std::initializer_list<CompositeField *> fields = {},
              std::initializer_list<FnDecl *> methods = {},
              std::initializer_list<FnDecl *> static_methods = {})
        : Decl(name, type),
          m_methods(methods),
          m_static_methods(static_methods),
          m_fields(fields) {}
    StructDef(String name, StructTy *type, const StructDefFields &fields,
              const StructDefMethods &methods, const StructDefStaticMethods &static_methods)
        : Decl(name, type),
          m_methods(methods),
          m_static_methods(static_methods),
          m_fields(fields) {}

    virtual StructTy *get_type() const override;

    StructDefMethods &get_methods();
    void add_method(FnDecl *method);
    void add_methods(std::initializer_list<FnDecl *> methods);
    void clear_methods();
    void remove_method(FnDecl *method);

    StructDefStaticMethods &get_static_methods();
    void add_static_method(FnDecl *method);
    void add_static_methods(std::initializer_list<FnDecl *> methods);
    void clear_static_methods();
    void remove_static_method(FnDecl *method);

    StructDefFields &get_fields();
    void add_field(CompositeField *field);
    void add_fields(std::initializer_list<CompositeField *> fields);
    void clear_fields();
    void remove_field(CompositeField *field);

    NODE_IMPL_CORE(StructDef)
  };

  typedef std::vector<CompositeField *, Arena<CompositeField *>> GroupDefFields;
  typedef std::vector<FnDecl *, Arena<FnDecl *>> GroupDefMethods;
  typedef std::vector<FnDecl *, Arena<FnDecl *>> GroupDefStaticMethods;

  class GroupDef : public Decl {
  protected:
    GroupDefMethods m_methods;
    GroupDefStaticMethods m_static_methods;
    GroupDefFields m_fields;

  public:
    GroupDef(String name = "", GroupTy *type = nullptr,
             std::initializer_list<CompositeField *> fields = {},
             std::initializer_list<FnDecl *> methods = {},
             std::initializer_list<FnDecl *> static_methods = {})
        : Decl(name, type),
          m_methods(methods),
          m_static_methods(static_methods),
          m_fields(fields) {}
    GroupDef(String name, GroupTy *type, const GroupDefFields &fields,
             const GroupDefMethods &methods, const GroupDefStaticMethods &static_methods)
        : Decl(name, type),
          m_methods(methods),
          m_static_methods(static_methods),
          m_fields(fields) {}

    virtual GroupTy *get_type() const override;

    GroupDefMethods &get_methods();
    void add_method(FnDecl *method);
    void add_methods(std::initializer_list<FnDecl *> methods);
    void clear_methods();
    void remove_method(FnDecl *method);

    GroupDefStaticMethods &get_static_methods();
    void add_static_method(FnDecl *method);
    void add_static_methods(std::initializer_list<FnDecl *> methods);
    void clear_static_methods();
    void remove_static_method(FnDecl *method);

    GroupDefFields &get_fields();
    void add_field(CompositeField *field);
    void add_fields(std::initializer_list<CompositeField *> fields);
    void clear_fields();
    void remove_field(CompositeField *field);

    NODE_IMPL_CORE(GroupDef);
  };

  typedef std::vector<CompositeField *, Arena<CompositeField *>> RegionDefFields;
  typedef std::vector<FnDecl *, Arena<FnDecl *>> RegionDefMethods;
  typedef std::vector<FnDecl *, Arena<FnDecl *>> RegionDefStaticMethods;

  class RegionDef : public Decl {
  protected:
    RegionDefMethods m_methods;
    RegionDefStaticMethods m_static_methods;
    RegionDefFields m_fields;

  public:
    RegionDef(String name = "", RegionTy *type = nullptr,
              std::initializer_list<CompositeField *> fields = {},
              std::initializer_list<FnDecl *> methods = {},
              std::initializer_list<FnDecl *> static_methods = {})
        : Decl(name, type),
          m_methods(methods),
          m_static_methods(static_methods),
          m_fields(fields) {}
    RegionDef(String name, RegionTy *type, const RegionDefFields &fields,
              const RegionDefMethods &methods, const RegionDefStaticMethods &static_methods)
        : Decl(name, type),
          m_methods(methods),
          m_static_methods(static_methods),
          m_fields(fields) {}

    virtual RegionTy *get_type() const override;

    RegionDefMethods &get_methods();
    void add_method(FnDecl *method);
    void add_methods(std::initializer_list<FnDecl *> methods);
    void clear_methods();
    void remove_method(FnDecl *method);

    RegionDefStaticMethods &get_static_methods();
    void add_static_method(FnDecl *method);
    void add_static_methods(std::initializer_list<FnDecl *> methods);
    void clear_static_methods();
    void remove_static_method(FnDecl *method);

    RegionDefFields &get_fields();
    void add_field(CompositeField *field);
    void add_fields(std::initializer_list<CompositeField *> fields);
    void clear_fields();
    void remove_field(CompositeField *field);

    NODE_IMPL_CORE(RegionDef);
  };

  typedef std::vector<CompositeField *, Arena<CompositeField *>> UnionDefFields;
  typedef std::vector<FnDecl *, Arena<FnDecl *>> UnionDefMethods;
  typedef std::vector<FnDecl *, Arena<FnDecl *>> UnionDefStaticMethods;

  class UnionDef : public Decl {
  protected:
    UnionDefMethods m_methods;
    UnionDefStaticMethods m_static_methods;
    UnionDefFields m_fields;

  public:
    UnionDef(String name = "", UnionTy *type = nullptr,
             std::initializer_list<CompositeField *> fields = {},
             std::initializer_list<FnDecl *> methods = {},
             std::initializer_list<FnDecl *> static_methods = {})
        : Decl(name, type),
          m_methods(methods),
          m_static_methods(static_methods),
          m_fields(fields) {}
    UnionDef(String name, UnionTy *type, const UnionDefFields &fields,
             const UnionDefMethods &methods, const UnionDefStaticMethods &static_methods)
        : Decl(name, type),
          m_methods(methods),
          m_static_methods(static_methods),
          m_fields(fields) {}

    virtual UnionTy *get_type() const override;

    UnionDefMethods &get_methods();
    void add_method(FnDecl *method);
    void add_methods(std::initializer_list<FnDecl *> methods);
    void clear_methods();
    void remove_method(FnDecl *method);

    UnionDefStaticMethods &get_static_methods();
    void add_static_method(FnDecl *method);
    void add_static_methods(std::initializer_list<FnDecl *> methods);
    void clear_static_methods();
    void remove_static_method(FnDecl *method);

    UnionDefFields &get_fields();
    void add_field(CompositeField *field);
    void add_fields(std::initializer_list<CompositeField *> fields);
    void clear_fields();
    void remove_field(CompositeField *field);

    NODE_IMPL_CORE(UnionDef);
  };

  typedef std::pair<String, ConstExpr *> EnumItem;
  typedef std::vector<EnumItem, Arena<EnumItem>> EnumDefItems;

  class EnumDef : public Decl {
  protected:
    EnumDefItems m_items;

  public:
    EnumDef(String name = "", EnumTy *type = nullptr, std::initializer_list<EnumItem> items = {})
        : Decl(name, type), m_items(items) {}
    EnumDef(String name, EnumTy *type, const EnumDefItems &items)
        : Decl(name, type), m_items(items) {}

    virtual EnumTy *get_type() const override;

    const EnumDefItems &get_items();
    void add_item(EnumItem item);
    void add_items(std::initializer_list<EnumItem> items);
    void clear_items();
    void remove_item(EnumItem item);

    NODE_IMPL_CORE(EnumDef)
  };

  typedef std::set<String, std::less<String>, Arena<String>> SubsystemDeps;

  class SubsystemDecl : public Decl {
  protected:
    Block *m_body;
    SubsystemDeps m_deps;

  public:
    SubsystemDecl(String name = "", Block *body = nullptr, SubsystemDeps deps = {})
        : Decl(name, nullptr), m_body(body), m_deps(deps) {}

    Block *get_body() const;
    void set_body(Block *body);

    const SubsystemDeps &get_deps() const;
    void add_dep(String dep);
    void add_deps(const SubsystemDeps &deps);
    void clear_deps();
    void remove_dep(String dep);

    NODE_IMPL_CORE(SubsystemDecl)
  };

  class ExportDecl : public Decl {
  protected:
    StmtList *m_body;
    ExportLang m_lang;

  public:
    ExportDecl(std::initializer_list<Stmt *> body = {}, ExportLang lang = ExportLang::Default)
        : Decl("", nullptr), m_body(StmtList::get(body)), m_lang(lang) {}
    ExportDecl(StmtList *content, ExportLang lang = ExportLang::Default)
        : Decl("", nullptr), m_body(content), m_lang(lang) {}

    StmtList *get_body() const;
    void set_body(StmtList *body);

    ExportLang get_lang() const;
    void set_lang(ExportLang lang);

    NODE_IMPL_CORE(ExportDecl)
  };
}  // namespace qparse

namespace std {
  std::ostream &operator<<(std::ostream &os, const qlex_op_t &op);
  std::ostream &operator<<(std::ostream &os, const qlex_op_t &expr);
  std::ostream &operator<<(std::ostream &os, const qlex_op_t &op);
  std::ostream &operator<<(std::ostream &os, const qparse::FuncPurity &purity);
}  // namespace std

#endif

#endif  // __QUIX_PARSER_NODE_H__
