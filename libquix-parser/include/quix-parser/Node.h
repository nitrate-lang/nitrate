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

#include <nitrate-lexer/Token.h>

/**
 * @brief Quixcc abstract syntax tree node.
 */
typedef struct qparse_node_t qparse_node_t;

/**
 * @brief Quixcc abstract syntax tree node type.
 */
typedef enum qparse_ty_t {
  QAST_NODE_STMT,
  QAST_NODE_TYPE,
  QAST_NODE_DECL,
  QAST_NODE_EXPR,
  QAST_NODE_CEXPR,

  QAST_NODE_BINEXPR,
  QAST_NODE_UNEXPR,
  QAST_NODE_TEREXPR,
  QAST_NODE_INT,
  QAST_NODE_FLOAT,
  QAST_NODE_STRING,
  QAST_NODE_CHAR,
  QAST_NODE_BOOL,
  QAST_NODE_NULL,
  QAST_NODE_UNDEF,
  QAST_NODE_CALL,
  QAST_NODE_LIST,
  QAST_NODE_ASSOC,
  QAST_NODE_FIELD,
  QAST_NODE_INDEX,
  QAST_NODE_SLICE,
  QAST_NODE_FSTRING,
  QAST_NODE_IDENT,
  QAST_NODE_SEQ_POINT,
  QAST_NODE_POST_UNEXPR,
  QAST_NODE_STMT_EXPR,
  QAST_NODE_TYPE_EXPR,
  QAST_NODE_TEMPL_CALL,

  QAST_NODE_REF_TY,
  QAST_NODE_U1_TY,
  QAST_NODE_U8_TY,
  QAST_NODE_U16_TY,
  QAST_NODE_U32_TY,
  QAST_NODE_U64_TY,
  QAST_NODE_U128_TY,
  QAST_NODE_I8_TY,
  QAST_NODE_I16_TY,
  QAST_NODE_I32_TY,
  QAST_NODE_I64_TY,
  QAST_NODE_I128_TY,
  QAST_NODE_F16_TY,
  QAST_NODE_F32_TY,
  QAST_NODE_F64_TY,
  QAST_NODE_F128_TY,
  QAST_NODE_VOID_TY,
  QAST_NODE_PTR_TY,
  QAST_NODE_OPAQUE_TY,
  QAST_NODE_STRUCT_TY,
  QAST_NODE_GROUP_TY,
  QAST_NODE_REGION_TY,
  QAST_NODE_UNION_TY,
  QAST_NODE_ARRAY_TY,
  QAST_NODE_TUPLE_TY,
  QAST_NODE_FN_TY,
  QAST_NODE_UNRES_TY,
  QAST_NODE_INFER_TY,
  QAST_NODE_TEMPL_TY,

  QAST_NODE_TYPEDEF,
  QAST_NODE_FNDECL,
  QAST_NODE_STRUCT,
  QAST_NODE_REGION,
  QAST_NODE_GROUP,
  QAST_NODE_UNION,
  QAST_NODE_ENUM,
  QAST_NODE_FN,
  QAST_NODE_SUBSYSTEM,
  QAST_NODE_EXPORT,
  QAST_NODE_COMPOSITE_FIELD,

  QAST_NODE_BLOCK,
  QAST_NODE_CONST,
  QAST_NODE_VAR,
  QAST_NODE_LET,
  QAST_NODE_INLINE_ASM,
  QAST_NODE_RETURN,
  QAST_NODE_RETIF,
  QAST_NODE_RETZ,
  QAST_NODE_RETV,
  QAST_NODE_BREAK,
  QAST_NODE_CONTINUE,
  QAST_NODE_IF,
  QAST_NODE_WHILE,
  QAST_NODE_FOR,
  QAST_NODE_FORM,
  QAST_NODE_FOREACH,
  QAST_NODE_CASE,
  QAST_NODE_SWITCH,
  QAST_NODE_EXPR_STMT,
  QAST_NODE_VOLSTMT,
} qparse_ty_t;

#define QAST_NODE_COUNT 88

typedef struct qparse_node_t qparse_node_t;

qlex_loc_t qparse_startpos(qparse_node_t *node);
qlex_loc_t qparse_endpos(qparse_node_t *node);

///=============================================================================
/// END: ABSTRACT SYNTAX TREE DATA TYPES
///=============================================================================

#if (defined(__cplusplus)) || defined(__QPARSE_IMPL__)

#include <nitrate-core/Error.h>
#include <nitrate-core/Memory.h>
#include <nitrate-lexer/Token.h>

#include <cassert>
#include <iostream>
#include <map>
#include <nitrate-core/Classes.hh>
#include <ostream>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

namespace qparse {
  class ArenaAllocatorImpl {
    qcore_arena m_arena;

  public:
    ArenaAllocatorImpl() = default;

    void *allocate(std::size_t bytes);
    void deallocate(void *ptr) noexcept;

    void swap(qcore_arena_t &arena);

    qcore_arena_t &get() { return *m_arena.get(); }
  };

  extern thread_local ArenaAllocatorImpl qparse_arena;

  template <class T>
  struct Arena {
    typedef T value_type;

    Arena() = default;

    template <class U>
    constexpr Arena(const Arena<U> &) noexcept {}

    [[nodiscard]] T *allocate(std::size_t n) {
      return static_cast<T *>(qparse_arena.allocate(sizeof(T) * n));
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

#define PNODE_IMPL_CORE(__typename)                                              \
protected:                                                                       \
  virtual bool verify_impl(std::ostream &os) override;                           \
                                                                                 \
protected:                                                                       \
  virtual void canonicalize_impl() override;                                     \
                                                                                 \
protected:                                                                       \
  virtual void print_impl(std::ostream &os, bool debug) override;                \
                                                                                 \
public:                                                                          \
  virtual __typename *clone_impl() override;                                     \
                                                                                 \
public:                                                                          \
public:                                                                          \
  template <typename T = __typename, typename... Args>                           \
  static __typename *get(Args &&...args) {                                       \
    void *ptr = Arena<__typename>().allocate(1);                                 \
    return new (ptr) __typename(std::forward<Args>(args)...);                    \
  }                                                                              \
                                                                                 \
public:                                                                          \
  virtual __typename *clone(ArenaAllocatorImpl &arena = qparse_arena) override { \
    ArenaAllocatorImpl old = qparse_arena;                                       \
    qparse_arena = arena;                                                        \
    __typename *node = clone_impl();                                             \
    qparse_arena = old;                                                          \
    return node;                                                                 \
  }

struct qparse_node_t {
public:
  qparse_node_t() = default;
};

namespace qparse {
  enum class Visibility {
    PUBLIC,
    PRIVATE,
    PROTECTED,
  };

  class String : public std::basic_string<char, std::char_traits<char>, Arena<char>> {
  public:
    String() = default;
    String(const char *str) : std::basic_string<char, std::char_traits<char>, Arena<char>>(str) {}
    String(const std::string &str)
        : std::basic_string<char, std::char_traits<char>, Arena<char>>(str.c_str(), str.size()) {}

    std::string_view view() { return std::string_view(data(), size()); }
  };

  class Node : public qparse_node_t {
  protected:
    virtual bool verify_impl(std::ostream &os) = 0;
    virtual void canonicalize_impl() = 0;
    virtual void print_impl(std::ostream &os, bool debug) = 0;
    virtual Node *clone_impl() = 0;

    qlex_loc_t m_pos_start{0}, m_pos_end{0};

  public:
    Node() = default;

    uint32_t this_sizeof();
    qparse_ty_t this_typeid();
    const char *this_nameof();

    bool is_type();
    bool is_stmt();
    bool is_decl();
    bool is_expr();
    bool is_const_expr();

    std::string to_string(bool minify = false, bool binary_repr = false);

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

    bool is(const qparse_ty_t type);
    bool verify(std::ostream &os = std::cerr);
    void canonicalize();
    virtual Node *clone(ArenaAllocatorImpl &arena = qparse_arena) = 0;

    static const char *type_name(qparse_ty_t type);
    void dump(bool isForDebug = false) { print_impl(std::cerr, isForDebug); }
    void print(std::ostream &os, bool isForDebug = false) { print_impl(os, isForDebug); }

    void set_start_pos(qlex_loc_t pos) { m_pos_start = pos; }
    void set_end_pos(qlex_loc_t pos) { m_pos_end = pos; }
    qlex_loc_t get_start_pos() { return m_pos_start; }
    qlex_loc_t get_end_pos() { return m_pos_end; }
  };

  constexpr size_t PNODE_BASE_SIZE = sizeof(Node);

  class Stmt : public Node {
  public:
    Stmt() = default;

    virtual Stmt *clone(ArenaAllocatorImpl &arena = qparse_arena) = 0;
  };

  class ConstExpr;

  class Type : public Node {
  protected:
    ConstExpr *m_width, *m_range_start, *m_range_end;
    bool m_volatile;

  public:
    Type(bool is_volatile = false)
        : m_width(nullptr), m_range_start(nullptr), m_range_end(nullptr), m_volatile(is_volatile) {}

    bool is_primitive();
    bool is_array();
    bool is_tuple();
    bool is_pointer();
    bool is_function();
    bool is_composite();
    bool is_union();
    bool is_numeric();
    bool is_integral();
    bool is_floating_point();
    bool is_signed();
    bool is_unsigned();
    bool is_void();
    bool is_bool();
    bool is_ref();
    bool is_volatile();
    bool is_ptr_to(Type *type);

    virtual Type *clone(ArenaAllocatorImpl &arena = qparse_arena) = 0;

    ConstExpr *get_width() { return m_width; }
    void set_width(ConstExpr *width) { m_width = width; }

    std::pair<ConstExpr *, ConstExpr *> get_range() { return {m_range_start, m_range_end}; }
    void set_range(ConstExpr *start, ConstExpr *end) {
      m_range_start = start;
      m_range_end = end;
    }
  };

  typedef std::set<ConstExpr *, std::less<ConstExpr *>, Arena<ConstExpr *>> DeclTags;

  class Decl : public Stmt {
  protected:
    DeclTags m_tags;
    String m_name;
    Type *m_type;
    Visibility m_visibility;

  public:
    Decl(String name = "", Type *type = nullptr, std::initializer_list<ConstExpr *> tags = {},
         Visibility visibility = Visibility::PRIVATE)
        : m_tags(tags), m_name(name), m_type(type), m_visibility(visibility) {}

    String get_name() { return m_name; }
    void set_name(String name) { m_name = name; }

    virtual Type *get_type() { return m_type; }
    void set_type(Type *type) { m_type = type; }

    DeclTags &get_tags() { return m_tags; }
    void add_tag(ConstExpr *tag) { m_tags.insert(tag); }
    void add_tags(const std::set<ConstExpr *> &tags) {
      for (const auto &tag : tags) {
        m_tags.insert(tag);
      }
    }
    void clear_tags() { m_tags.clear(); }
    void remove_tag(ConstExpr *tag) { m_tags.erase(tag); }

    Visibility get_visibility() { return m_visibility; }
    void set_visibility(Visibility visibility) { m_visibility = visibility; }

    virtual Decl *clone(ArenaAllocatorImpl &arena = qparse_arena) = 0;
  };

  class Expr : public Node {
  protected:
    Type *m_type;

  public:
    Expr() : m_type(nullptr) {}

    bool is_binexpr();
    bool is_unaryexpr();
    bool is_ternaryexpr();

    virtual Expr *clone(ArenaAllocatorImpl &arena = qparse_arena) = 0;
  };

  class ExprStmt : public Stmt {
  protected:
    Expr *m_expr;

  public:
    ExprStmt(Expr *expr = nullptr) : m_expr(expr) {}

    Expr *get_expr() { return m_expr; }
    void set_expr(Expr *expr) { m_expr = expr; }

    PNODE_IMPL_CORE(ExprStmt)
  };

  class StmtExpr : public Expr {
  protected:
    Stmt *m_stmt;

  public:
    StmtExpr(Stmt *stmt = nullptr) : m_stmt(stmt) {}

    Stmt *get_stmt() { return m_stmt; }
    void set_stmt(Stmt *stmt) { m_stmt = stmt; }

    PNODE_IMPL_CORE(StmtExpr)
  };

  class TypeExpr : public Expr {
  protected:
    Type *m_type;

  public:
    TypeExpr(Type *type = nullptr) : m_type(type) {}

    Type *get_type() { return m_type; }
    void set_type(Type *type) { m_type = type; }

    PNODE_IMPL_CORE(TypeExpr)
  };

  class ConstExpr : public Expr {
  protected:
    Expr *m_value;

  public:
    ConstExpr(Expr *value = nullptr) : m_value(value) {}

    Expr *get_value() { return m_value; }
    void set_value(Expr *value) { m_value = value; }

    PNODE_IMPL_CORE(ConstExpr)
  };

  class LitExpr : public ConstExpr {
  public:
    LitExpr() = default;

    virtual LitExpr *clone(ArenaAllocatorImpl &arena = qparse_arena) = 0;
  };

  class FlowStmt : public Stmt {
  public:
    FlowStmt() = default;

    virtual FlowStmt *clone(ArenaAllocatorImpl &arena = qparse_arena) = 0;
  };

  class DeclStmt : public Stmt {
  public:
    DeclStmt() = default;

    virtual DeclStmt *clone(ArenaAllocatorImpl &arena = qparse_arena) = 0;
  };

  class TypeBuiltin : public Type {
  public:
    TypeBuiltin() = default;

    virtual TypeBuiltin *clone(ArenaAllocatorImpl &arena = qparse_arena) = 0;
  };

  class TypeComplex : public Type {
  public:
    TypeComplex() = default;

    virtual TypeComplex *clone(ArenaAllocatorImpl &arena = qparse_arena) = 0;
  };

  class TypeComposite : public Type {
  public:
    TypeComposite() = default;

    virtual TypeComposite *clone(ArenaAllocatorImpl &arena = qparse_arena) = 0;
  };

  class UnresolvedType : public Type {
    String m_name;

  public:
    UnresolvedType(String name = "") : m_name(name) {}

    String get_name() { return m_name; }
    void set_name(String name) { m_name = name; }

    PNODE_IMPL_CORE(UnresolvedType)
  };

  class InferType : public Type {
  public:
    InferType() = default;

    PNODE_IMPL_CORE(InferType)
  };

  typedef std::vector<Expr *, Arena<Expr *>> TemplTypeArgs;
  class TemplType : public Type {
    Type *m_template;
    TemplTypeArgs m_args;

  public:
    TemplType(Type *templ = nullptr, std::initializer_list<Expr *> args = {})
        : m_template(templ), m_args(args) {}
    TemplType(Type *templ, const TemplTypeArgs &args) : m_template(templ), m_args(args) {}

    Type *get_template() { return m_template; }
    void set_template(Type *templ) { m_template = templ; }

    TemplTypeArgs &get_args() { return m_args; }

    PNODE_IMPL_CORE(TemplType)
  };

  class U1 : public TypeBuiltin {
  public:
    U1() = default;

    PNODE_IMPL_CORE(U1)
  };

  class U8 : public TypeBuiltin {
  public:
    U8() = default;

    PNODE_IMPL_CORE(U8)
  };

  class U16 : public TypeBuiltin {
  public:
    U16() = default;

    PNODE_IMPL_CORE(U16)
  };

  class U32 : public TypeBuiltin {
  public:
    U32() = default;

    PNODE_IMPL_CORE(U32)
  };

  class U64 : public TypeBuiltin {
  public:
    U64() = default;

    PNODE_IMPL_CORE(U64)
  };

  class U128 : public TypeBuiltin {
  public:
    U128() = default;

    PNODE_IMPL_CORE(U128)
  };

  class I8 : public TypeBuiltin {
  public:
    I8() = default;

    PNODE_IMPL_CORE(I8)
  };

  class I16 : public TypeBuiltin {
  public:
    I16() = default;

    PNODE_IMPL_CORE(I16)
  };

  class I32 : public TypeBuiltin {
  public:
    I32() = default;

    PNODE_IMPL_CORE(I32)
  };

  class I64 : public TypeBuiltin {
  public:
    I64() = default;

    PNODE_IMPL_CORE(I64)
  };

  class I128 : public TypeBuiltin {
  public:
    I128() = default;

    PNODE_IMPL_CORE(I128)
  };

  class F16 : public TypeBuiltin {
  public:
    F16() = default;

    PNODE_IMPL_CORE(F16)
  };

  class F32 : public TypeBuiltin {
  public:
    F32() = default;

    PNODE_IMPL_CORE(F32)
  };

  class F64 : public TypeBuiltin {
  public:
    F64() = default;

    PNODE_IMPL_CORE(F64)
  };

  class F128 : public TypeBuiltin {
  public:
    F128() = default;

    PNODE_IMPL_CORE(F128)
  };

  class VoidTy : public TypeBuiltin {
  public:
    VoidTy() = default;

    PNODE_IMPL_CORE(VoidTy)
  };

  class PtrTy : public TypeComplex {
    Type *m_item;
    bool m_is_volatile;

  public:
    PtrTy(Type *item = nullptr, bool is_volatile = false)
        : m_item(item), m_is_volatile(is_volatile) {}

    Type *get_item() { return m_item; }
    void set_item(Type *item) { m_item = item; }

    bool is_volatile() { return m_is_volatile; }
    void set_volatile(bool is_volatile) { m_is_volatile = is_volatile; }

    PNODE_IMPL_CORE(PtrTy)
  };

  class OpaqueTy : public TypeComplex {
    String m_name;

  public:
    OpaqueTy(String name = "") : m_name(name) {}

    String get_name() { return m_name; }
    void set_name(String name) { m_name = name; }

    PNODE_IMPL_CORE(OpaqueTy)
  };

  typedef std::vector<Type *, Arena<Type *>> TupleTyItems;
  class TupleTy : public TypeComposite {
    TupleTyItems m_items;

  public:
    TupleTy(std::initializer_list<Type *> items = {}) : m_items(items) {}
    TupleTy(const TupleTyItems &items) : m_items(items) {}

    TupleTyItems &get_items() { return m_items; }
    void add_item(Type *item);
    void add_items(std::initializer_list<Type *> items);
    void clear_items();
    void remove_item(Type *item);

    PNODE_IMPL_CORE(TupleTy)
  };

  class ArrayTy : public TypeComposite {
    Type *m_item;
    ConstExpr *m_size;

  public:
    ArrayTy(Type *item = nullptr, ConstExpr *size = nullptr) : m_item(item), m_size(size) {}

    Type *get_item() { return m_item; }
    void set_item(Type *item) { m_item = item; }

    ConstExpr *get_size() { return m_size; }
    void set_size(ConstExpr *size) { m_size = size; }

    PNODE_IMPL_CORE(ArrayTy)
  };

  class RefTy : public TypeComplex {
    Type *m_item;

  public:
    RefTy(Type *item = nullptr) : m_item(item) {}

    Type *get_item() { return m_item; }
    void set_item(Type *item) { m_item = item; }

    PNODE_IMPL_CORE(RefTy)
  };

  typedef std::pair<String, Type *> StructItem;
  typedef std::vector<StructItem, Arena<StructItem>> StructItems;

  class StructTy : public TypeComposite {
    StructItems m_items;

  public:
    StructTy(std::initializer_list<StructItem> items = {}) : m_items(items) {}
    StructTy(const StructItems &items) : m_items(items) {}

    StructItems &get_items() { return m_items; }
    void add_item(String name, Type *type);
    void add_items(std::initializer_list<StructItem> items);
    void clear_items();
    void remove_item(String name);

    PNODE_IMPL_CORE(StructTy)
  };

  typedef std::vector<Type *, Arena<Type *>> GroupTyItems;

  class GroupTy : public TypeComposite {
    GroupTyItems m_items;

  public:
    GroupTy(std::initializer_list<Type *> items = {}) : m_items(items) {}
    GroupTy(const GroupTyItems &items) : m_items(items) {}

    GroupTyItems &get_items() { return m_items; }
    void add_item(Type *item);
    void add_items(std::initializer_list<Type *> items);
    void clear_items();
    void remove_item(Type *item);

    PNODE_IMPL_CORE(GroupTy)
  };

  typedef std::vector<Type *, Arena<Type *>> RegionTyItems;

  class RegionTy : public TypeComposite {
    RegionTyItems m_items;

  public:
    RegionTy(std::initializer_list<Type *> items = {}) : m_items(items) {}
    RegionTy(const RegionTyItems &items) : m_items(items) {}

    RegionTyItems &get_items() { return m_items; }
    void add_item(Type *item);
    void add_items(std::initializer_list<Type *> items);
    void clear_items();
    void remove_item(Type *item);

    PNODE_IMPL_CORE(RegionTy)
  };

  typedef std::vector<Type *, Arena<Type *>> UnionTyItems;

  class UnionTy : public TypeComposite {
    UnionTyItems m_items;

  public:
    UnionTy(std::initializer_list<Type *> items = {}) : m_items(items) {}
    UnionTy(const UnionTyItems &items) : m_items(items) {}

    UnionTyItems &get_items() { return m_items; }
    void add_item(Type *item);
    void add_items(std::initializer_list<Type *> items);
    void clear_items();
    void remove_item(Type *item);

    PNODE_IMPL_CORE(UnionTy)
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

    bool is_noreturn();
    void set_noreturn(bool noreturn);

    Type *get_return_ty() { return m_return; }
    void set_return_ty(Type *return_ty) { m_return = return_ty; }

    FuncParams &get_params() { return m_params; }
    void add_param(String name, Type *type, Expr *default_val = nullptr);
    void add_params(std::initializer_list<FuncParam> params);
    void clear_params();
    void remove_param(String name);

    FuncPurity get_purity() { return m_purity; }
    void set_purity(FuncPurity purity) { m_purity = purity; }

    bool is_variadic() { return m_variadic; }
    void set_variadic(bool variadic) { m_variadic = variadic; }

    bool is_foreign() { return m_is_foreign; }
    void set_foreign(bool is_foreign) { m_is_foreign = is_foreign; }

    bool is_crashpoint() { return m_crashpoint; }
    void set_crashpoint(bool crashpoint) { m_crashpoint = crashpoint; }

    bool is_noexcept() { return m_noexcept; }
    void set_noexcept(bool noexcept_) { m_noexcept = noexcept_; }

    PNODE_IMPL_CORE(FuncTy)
  };

  ///=============================================================================

  class UnaryExpr : public Expr {
  protected:
    Expr *m_rhs;
    qlex_op_t m_op;

  public:
    UnaryExpr(qlex_op_t op = qOpTernary, Expr *rhs = nullptr) : m_rhs(rhs), m_op(op) {}

    Expr *get_rhs() { return m_rhs; }
    void set_rhs(Expr *rhs) { m_rhs = rhs; }

    qlex_op_t get_op() { return m_op; }
    void set_op(qlex_op_t op) { m_op = op; }

    PNODE_IMPL_CORE(UnaryExpr)
  };

  class BinExpr : public Expr {
  protected:
    Expr *m_lhs;
    Expr *m_rhs;
    qlex_op_t m_op;

  public:
    BinExpr(Expr *lhs = nullptr, qlex_op_t op = qOpTernary, Expr *rhs = nullptr)
        : m_lhs(lhs), m_rhs(rhs), m_op(op) {}

    Expr *get_lhs() { return m_lhs; }
    void set_lhs(Expr *lhs) { m_lhs = lhs; }

    Expr *get_rhs() { return m_rhs; }
    void set_rhs(Expr *rhs) { m_rhs = rhs; }

    qlex_op_t get_op() { return m_op; }
    void set_op(qlex_op_t op) { m_op = op; }

    PNODE_IMPL_CORE(BinExpr)
  };

  class PostUnaryExpr : public Expr {
  protected:
    Expr *m_lhs;
    qlex_op_t m_op;

  public:
    PostUnaryExpr(Expr *lhs = nullptr, qlex_op_t op = qOpTernary) : m_lhs(lhs), m_op(op) {}

    Expr *get_lhs() { return m_lhs; }
    void set_lhs(Expr *lhs) { m_lhs = lhs; }

    qlex_op_t get_op() { return m_op; }
    void set_op(qlex_op_t op) { m_op = op; }

    PNODE_IMPL_CORE(PostUnaryExpr)
  };

  class TernaryExpr : public Expr {
  protected:
    Expr *m_cond;
    Expr *m_lhs;
    Expr *m_rhs;

  public:
    TernaryExpr(Expr *cond = nullptr, Expr *lhs = nullptr, Expr *rhs = nullptr)
        : m_cond(cond), m_lhs(lhs), m_rhs(rhs) {}

    Expr *get_cond() { return m_cond; }
    void set_cond(Expr *cond) { m_cond = cond; }

    Expr *get_lhs() { return m_lhs; }
    void set_lhs(Expr *lhs) { m_lhs = lhs; }

    Expr *get_rhs() { return m_rhs; }
    void set_rhs(Expr *rhs) { m_rhs = rhs; }

    PNODE_IMPL_CORE(TernaryExpr)
  };

  ///=============================================================================

  class ConstInt : public LitExpr {
    String m_value;

  public:
    ConstInt(String value = "") : m_value(value) {}
    ConstInt(uint64_t value) : m_value(std::to_string(value)) {}

    String get_value() { return m_value; }

    PNODE_IMPL_CORE(ConstInt)
  };

  class ConstFloat : public LitExpr {
    String m_value;

  public:
    ConstFloat(String value = "") : m_value(value) {}
    ConstFloat(double value) : m_value(std::to_string(value)) {}

    String get_value() { return m_value; }

    PNODE_IMPL_CORE(ConstFloat)
  };

  class ConstBool : public LitExpr {
    bool m_value;

  public:
    ConstBool(bool value = false) : m_value(value) {}

    bool get_value() { return m_value; }

    PNODE_IMPL_CORE(ConstBool)
  };

  class ConstString : public LitExpr {
    String m_value;

  public:
    ConstString(String value = "") : m_value(value) {}

    String get_value() { return m_value; }

    PNODE_IMPL_CORE(ConstString)
  };

  class ConstChar : public LitExpr {
    char32_t m_value;

  public:
    ConstChar(char32_t value = 0) : m_value(value) {}

    char32_t get_value() { return m_value; }

    PNODE_IMPL_CORE(ConstChar)
  };

  class ConstNull : public LitExpr {
  public:
    ConstNull() = default;

    PNODE_IMPL_CORE(ConstNull)
  };

  class ConstUndef : public LitExpr {
  public:
    ConstUndef() = default;

    PNODE_IMPL_CORE(ConstUndef)
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

    Expr *get_func() { return m_func; }
    void set_func(Expr *func) { m_func = func; }

    CallArgs &get_args() { return m_args; }
    void add_arg(CallArg arg);
    void add_args(std::initializer_list<CallArg> args);
    void clear_args();
    void remove_arg(String name);

    PNODE_IMPL_CORE(Call)
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

    Expr *get_func() { return m_func; }
    void set_func(Expr *func) { m_func = func; }

    TemplateArgs &get_template_args() { return m_template_args; }
    void add_template_arg(String name, ConstExpr *arg);
    void add_template_args(std::map<String, ConstExpr *> args);
    void clear_template_args();
    void remove_template_arg(String name);

    PNODE_IMPL_CORE(TemplCall)
  };

  typedef std::vector<Expr *, Arena<Expr *>> ListData;

  class List : public Expr {
  protected:
    ListData m_items;

  public:
    List(std::initializer_list<Expr *> items = {}) : m_items(items) {}
    List(const ListData &items) : m_items(items) {}

    ListData &get_items() { return m_items; }
    void add_item(Expr *item);
    void add_items(std::initializer_list<Expr *> items);
    void clear_items();
    void remove_item(Expr *item);

    PNODE_IMPL_CORE(List)
  };

  class Assoc : public Expr {
  protected:
    Expr *m_key;
    Expr *m_value;

  public:
    Assoc(Expr *key = nullptr, Expr *value = nullptr) : m_key(key), m_value(value) {}

    Expr *get_key() { return m_key; }
    void set_key(Expr *key) { m_key = key; }

    Expr *get_value() { return m_value; }
    void set_value(Expr *value) { m_value = value; }

    PNODE_IMPL_CORE(Assoc)
  };

  class Field : public Expr {
  protected:
    Expr *m_base;
    String m_field;

  public:
    Field(Expr *base = nullptr, String field = "") : m_base(base), m_field(field) {}

    Expr *get_base() { return m_base; }
    void set_base(Expr *base) { m_base = base; }

    String get_field() { return m_field; }
    void set_field(String field) { m_field = field; }

    PNODE_IMPL_CORE(Field)
  };

  class Index : public Expr {
  protected:
    Expr *m_base;
    Expr *m_index;

  public:
    Index(Expr *base = nullptr, Expr *index = nullptr) : m_base(base), m_index(index) {}

    Expr *get_base() { return m_base; }
    void set_base(Expr *base) { m_base = base; }

    Expr *get_index() { return m_index; }
    void set_index(Expr *index) { m_index = index; }

    PNODE_IMPL_CORE(Index)
  };

  class Slice : public Expr {
  protected:
    Expr *m_base;
    Expr *m_start;
    Expr *m_end;

  public:
    Slice(Expr *base = nullptr, Expr *start = nullptr, Expr *end = nullptr)
        : m_base(base), m_start(start), m_end(end) {}

    Expr *get_base() { return m_base; }
    void set_base(Expr *base) { m_base = base; }

    Expr *get_start() { return m_start; }
    void set_start(Expr *start) { m_start = start; }

    Expr *get_end() { return m_end; }
    void set_end(Expr *end) { m_end = end; }

    PNODE_IMPL_CORE(Slice)
  };

  typedef std::vector<std::variant<String, Expr *>, Arena<std::variant<String, Expr *>>>
      FStringItems;

  class FString : public Expr {
  protected:
    FStringItems m_items;

  public:
    FString(FStringItems items = {}) : m_items(items) {}

    FStringItems &get_items() { return m_items; }
    void add_item(String item);
    void add_item(Expr *item);

    PNODE_IMPL_CORE(FString)
  };

  class Ident : public Expr {
    String m_name;

  public:
    Ident(String name = "") : m_name(name) {}

    String get_name() { return m_name; }
    void set_name(String name) { m_name = name; }

    PNODE_IMPL_CORE(Ident)
  };

  typedef std::vector<Expr *, Arena<Expr *>> SeqPointItems;
  class SeqPoint : public Expr {
  protected:
    SeqPointItems m_items;

  public:
    SeqPoint(std::initializer_list<Expr *> items = {}) : m_items(items) {}
    SeqPoint(const SeqPointItems &items) : m_items(items) {}

    SeqPointItems &get_items() { return m_items; }
    void add_item(Expr *item);
    void add_items(std::initializer_list<Expr *> items);
    void clear_items();
    void remove_item(Expr *item);

    PNODE_IMPL_CORE(SeqPoint)
  };

  ///=============================================================================

  typedef std::vector<Stmt *, Arena<Stmt *>> BlockItems;

  enum class SafetyMode {
    Unknown = 0,
    Safe = 1,
    Unsafe = 2,
  };

  class Block : public Stmt {
  protected:
    BlockItems m_items;
    SafetyMode m_safety;

  public:
    Block(std::initializer_list<Stmt *> items = {})
        : m_items(items), m_safety(SafetyMode::Unknown) {}
    Block(const BlockItems &items, SafetyMode safety) : m_items(items), m_safety(safety) {}

    BlockItems &get_items() { return m_items; }
    void add_item(Stmt *item);
    void add_items(std::initializer_list<Stmt *> items);
    void clear_items();
    void remove_item(Stmt *item);

    SafetyMode get_safety() { return m_safety; }
    void set_safety(SafetyMode safety) { m_safety = safety; }

    PNODE_IMPL_CORE(Block)
  };

  class VolStmt : public Stmt {
  protected:
    Stmt *m_stmt;

  public:
    VolStmt(Stmt *stmt = nullptr) : m_stmt(stmt) {}

    Stmt *get_stmt() { return m_stmt; }
    void set_stmt(Stmt *stmt) { m_stmt = stmt; }

    PNODE_IMPL_CORE(VolStmt)
  };

  class ConstDecl : public Decl {
  protected:
    Expr *m_value;

  public:
    ConstDecl(String name = "", Type *type = nullptr, Expr *value = nullptr)
        : Decl(name, type), m_value(value) {}

    Expr *get_value() { return m_value; }
    void set_value(Expr *value) { m_value = value; }

    PNODE_IMPL_CORE(ConstDecl)
  };

  class VarDecl : public Decl {
  protected:
    Expr *m_value;

  public:
    VarDecl(String name = "", Type *type = nullptr, Expr *value = nullptr)
        : Decl(name, type), m_value(value) {}

    Expr *get_value() { return m_value; }
    void set_value(Expr *value) { m_value = value; }

    PNODE_IMPL_CORE(VarDecl)
  };

  class LetDecl : public Decl {
  protected:
    Expr *m_value;

  public:
    LetDecl(String name = "", Type *type = nullptr, Expr *value = nullptr)
        : Decl(name, type), m_value(value) {}

    Expr *get_value() { return m_value; }
    void set_value(Expr *value) { m_value = value; }

    PNODE_IMPL_CORE(LetDecl)
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

    String get_code() { return m_code; }
    void set_code(String code) { m_code = code; }

    InlineAsmArgs &get_args() { return m_args; }
    void add_arg(Expr *arg);
    void add_args(std::initializer_list<Expr *> args);
    void clear_args();
    void remove_arg(Expr *arg);

    PNODE_IMPL_CORE(InlineAsm)
  };

  class IfStmt : public FlowStmt {
  protected:
    Expr *m_cond;
    Block *m_then;
    Block *m_else;

  public:
    IfStmt(Expr *cond = nullptr, Block *then = nullptr, Block *else_ = nullptr)
        : m_cond(cond), m_then(then), m_else(else_) {}

    Expr *get_cond() { return m_cond; }
    void set_cond(Expr *cond) { m_cond = cond; }

    Block *get_then() { return m_then; }
    void set_then(Block *then) { m_then = then; }

    Block *get_else() { return m_else; }
    void set_else(Block *else_) { m_else = else_; }

    PNODE_IMPL_CORE(IfStmt)
  };

  class WhileStmt : public FlowStmt {
  protected:
    Expr *m_cond;
    Block *m_body;

  public:
    WhileStmt(Expr *cond = nullptr, Block *body = nullptr) : m_cond(cond), m_body(body) {}

    Expr *get_cond() { return m_cond; }
    void set_cond(Expr *cond) { m_cond = cond; }

    Block *get_body() { return m_body; }
    void set_body(Block *body) { m_body = body; }

    PNODE_IMPL_CORE(WhileStmt)
  };

  class ForStmt : public FlowStmt {
  protected:
    Expr *m_init;
    Expr *m_cond;
    Expr *m_step;
    Block *m_body;

  public:
    ForStmt(Expr *init = nullptr, Expr *cond = nullptr, Expr *step = nullptr, Block *body = nullptr)
        : m_init(init), m_cond(cond), m_step(step), m_body(body) {}

    Expr *get_init() { return m_init; }
    void set_init(Expr *init) { m_init = init; }

    Expr *get_cond() { return m_cond; }
    void set_cond(Expr *cond) { m_cond = cond; }

    Expr *get_step() { return m_step; }
    void set_step(Expr *step) { m_step = step; }

    Block *get_body() { return m_body; }
    void set_body(Block *body) { m_body = body; }

    PNODE_IMPL_CORE(ForStmt)
  };

  class FormStmt : public FlowStmt {
  protected:
    String m_idx_ident;
    String m_val_ident;
    Expr *m_expr;
    Expr *m_maxjobs;
    Block *m_body;

  public:
    FormStmt(String idx_ident = "", String val_ident = "", Expr *expr = nullptr,
             Expr *maxjobs = nullptr, Block *body = nullptr)
        : m_idx_ident(idx_ident),
          m_val_ident(val_ident),
          m_expr(expr),
          m_maxjobs(maxjobs),
          m_body(body) {}

    String get_idx_ident() { return m_idx_ident; }
    void set_idx_ident(String idx_ident) { m_idx_ident = idx_ident; }

    String get_val_ident() { return m_val_ident; }
    void set_val_ident(String val_ident) { m_val_ident = val_ident; }

    Expr *get_expr() { return m_expr; }
    void set_expr(Expr *expr) { m_expr = expr; }

    Expr *get_maxjobs() { return m_maxjobs; }
    void set_maxjobs(Expr *maxjobs) { m_maxjobs = maxjobs; }

    Block *get_body() { return m_body; }
    void set_body(Block *body) { m_body = body; }

    PNODE_IMPL_CORE(FormStmt)
  };

  class ForeachStmt : public FlowStmt {
  protected:
    String m_idx_ident;
    String m_val_ident;
    Expr *m_expr;
    Block *m_body;

  public:
    ForeachStmt(String idx_ident = "", String val_ident = "", Expr *expr = nullptr,
                Block *body = nullptr)
        : m_idx_ident(idx_ident), m_val_ident(val_ident), m_expr(expr), m_body(body) {}

    String get_idx_ident() { return m_idx_ident; }
    void set_idx_ident(String idx_ident) { m_idx_ident = idx_ident; }

    String get_val_ident() { return m_val_ident; }
    void set_val_ident(String val_ident) { m_val_ident = val_ident; }

    Expr *get_expr() { return m_expr; }
    void set_expr(Expr *expr) { m_expr = expr; }

    Block *get_body() { return m_body; }
    void set_body(Block *body) { m_body = body; }

    PNODE_IMPL_CORE(ForeachStmt)
  };

  class BreakStmt : public FlowStmt {
  public:
    BreakStmt() = default;

    PNODE_IMPL_CORE(BreakStmt)
  };

  class ContinueStmt : public FlowStmt {
  public:
    ContinueStmt() = default;

    PNODE_IMPL_CORE(ContinueStmt)
  };

  class ReturnStmt : public FlowStmt {
  protected:
    Expr *m_value;

  public:
    ReturnStmt(Expr *value = nullptr) : m_value(value) {}

    Expr *get_value() { return m_value; }
    void set_value(Expr *value) { m_value = value; }

    PNODE_IMPL_CORE(ReturnStmt)
  };

  class ReturnIfStmt : public FlowStmt {
  protected:
    Expr *m_cond;
    Expr *m_value;

  public:
    ReturnIfStmt(Expr *cond = nullptr, Expr *value = nullptr) : m_cond(cond), m_value(value) {}

    Expr *get_cond() { return m_cond; }
    void set_cond(Expr *cond) { m_cond = cond; }

    Expr *get_value() { return m_value; }
    void set_value(Expr *value) { m_value = value; }

    PNODE_IMPL_CORE(ReturnIfStmt)
  };

  class RetZStmt : public FlowStmt {
  protected:
    Expr *m_cond;
    Expr *m_value;

  public:
    RetZStmt(Expr *cond = nullptr, Expr *value = nullptr) : m_cond(cond), m_value(value) {}

    Expr *get_cond() { return m_cond; }
    void set_cond(Expr *cond) { m_cond = cond; }

    Expr *get_value() { return m_value; }
    void set_value(Expr *value) { m_value = value; }

    PNODE_IMPL_CORE(RetZStmt)
  };

  class RetVStmt : public FlowStmt {
  protected:
    Expr *m_cond;

  public:
    RetVStmt(Expr *cond = nullptr) : m_cond(cond) {}

    Expr *get_cond() { return m_cond; }
    void set_cond(Expr *cond) { m_cond = cond; }

    PNODE_IMPL_CORE(RetVStmt)
  };

  class CaseStmt : public FlowStmt {
  protected:
    Expr *m_cond;
    Block *m_body;

  public:
    CaseStmt(Expr *cond = nullptr, Block *body = nullptr) : m_cond(cond), m_body(body) {}

    Expr *get_cond() { return m_cond; }
    void set_cond(Expr *cond) { m_cond = cond; }

    Block *get_body() { return m_body; }
    void set_body(Block *body) { m_body = body; }

    PNODE_IMPL_CORE(CaseStmt)
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

    Expr *get_cond() { return m_cond; }
    void set_cond(Expr *cond) { m_cond = cond; }

    SwitchCases &get_cases() { return m_cases; }
    void add_case(CaseStmt *case_);
    void add_cases(std::initializer_list<CaseStmt *> cases);
    void clear_cases();
    void remove_case(CaseStmt *case_);

    Stmt *get_default() { return m_default; }
    void set_default(Stmt *default_) { m_default = default_; }

    PNODE_IMPL_CORE(SwitchStmt)
  };

  ///=============================================================================

  class TypedefDecl : public Decl {
  protected:
  public:
    TypedefDecl(String name = "", Type *type = nullptr) : Decl(name, type) {}

    PNODE_IMPL_CORE(TypedefDecl)
  };

  class FnDecl : public Decl {
  protected:
  public:
    FnDecl(String name = "", FuncTy *type = nullptr) : Decl(name, type) {}

    virtual FuncTy *get_type() override { return static_cast<FuncTy *>(m_type); }

    PNODE_IMPL_CORE(FnDecl)
  };

  typedef std::vector<std::pair<String, bool>, Arena<std::pair<String, bool>>> FnCaptures;

  class FnDef : public FnDecl {
  protected:
    FnCaptures m_captures;
    Block *m_body;
    Expr *m_precond;
    Expr *m_postcond;

  public:
    FnDef(FnDecl *decl = nullptr, Block *body = nullptr, Expr *precond = nullptr,
          Expr *postcond = nullptr, FnCaptures captures = {})
        : FnDecl(decl->get_name(), decl->get_type()),
          m_captures(captures),
          m_body(body),
          m_precond(precond),
          m_postcond(postcond) {}

    Block *get_body() { return m_body; }
    void set_body(Block *body) { m_body = body; }

    Expr *get_precond() { return m_precond; }
    void set_precond(Expr *precond) { m_precond = precond; }

    Expr *get_postcond() { return m_postcond; }
    void set_postcond(Expr *postcond) { m_postcond = postcond; }

    FnCaptures &get_captures() { return m_captures; }
    void add_capture(String name, bool by_ref) { m_captures.push_back({name, by_ref}); }

    PNODE_IMPL_CORE(FnDef)
  };

  class CompositeField : public Decl {
  protected:
    Expr *m_value;

  public:
    CompositeField(String name = "", Type *type = nullptr, Expr *value = nullptr)
        : Decl(name, type), m_value(value) {}

    Expr *get_value() { return m_value; }
    void set_value(Expr *value) { m_value = value; }

    PNODE_IMPL_CORE(CompositeField)
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

    virtual StructTy *get_type() override { return static_cast<StructTy *>(m_type); }

    StructDefMethods &get_methods() { return m_methods; }
    void add_method(FnDecl *method);
    void add_methods(std::initializer_list<FnDecl *> methods);
    void clear_methods();
    void remove_method(FnDecl *method);

    StructDefStaticMethods &get_static_methods() { return m_static_methods; }
    void add_static_method(FnDecl *method);
    void add_static_methods(std::initializer_list<FnDecl *> methods);
    void clear_static_methods();
    void remove_static_method(FnDecl *method);

    StructDefFields &get_fields() { return m_fields; }
    void add_field(CompositeField *field);
    void add_fields(std::initializer_list<CompositeField *> fields);
    void clear_fields();
    void remove_field(CompositeField *field);

    PNODE_IMPL_CORE(StructDef)
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

    virtual GroupTy *get_type() override { return static_cast<GroupTy *>(m_type); }

    GroupDefMethods &get_methods() { return m_methods; }
    void add_method(FnDecl *method);
    void add_methods(std::initializer_list<FnDecl *> methods);
    void clear_methods();
    void remove_method(FnDecl *method);

    GroupDefStaticMethods &get_static_methods() { return m_static_methods; }
    void add_static_method(FnDecl *method);
    void add_static_methods(std::initializer_list<FnDecl *> methods);
    void clear_static_methods();
    void remove_static_method(FnDecl *method);

    GroupDefFields &get_fields() { return m_fields; }
    void add_field(CompositeField *field);
    void add_fields(std::initializer_list<CompositeField *> fields);
    void clear_fields();
    void remove_field(CompositeField *field);

    PNODE_IMPL_CORE(GroupDef);
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

    virtual RegionTy *get_type() override { return static_cast<RegionTy *>(m_type); }

    RegionDefMethods &get_methods() { return m_methods; }
    void add_method(FnDecl *method);
    void add_methods(std::initializer_list<FnDecl *> methods);
    void clear_methods();
    void remove_method(FnDecl *method);

    RegionDefStaticMethods &get_static_methods() { return m_static_methods; }
    void add_static_method(FnDecl *method);
    void add_static_methods(std::initializer_list<FnDecl *> methods);
    void clear_static_methods();
    void remove_static_method(FnDecl *method);

    RegionDefFields &get_fields() { return m_fields; }
    void add_field(CompositeField *field);
    void add_fields(std::initializer_list<CompositeField *> fields);
    void clear_fields();
    void remove_field(CompositeField *field);

    PNODE_IMPL_CORE(RegionDef);
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

    virtual UnionTy *get_type() override { return static_cast<UnionTy *>(m_type); }

    UnionDefMethods &get_methods() { return m_methods; }
    void add_method(FnDecl *method);
    void add_methods(std::initializer_list<FnDecl *> methods);
    void clear_methods();
    void remove_method(FnDecl *method);

    UnionDefStaticMethods &get_static_methods() { return m_static_methods; }
    void add_static_method(FnDecl *method);
    void add_static_methods(std::initializer_list<FnDecl *> methods);
    void clear_static_methods();
    void remove_static_method(FnDecl *method);

    UnionDefFields &get_fields() { return m_fields; }
    void add_field(CompositeField *field);
    void add_fields(std::initializer_list<CompositeField *> fields);
    void clear_fields();
    void remove_field(CompositeField *field);

    PNODE_IMPL_CORE(UnionDef);
  };

  typedef std::pair<String, ConstExpr *> EnumItem;
  typedef std::vector<EnumItem, Arena<EnumItem>> EnumDefItems;

  class EnumDef : public Decl {
  protected:
    EnumDefItems m_items;

  public:
    EnumDef(String name = "", Type *type = nullptr, std::initializer_list<EnumItem> items = {})
        : Decl(name, type), m_items(items) {}
    EnumDef(String name, Type *type, const EnumDefItems &items)
        : Decl(name, type), m_items(items) {}

    virtual Type *get_type() override { return static_cast<Type *>(m_type); }

    EnumDefItems &get_items() { return m_items; }
    void add_item(EnumItem item);
    void add_items(std::initializer_list<EnumItem> items);
    void clear_items();
    void remove_item(EnumItem item);

    PNODE_IMPL_CORE(EnumDef)
  };

  typedef std::set<String, std::less<String>, Arena<String>> SubsystemDeps;

  class SubsystemDecl : public Decl {
  protected:
    Block *m_body;
    SubsystemDeps m_deps;

  public:
    SubsystemDecl(String name = "", Block *body = nullptr, SubsystemDeps deps = {})
        : Decl(name, nullptr), m_body(body), m_deps(deps) {}

    Block *get_body() { return m_body; }
    void set_body(Block *body) { m_body = body; }

    SubsystemDeps &get_deps() { return m_deps; }
    void add_dep(String dep);
    void add_deps(const SubsystemDeps &deps);
    void clear_deps();
    void remove_dep(String dep);

    PNODE_IMPL_CORE(SubsystemDecl)
  };

  class ExportDecl : public Decl {
  protected:
    Block *m_body;
    String m_abi_name;

  public:
    ExportDecl(std::initializer_list<Stmt *> body = {}, String abi_name = "")
        : Decl("", nullptr), m_body(Block::get(body)), m_abi_name(abi_name) {}
    ExportDecl(Block *content, String abi_name = "")
        : Decl("", nullptr), m_body(content), m_abi_name(abi_name) {}

    Block *get_body() { return m_body; }
    void set_body(Block *body) { m_body = body; }

    String get_abi_name() { return m_abi_name; }
    void set_abi_name(String abi_name) { m_abi_name = abi_name; }

    PNODE_IMPL_CORE(ExportDecl)
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
