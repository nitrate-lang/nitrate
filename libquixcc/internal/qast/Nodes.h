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

#ifndef __QUIXCC_QAST_NODES_H__
#define __QUIXCC_QAST_NODES_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <core/Interfaces.h>
#include <quixcc/Library.h>

#include <cassert>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>
#include <variant>
#include <vector>

namespace libquixcc::qast {
  class ArenaAllocatorImpl {
    quixcc_arena_t m_arena;

public:
    ArenaAllocatorImpl();
    ~ArenaAllocatorImpl();

    void *allocate(std::size_t bytes);
    void deallocate(void *ptr) noexcept;
  };

  extern thread_local ArenaAllocatorImpl g_allocator;

  template <class T> struct Arena {
    typedef T value_type;

    Arena() = default;

    template <class U> constexpr Arena(const Arena<U> &) noexcept {}

    [[nodiscard]] T *allocate(std::size_t n) {
      return static_cast<T *>(g_allocator.allocate(sizeof(T) * n));
    }

    void deallocate(T *p, std::size_t n) noexcept {
      (void)n;
      (void)p;
    }
  };

  template <class T, class U> bool operator==(const Arena<T> &, const Arena<U> &) { return true; }

  template <class T, class U> bool operator!=(const Arena<T> &, const Arena<U> &) { return false; }

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
}; // namespace libquixcc::qast

#define NODE_IMPL_CORE(__typename)                                                                 \
  protected:                                                                                       \
  virtual bool verify_impl(std::ostream &os) const override;                                       \
                                                                                                   \
  protected:                                                                                       \
  virtual void canonicalize_impl() override;                                                       \
                                                                                                   \
  protected:                                                                                       \
  virtual void print_impl(std::ostream &os, bool debug) const override;                            \
                                                                                                   \
  public:                                                                                          \
  virtual __typename *clone_impl() const;                                                          \
                                                                                                   \
  public:                                                                                          \
  public:                                                                                          \
  template <typename T = __typename, typename... Args> static __typename *get(Args &&...args) {    \
    void *ptr = Arena<__typename>().allocate(1);                                                   \
    return new (ptr) __typename(std::forward<Args>(args)...);                                      \
  }                                                                                                \
                                                                                                   \
  public:                                                                                          \
  virtual __typename *clone(ArenaAllocatorImpl &arena = g_allocator) const override {              \
    ArenaAllocatorImpl old = g_allocator;                                                          \
    g_allocator = arena;                                                                           \
    __typename *node = clone_impl();                                                               \
    g_allocator = old;                                                                             \
    return node;                                                                                   \
  }

class quixcc_ast_node_t : public libquixcc::core::IDumpable {
  public:
  quixcc_ast_node_t() = default;
  virtual ~quixcc_ast_node_t() = default;
};

namespace libquixcc::qast {
  enum class UnaryOp {
    UNKNOWN,
  };

  enum class BinOp {
    UNKNOWN,
  };

  class Node : public quixcc_ast_node_t {
protected:
    virtual bool verify_impl(std::ostream &os) const = 0;
    virtual void canonicalize_impl() = 0;
    virtual void print_impl(std::ostream &os, bool debug) const override = 0;
    virtual Node *clone_impl() const = 0;

public:
    Node() = default;
    virtual ~Node() override = default;

    uint32_t this_sizeof() const;
    quixcc_ast_ntype_t this_typeid() const;
    const char *this_nameof() const;

    size_t children_count(bool recursive = true) const;
    bool inherits_from(quixcc_ast_ntype_t type) const;

    bool is_type() const;
    bool is_stmt() const;
    bool is_decl() const;
    bool is_expr() const;
    bool is_const_expr() const;

    template <typename T> const T *as() const {
#if !defined(NDEBUG)
      auto p = dynamic_cast<const T *>(this);

      if (!p) {
        const char *this_str = typeid(*this).name();
        const char *other_str = typeid(T).name();

        quixcc_panicf("quixcc_ast_node_t::as(const %s *this): Invalid cast from `%s` to "
                      "`%s`.",
                      this_str, this_str, other_str);
        __builtin_unreachable();
      }
      return p;
#else
      return reinterpret_cast<const T *>(this);
#endif
    }

    template <typename T> T *as() {
#if !defined(NDEBUG)
      auto p = dynamic_cast<T *>(this);

      if (!p) {
        const char *this_str = typeid(*this).name();
        const char *other_str = typeid(T).name();

        quixcc_panicf("quixcc_ast_node_t::as(%s *this): Invalid cast from `%s` to `%s`.", this_str,
                      this_str, other_str);
        __builtin_unreachable();
      }
      return p;
#else
      return reinterpret_cast<T *>(this);
#endif
    }

    template <typename T> bool is() const { return typeid(*this) == typeid(T); }

    bool is(const quixcc_ast_ntype_t type) const;
    bool verify(std::ostream &os = std::cerr) const;
    void canonicalize();
    virtual Node *clone(ArenaAllocatorImpl &arena = g_allocator) const = 0;

    static const char *type_name(quixcc_ast_ntype_t type);
  };

  class Stmt : public Node {
public:
    Stmt() = default;
    virtual ~Stmt() override = default;

    virtual Stmt *clone(ArenaAllocatorImpl &arena = g_allocator) const = 0;
  };

  class Type : public Node {
protected:
    virtual uint64_t infer_size_bits_impl() const = 0;

public:
    Type() = default;
    virtual ~Type() override = default;

    uint64_t infer_size_bits() const;
    uint64_t infer_size() const;

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

    virtual Type *clone(ArenaAllocatorImpl &arena = g_allocator) const = 0;
  };

  typedef std::set<std::string_view, std::less<std::string_view>, Arena<std::string_view>> DeclTags;

  class Decl : public Stmt {
protected:
    DeclTags m_tags;
    std::string_view m_name;
    Type *m_type;

    virtual Type *infer_type_impl() const = 0;

public:
    Decl(std::string_view name = "", Type *type = nullptr,
         std::initializer_list<std::string_view> tags = {})
        : m_tags(tags), m_name(name), m_type(type) {}
    virtual ~Decl() override = default;

    std::string_view get_name() const;
    void set_name(std::string_view name);

    virtual Type *get_type() const;
    void set_type(Type *type);

    const DeclTags &get_tags() const;
    void add_tag(std::string_view tag);
    void add_tags(std::initializer_list<std::string_view> tags);
    void clear_tags();
    void remove_tag(std::string_view tag);

    Type *infer_type() const;

    virtual Decl *clone(ArenaAllocatorImpl &arena = g_allocator) const = 0;
  };

  class Expr : public Node {
protected:
    Type *m_type;

    virtual Type *infer_type_impl() const = 0;
    virtual bool is_const_impl() const = 0;
    virtual bool is_stochastic_impl() const = 0;

public:
    Expr() : m_type(nullptr) {}
    virtual ~Expr() override = default;

    Type *infer_type() const;
    bool is_const() const;
    bool is_stochastic() const;
    bool is_binexpr() const;
    bool is_unaryexpr() const;
    bool is_ternaryexpr() const;

    virtual Expr *clone(ArenaAllocatorImpl &arena = g_allocator) const = 0;
  };

  class ConstExpr : public Expr {
protected:
    Expr *m_value;

    template <typename> struct is_std_vector : std::false_type {};

    template <typename T, typename A> struct is_std_vector<std::vector<T, A>> : std::true_type {};

    template <typename T> struct is_std_map : std::false_type {};

    template <typename K, typename V, typename C, typename A>
    struct is_std_map<std::map<K, V, C, A>> : std::true_type {};

    template <typename T> struct is_std_set : std::false_type {};

    template <typename K, typename C, typename A>
    struct is_std_set<std::set<K, C, A>> : std::true_type {};

    template <typename> struct is_std_tuple : std::false_type {};

    template <typename... T> struct is_std_tuple<std::tuple<T...>> : std::true_type {};

    template <typename T> T do_eval() const {
      /// TODO: Implement this.
      quixcc_panicf("ConstExpr::do_eval<T>(): Not implemented.");
    }

    virtual Type *infer_type_impl() const override final;
    virtual bool is_const_impl() const override final;
    virtual bool is_stochastic_impl() const override final;

public:
    ConstExpr(Expr *value = nullptr) : m_value(value) {}
    virtual ~ConstExpr() override = default;

    template <typename T> T eval_as() const {
      if constexpr (std::is_same_v<T, bool>) {
        assert(infer_type()->is_bool());
        return do_eval<bool>();
      } else if constexpr (std::is_integral_v<T>) {
        assert(infer_type()->is_integral());
        return do_eval<T>();
      } else if constexpr (std::is_floating_point_v<T>) {
        assert(infer_type()->is_floating_point());
        return do_eval<T>();
      } else if constexpr (std::is_same_v<T, std::string_view>) {
        assert(infer_type()->is_string());
        return do_eval<std::string_view>();
      } else if constexpr (is_std_vector<T>::value) {
#if !defined(NDEBUG)
        Type *type = infer_type();
        assert(type->is_vector() || type->is_array());
#endif
        return do_eval<T>();
      } else if constexpr (is_std_map<T>::value) {
#if !defined(NDEBUG)
        Type *type = infer_type();
        assert(type->is_map());
#endif
        return do_eval<T>();
      } else if constexpr (is_std_set<T>::value) {
#if !defined(NDEBUG)
        Type *type = infer_type();
        assert(type->is_set());
#endif
        return do_eval<T>();
      } else if constexpr (is_std_tuple<T>::value) {
#if !defined(NDEBUG)
        Type *type = infer_type();
        assert(type->is_tuple());
#endif
        return do_eval<T>();
      } else {
        quixcc_panicf("ConstExpr::eval_as<T>(): Invalid type `%s`.", typeid(T).name());
        __builtin_unreachable();
      }
    }

    Expr *get_value() const;
    void set_value(Expr *value);

    NODE_IMPL_CORE(ConstExpr)
  };

  class LitExpr : public ConstExpr {
public:
    LitExpr() = default;
    virtual ~LitExpr() override = default;

    virtual LitExpr *clone(ArenaAllocatorImpl &arena = g_allocator) const = 0;
  };

  class FlowStmt : public Stmt {
public:
    FlowStmt() = default;
    virtual ~FlowStmt() override = default;

    virtual FlowStmt *clone(ArenaAllocatorImpl &arena = g_allocator) const = 0;
  };

  class DeclStmt : public Stmt {
public:
    DeclStmt() = default;
    virtual ~DeclStmt() override = default;

    virtual DeclStmt *clone(ArenaAllocatorImpl &arena = g_allocator) const = 0;
  };

  class TypeBuiltin : public Type {
public:
    TypeBuiltin() = default;
    virtual ~TypeBuiltin() override = default;

    virtual TypeBuiltin *clone(ArenaAllocatorImpl &arena = g_allocator) const = 0;
  };

  class TypeComplex : public Type {
public:
    TypeComplex() = default;
    virtual ~TypeComplex() override = default;

    virtual TypeComplex *clone(ArenaAllocatorImpl &arena = g_allocator) const = 0;
  };

  class TypeComposite : public Type {
public:
    TypeComposite() = default;
    virtual ~TypeComposite() override = default;

    virtual TypeComposite *clone(ArenaAllocatorImpl &arena = g_allocator) const = 0;
  };

  class UnresolvedType : public Type {
    std::string_view m_name;

    virtual uint64_t infer_size_bits_impl() const override final;

public:
    UnresolvedType() = default;
    virtual ~UnresolvedType() override = default;

    NODE_IMPL_CORE(UnresolvedType)
  };

  class U1 : public TypeBuiltin {
    virtual uint64_t infer_size_bits_impl() const override final;

public:
    U1() = default;
    virtual ~U1() override = default;

    NODE_IMPL_CORE(U1)
  };

  class U8 : public TypeBuiltin {
    virtual uint64_t infer_size_bits_impl() const override final;

public:
    U8() = default;
    virtual ~U8() override = default;

    NODE_IMPL_CORE(U8)
  };

  class U16 : public TypeBuiltin {
    virtual uint64_t infer_size_bits_impl() const override final;

public:
    U16() = default;
    virtual ~U16() override = default;

    NODE_IMPL_CORE(U16)
  };

  class U32 : public TypeBuiltin {
    virtual uint64_t infer_size_bits_impl() const override final;

public:
    U32() = default;
    virtual ~U32() override = default;

    NODE_IMPL_CORE(U32)
  };

  class U64 : public TypeBuiltin {
    virtual uint64_t infer_size_bits_impl() const override final;

public:
    U64() = default;
    virtual ~U64() override = default;

    NODE_IMPL_CORE(U64)
  };

  class U128 : public TypeBuiltin {
    virtual uint64_t infer_size_bits_impl() const override final;

public:
    U128() = default;
    virtual ~U128() override = default;

    NODE_IMPL_CORE(U128)
  };

  class I8 : public TypeBuiltin {
    virtual uint64_t infer_size_bits_impl() const override final;

public:
    I8() = default;
    virtual ~I8() override = default;

    NODE_IMPL_CORE(I8)
  };

  class I16 : public TypeBuiltin {
    virtual uint64_t infer_size_bits_impl() const override final;

public:
    I16() = default;
    virtual ~I16() override = default;

    NODE_IMPL_CORE(I16)
  };

  class I32 : public TypeBuiltin {
    virtual uint64_t infer_size_bits_impl() const override final;

public:
    I32() = default;
    virtual ~I32() override = default;

    NODE_IMPL_CORE(I32)
  };

  class I64 : public TypeBuiltin {
    virtual uint64_t infer_size_bits_impl() const override final;

public:
    I64() = default;
    virtual ~I64() override = default;

    NODE_IMPL_CORE(I64)
  };

  class I128 : public TypeBuiltin {
    virtual uint64_t infer_size_bits_impl() const override final;

public:
    I128() = default;
    virtual ~I128() override = default;

    NODE_IMPL_CORE(I128)
  };

  class F32 : public TypeBuiltin {
    virtual uint64_t infer_size_bits_impl() const override final;

public:
    F32() = default;
    virtual ~F32() override = default;

    NODE_IMPL_CORE(F32)
  };

  class F64 : public TypeBuiltin {
    virtual uint64_t infer_size_bits_impl() const override final;

public:
    F64() = default;
    virtual ~F64() override = default;

    NODE_IMPL_CORE(F64)
  };

  class Void : public TypeBuiltin {
    virtual uint64_t infer_size_bits_impl() const override final;

public:
    Void() = default;
    virtual ~Void() override = default;

    NODE_IMPL_CORE(Void)
  };

  class StringTy : public TypeBuiltin {
    virtual uint64_t infer_size_bits_impl() const override final;

public:
    StringTy() = default;
    virtual ~StringTy() override = default;

    NODE_IMPL_CORE(StringTy)
  };

  class PtrTy : public TypeComplex {
    Type *m_item;
    bool m_is_volatile;

    virtual uint64_t infer_size_bits_impl() const override final;

public:
    PtrTy(Type *item = nullptr, bool is_volatile = false)
        : m_item(item), m_is_volatile(is_volatile) {}
    virtual ~PtrTy() override = default;

    Type *get_item() const;
    void set_item(Type *item);

    bool is_volatile() const;
    void set_volatile(bool is_volatile);

    NODE_IMPL_CORE(PtrTy)
  };

  class OpaqueTy : public TypeComplex {
    std::string_view m_name;

    virtual uint64_t infer_size_bits_impl() const override final;

public:
    OpaqueTy(std::string_view name = "") : m_name(name) {}
    virtual ~OpaqueTy() override = default;

    std::string_view get_name() const;
    void set_name(std::string_view name);

    NODE_IMPL_CORE(OpaqueTy)
  };

  class VectorTy : public TypeComposite {
    Type *m_item;

    virtual uint64_t infer_size_bits_impl() const override final;

public:
    VectorTy(Type *item = nullptr) : m_item(item) {}
    virtual ~VectorTy() override = default;

    Type *get_item() const;
    void set_item(Type *item);

    NODE_IMPL_CORE(VectorTy)
  };

  class SetTy : public TypeComposite {
    Type *m_item;

    virtual uint64_t infer_size_bits_impl() const override final;

public:
    SetTy(Type *item = nullptr) : m_item(item) {}
    virtual ~SetTy() override = default;

    Type *get_item() const;
    void set_item(Type *item);

    NODE_IMPL_CORE(SetTy)
  };

  class MapTy : public TypeComposite {
    Type *m_key;
    Type *m_value;

    virtual uint64_t infer_size_bits_impl() const override final;

public:
    MapTy(Type *key = nullptr, Type *value = nullptr) : m_key(key), m_value(value) {}
    virtual ~MapTy() override = default;

    Type *get_key() const;
    void set_key(Type *key);

    Type *get_value() const;
    void set_value(Type *value);

    NODE_IMPL_CORE(MapTy)
  };

  class TupleTy : public TypeComposite {
    std::vector<Type *, Arena<Type *>> m_items;

    virtual uint64_t infer_size_bits_impl() const override final;

public:
    TupleTy(std::initializer_list<Type *> items = {}) : m_items(items) {}
    TupleTy(const std::vector<Type *, Arena<Type *>> &items) : m_items(items) {}
    virtual ~TupleTy() override = default;

    const std::vector<Type *, Arena<Type *>> &get_items() const;
    void add_item(Type *item);
    void add_items(std::initializer_list<Type *> items);
    void clear_items();
    void remove_item(Type *item);

    NODE_IMPL_CORE(TupleTy)
  };

  class OptionalTy : public TypeComposite {
    Type *m_item;

    virtual uint64_t infer_size_bits_impl() const override final;

public:
    OptionalTy(Type *item = nullptr) : m_item(item) {}
    virtual ~OptionalTy() override = default;

    Type *get_item() const;
    void set_item(Type *item);

    NODE_IMPL_CORE(OptionalTy)
  };

  class ArrayTy : public TypeComposite {
    Type *m_item;
    ConstExpr *m_size;

    virtual uint64_t infer_size_bits_impl() const override final;

public:
    ArrayTy(Type *item = nullptr, ConstExpr *size = nullptr) : m_item(item), m_size(size) {}
    virtual ~ArrayTy() override = default;

    Type *get_item() const;
    void set_item(Type *item);

    ConstExpr *get_size() const;
    void set_size(ConstExpr *size);

    NODE_IMPL_CORE(ArrayTy)
  };

  class EnumTy : public TypeComplex {
    std::string_view m_name;
    Type *m_memtype;

    virtual uint64_t infer_size_bits_impl() const override final;

public:
    EnumTy(std::string_view name = "", Type *memtype = nullptr)
        : m_name(name), m_memtype(memtype) {}
    virtual ~EnumTy() override = default;

    std::string_view get_name() const;
    void set_name(std::string_view name);

    Type *get_memtype() const;
    void set_memtype(Type *memtype);

    NODE_IMPL_CORE(EnumTy)
  };

  class MutTy : public TypeComplex {
    Type *m_item;

    virtual uint64_t infer_size_bits_impl() const override final;

public:
    MutTy(Type *item = nullptr) : m_item(item) {}
    virtual ~MutTy() override = default;

    Type *get_item() const;
    void set_item(Type *item);

    NODE_IMPL_CORE(MutTy)
  };

  typedef std::pair<std::string_view, Type *> StructItem;
  typedef std::vector<StructItem, Arena<StructItem>> StructItems;

  class StructTy : public TypeComposite {
    StructItems m_items;

    virtual uint64_t infer_size_bits_impl() const override final;

public:
    StructTy(std::initializer_list<StructItem> items = {}) : m_items(items) {}
    StructTy(const StructItems &items) : m_items(items) {}
    virtual ~StructTy() override = default;

    const StructItems &get_items() const;
    void add_item(std::string_view name, Type *type);
    void add_items(std::initializer_list<StructItem> items);
    void clear_items();
    void remove_item(std::string_view name);

    NODE_IMPL_CORE(StructTy)
  };

  typedef std::vector<Type *, Arena<Type *>> GroupTyItems;

  class GroupTy : public TypeComposite {
    GroupTyItems m_items;

    virtual uint64_t infer_size_bits_impl() const override final;

public:
    GroupTy(std::initializer_list<Type *> items = {}) : m_items(items) {}
    GroupTy(const GroupTyItems &items) : m_items(items) {}
    virtual ~GroupTy() override = default;

    const GroupTyItems &get_items() const;
    void add_item(Type *item);
    void add_items(std::initializer_list<Type *> items);
    void clear_items();
    void remove_item(Type *item);

    NODE_IMPL_CORE(GroupTy)
  };

  typedef std::vector<Type *, Arena<Type *>> RegionTyItems;

  class RegionTy : public TypeComposite {
    RegionTyItems m_items;

    virtual uint64_t infer_size_bits_impl() const override final;

public:
    RegionTy(std::initializer_list<Type *> items = {}) : m_items(items) {}
    RegionTy(const RegionTyItems &items) : m_items(items) {}
    virtual ~RegionTy() override = default;

    const RegionTyItems &get_items() const;
    void add_item(Type *item);
    void add_items(std::initializer_list<Type *> items);
    void clear_items();
    void remove_item(Type *item);

    NODE_IMPL_CORE(RegionTy)
  };

  typedef std::vector<Type *, Arena<Type *>> UnionTyItems;

  class UnionTy : public TypeComposite {
    UnionTyItems m_items;

    virtual uint64_t infer_size_bits_impl() const override final;

public:
    UnionTy(std::initializer_list<Type *> items = {}) : m_items(items) {}
    UnionTy(const UnionTyItems &items) : m_items(items) {}
    UnionTy() = default;
    virtual ~UnionTy() override = default;

    const UnionTyItems &get_items() const;
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

  typedef std::tuple<std::string_view, Type *, Expr *> FuncParam;
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

    virtual uint64_t infer_size_bits_impl() const override final;

public:
    FuncTy()
        : m_return(nullptr), m_purity(FuncPurity::IMPURE_THREAD_UNSAFE), m_variadic(false),
          m_is_foreign(false), m_crashpoint(false), m_noexcept(false), m_noreturn(false) {}
    FuncTy(Type *return_type, FuncParams parameters, bool variadic = false,
           FuncPurity purity = FuncPurity::IMPURE_THREAD_UNSAFE, bool is_foreign = false,
           bool crashpoint = false, bool noexcept_ = false, bool noreturn = false)
        : m_params(parameters), m_return(return_type), m_purity(purity), m_variadic(variadic),
          m_is_foreign(is_foreign), m_crashpoint(crashpoint), m_noexcept(noexcept_),
          m_noreturn(noreturn) {
      assert(!noreturn || (purity == FuncPurity::IMPURE_THREAD_UNSAFE ||
                           purity == FuncPurity::IMPURE_THREAD_SAFE));
    }
    FuncTy(Type *return_type, std::vector<Type *, Arena<Type *>> parameters, bool variadic = false,
           FuncPurity purity = FuncPurity::IMPURE_THREAD_UNSAFE, bool is_foreign = false,
           bool crashpoint = false, bool noexcept_ = false, bool noreturn = false)
        : m_return(return_type), m_purity(purity), m_variadic(variadic), m_is_foreign(is_foreign),
          m_crashpoint(crashpoint), m_noexcept(noexcept_), m_noreturn(noreturn) {
      assert(!noreturn || (purity == FuncPurity::IMPURE_THREAD_UNSAFE ||
                           purity == FuncPurity::IMPURE_THREAD_SAFE));

      for (size_t i = 0; i < parameters.size(); i++) {
        m_params.push_back({"_" + std::to_string(i), parameters[i], nullptr});
      }
    }
    virtual ~FuncTy() override = default;

    bool is_noreturn() const;
    void set_noreturn(bool noreturn);

    Type *get_return_ty() const;
    void set_return_ty(Type *return_ty);

    const FuncParams &get_params() const;
    void add_param(std::string_view name, Type *type, Expr *default_val = nullptr);
    void add_params(std::initializer_list<FuncParam> params);
    void clear_params();
    void remove_param(std::string_view name);

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
    UnaryOp m_op;

    virtual Type *infer_type_impl() const override final;
    virtual bool is_const_impl() const override final;
    virtual bool is_stochastic_impl() const override final;

public:
    UnaryExpr(UnaryOp op = UnaryOp::UNKNOWN, Expr *rhs = nullptr) : m_rhs(rhs), m_op(op) {}
    virtual ~UnaryExpr() override = default;

    Expr *get_rhs() const;
    void set_rhs(Expr *rhs);

    UnaryOp get_op() const;
    void set_op(UnaryOp op);

    NODE_IMPL_CORE(UnaryExpr)
  };

  class BinExpr : public Expr {
protected:
    Expr *m_lhs;
    Expr *m_rhs;
    BinOp m_op;

    virtual Type *infer_type_impl() const override final;
    virtual bool is_const_impl() const override final;
    virtual bool is_stochastic_impl() const override final;

public:
    BinExpr(Expr *lhs = nullptr, BinOp op = BinOp::UNKNOWN, Expr *rhs = nullptr)
        : m_lhs(lhs), m_rhs(rhs), m_op(op) {}
    virtual ~BinExpr() override = default;

    Expr *get_lhs() const;
    void set_lhs(Expr *lhs);

    Expr *get_rhs() const;
    void set_rhs(Expr *rhs);

    BinOp get_op() const;
    void set_op(BinOp op);

    NODE_IMPL_CORE(BinExpr)
  };

  class TernaryExpr : public Expr {
protected:
    Expr *m_cond;
    Expr *m_lhs;
    Expr *m_rhs;

    virtual Type *infer_type_impl() const override final;
    virtual bool is_const_impl() const override final;
    virtual bool is_stochastic_impl() const override final;

public:
    TernaryExpr(Expr *cond = nullptr, Expr *lhs = nullptr, Expr *rhs = nullptr)
        : m_cond(cond), m_lhs(lhs), m_rhs(rhs) {}
    virtual ~TernaryExpr() override = default;

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
    std::string_view m_value;

public:
    ConstInt(std::string_view value = "") : m_value(value) {}
    ConstInt(uint64_t value) : m_value(std::to_string(value)) {}
    virtual ~ConstInt() override = default;

    std::string_view get_value() const;

    NODE_IMPL_CORE(ConstInt)
  };

  class ConstFloat : public LitExpr {
    std::string_view m_value;

public:
    ConstFloat(std::string_view value = "") : m_value(value) {}
    ConstFloat(double value) : m_value(std::to_string(value)) {}
    virtual ~ConstFloat() override = default;

    std::string_view get_value() const;

    NODE_IMPL_CORE(ConstFloat)
  };

  class ConstBool : public LitExpr {
    bool m_value;

public:
    ConstBool(bool value = false) : m_value(value) {}
    virtual ~ConstBool() override = default;

    bool get_value() const;

    NODE_IMPL_CORE(ConstBool)
  };

  class ConstString : public LitExpr {
    std::string_view m_value;

public:
    ConstString(std::string_view value = "") : m_value(value) {}
    virtual ~ConstString() override = default;

    std::string_view get_value() const;

    NODE_IMPL_CORE(ConstString)
  };

  class ConstChar : public LitExpr {
    std::string_view m_value;

public:
    ConstChar(std::string_view value = "") : m_value(value) {}
    virtual ~ConstChar() override = default;

    std::string_view get_value() const;

    NODE_IMPL_CORE(ConstChar)
  };

  class ConstNull : public LitExpr {
public:
    ConstNull() = default;
    virtual ~ConstNull() override = default;

    NODE_IMPL_CORE(ConstNull)
  };

  class ConstUndef : public LitExpr {
public:
    ConstUndef() = default;
    virtual ~ConstUndef() override = default;

    NODE_IMPL_CORE(ConstUndef)
  };

  ///=============================================================================

  typedef std::pair<const std::string_view, Expr *> CallArg;
  typedef std::map<std::string_view, Expr *, std::less<std::string_view>, Arena<CallArg>> CallArgs;

  class Call : public Expr {
protected:
    Expr *m_func;
    CallArgs m_args;

    virtual Type *infer_type_impl() const override final;
    virtual bool is_const_impl() const override final;
    virtual bool is_stochastic_impl() const override final;

public:
    Call(Expr *func = nullptr, CallArgs args = {}) : m_func(func), m_args(args) {}
    virtual ~Call() override = default;

    Expr *get_func() const;
    void set_func(Expr *func);

    const CallArgs &get_args() const;
    void add_arg(std::string_view name, Expr *arg);
    void add_args(std::map<std::string_view, Expr *> args);
    void clear_args();
    void remove_arg(std::string_view name);

    NODE_IMPL_CORE(Call)
  };

  typedef std::vector<Expr *, Arena<Expr *>> ListData;

  class List : public Expr {
protected:
    ListData m_items;

    virtual Type *infer_type_impl() const override final;
    virtual bool is_const_impl() const override final;
    virtual bool is_stochastic_impl() const override final;

public:
    List(std::initializer_list<Expr *> items = {}) : m_items(items) {}
    List(const ListData &items) : m_items(items) {}
    virtual ~List() override = default;

    const ListData &get_items() const;
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

    virtual Type *infer_type_impl() const override final;
    virtual bool is_const_impl() const override final;
    virtual bool is_stochastic_impl() const override final;

public:
    Assoc(Expr *key = nullptr, Expr *value = nullptr) : m_key(key), m_value(value) {}
    virtual ~Assoc() override = default;

    Expr *get_key() const;
    void set_key(Expr *key);

    Expr *get_value() const;
    void set_value(Expr *value);

    NODE_IMPL_CORE(Assoc)
  };

  class Field : public Expr {
protected:
    Expr *m_base;
    std::string_view m_field;

    virtual Type *infer_type_impl() const override final;
    virtual bool is_const_impl() const override final;
    virtual bool is_stochastic_impl() const override final;

public:
    Field(Expr *base = nullptr, std::string_view field = "") : m_base(base), m_field(field) {}
    virtual ~Field() override = default;

    Expr *get_base() const;
    void set_base(Expr *base);

    std::string_view get_field() const;
    void set_field(std::string_view field);

    NODE_IMPL_CORE(Field)
  };

  class Index : public Expr {
protected:
    Expr *m_base;
    Expr *m_index;

    virtual Type *infer_type_impl() const override final;
    virtual bool is_const_impl() const override final;
    virtual bool is_stochastic_impl() const override final;

public:
    Index(Expr *base = nullptr, Expr *index = nullptr) : m_base(base), m_index(index) {}
    virtual ~Index() override = default;

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

    virtual Type *infer_type_impl() const override final;
    virtual bool is_const_impl() const override final;
    virtual bool is_stochastic_impl() const override final;

public:
    Slice(Expr *base = nullptr, Expr *start = nullptr, Expr *end = nullptr)
        : m_base(base), m_start(start), m_end(end) {}
    virtual ~Slice() override = default;

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
    std::string_view m_value;
    FStringArgs m_items;

    virtual Type *infer_type_impl() const override final;
    virtual bool is_const_impl() const override final;
    virtual bool is_stochastic_impl() const override final;

public:
    FString(std::string_view value = "", std::initializer_list<Expr *> items = {})
        : m_value(value), m_items(items) {}
    FString(std::string_view value, const FStringArgs &items) : m_value(value), m_items(items) {}
    virtual ~FString() override = default;

    std::string_view get_value() const;
    void set_value(std::string_view value);

    const FStringArgs &get_items() const;
    void add_item(Expr *item);
    void add_items(std::initializer_list<Expr *> items);
    void clear_items();
    void remove_item(Expr *item);

    NODE_IMPL_CORE(FString)
  };

  class Ident : public Expr {
    std::string_view m_name;

    virtual Type *infer_type_impl() const override final;
    virtual bool is_const_impl() const override final;
    virtual bool is_stochastic_impl() const override final;

public:
    Ident(std::string_view name = "") : m_name(name) {}
    virtual ~Ident() override = default;

    std::string_view get_name() const;
    void set_name(std::string_view name);

    NODE_IMPL_CORE(Ident)
  };

  ///=============================================================================

  typedef std::vector<Stmt *, Arena<Stmt *>> BlockItems;
  class Block : public Stmt {
protected:
    BlockItems m_items;

public:
    Block(std::initializer_list<Stmt *> items = {}) : m_items(items) {}
    Block(const BlockItems &items) : m_items(items) {}
    virtual ~Block() override = default;

    const BlockItems &get_items() const;
    void add_item(Stmt *item);
    void add_items(std::initializer_list<Stmt *> items);
    void clear_items();
    void remove_item(Stmt *item);

    NODE_IMPL_CORE(Block)
  };

  class ConstDecl : public Decl {
protected:
    Expr *m_value;

    virtual Type *infer_type_impl() const override final;

public:
    ConstDecl(std::string_view name = "", Type *type = nullptr, Expr *value = nullptr)
        : Decl(name, type), m_value(value) {}
    virtual ~ConstDecl() override = default;

    Expr *get_value() const;
    void set_value(Expr *value);

    NODE_IMPL_CORE(ConstDecl)
  };

  class VarDecl : public Decl {
protected:
    Expr *m_value;

    virtual Type *infer_type_impl() const override final;

public:
    VarDecl(std::string_view name = "", Type *type = nullptr, Expr *value = nullptr)
        : Decl(name, type), m_value(value) {}
    virtual ~VarDecl() override = default;

    Expr *get_value() const;
    void set_value(Expr *value);

    NODE_IMPL_CORE(VarDecl)
  };

  class LetDecl : public Decl {
protected:
    Expr *m_value;

    virtual Type *infer_type_impl() const override final;

public:
    LetDecl(std::string_view name = "", Type *type = nullptr, Expr *value = nullptr)
        : Decl(name, type), m_value(value) {}
    virtual ~LetDecl() override = default;

    Expr *get_value() const;
    void set_value(Expr *value);

    NODE_IMPL_CORE(LetDecl)
  };

  class InlineAsm : public Stmt {
protected:
    std::string_view m_code;
    std::vector<Expr *, Arena<Expr *>> m_args;

public:
    InlineAsm(std::string_view code = "", std::initializer_list<Expr *> args = {})
        : m_code(code), m_args(args) {}
    InlineAsm(std::string_view code, const std::vector<Expr *, Arena<Expr *>> &args)
        : m_code(code), m_args(args) {}
    virtual ~InlineAsm() override = default;

    std::string_view get_code() const;
    void set_code(std::string_view code);

    const std::vector<Expr *, Arena<Expr *>> &get_args() const;
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
    virtual ~IfStmt() override = default;

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
    virtual ~WhileStmt() override = default;

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
    Stmt *m_step;
    Stmt *m_body;

public:
    ForStmt(Stmt *init = nullptr, Expr *cond = nullptr, Stmt *step = nullptr, Stmt *body = nullptr)
        : m_init(init), m_cond(cond), m_step(step), m_body(body) {}
    virtual ~ForStmt() override = default;

    Stmt *get_init() const;
    void set_init(Stmt *init);

    Expr *get_cond() const;
    void set_cond(Expr *cond);

    Stmt *get_step() const;
    void set_step(Stmt *step);

    Stmt *get_body() const;
    void set_body(Stmt *body);

    NODE_IMPL_CORE(ForStmt)
  };

  class FormStmt : public FlowStmt {
protected:
    Expr *m_init;
    Expr *m_generator;
    Stmt *m_body;

public:
    FormStmt(Expr *init = nullptr, Expr *generator = nullptr, Stmt *body = nullptr)
        : m_init(init), m_generator(generator), m_body(body) {}
    virtual ~FormStmt() override = default;

    Expr *get_init() const;
    void set_init(Expr *init);

    Expr *get_generator() const;
    void set_generator(Expr *generator);

    Stmt *get_body() const;
    void set_body(Stmt *body);

    NODE_IMPL_CORE(FormStmt)
  };

  class ForeachStmt : public FlowStmt {
protected:
    std::string_view m_name;
    Expr *m_iter;
    Stmt *m_body;

public:
    ForeachStmt(std::string_view name = "", Expr *iter = nullptr, Stmt *body = nullptr)
        : m_name(name), m_iter(iter), m_body(body) {}
    virtual ~ForeachStmt() override = default;

    std::string_view get_name() const;
    void set_name(std::string_view name);

    Expr *get_iter() const;
    void set_iter(Expr *iter);

    Stmt *get_body() const;
    void set_body(Stmt *body);

    NODE_IMPL_CORE(ForeachStmt)
  };

  class BreakStmt : public FlowStmt {
public:
    BreakStmt() = default;
    virtual ~BreakStmt() override = default;

    NODE_IMPL_CORE(BreakStmt)
  };

  class ContinueStmt : public FlowStmt {
public:
    ContinueStmt() = default;
    virtual ~ContinueStmt() override = default;

    NODE_IMPL_CORE(ContinueStmt)
  };

  class ReturnStmt : public FlowStmt {
protected:
    Expr *m_value;

public:
    ReturnStmt(Expr *value = nullptr) : m_value(value) {}
    virtual ~ReturnStmt() override = default;

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
    virtual ~ReturnIfStmt() override = default;

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
    virtual ~RetZStmt() override = default;

    Expr *get_cond() const;
    void set_cond(Expr *cond);

    Expr *get_value() const;
    void set_value(Expr *value);

    NODE_IMPL_CORE(RetZStmt)
  };

  class RetVStmt : public FlowStmt {
protected:
    Expr *m_value;

public:
    RetVStmt(Expr *value = nullptr) : m_value(value) {}
    virtual ~RetVStmt() override = default;

    Expr *get_value() const;
    void set_value(Expr *value);

    NODE_IMPL_CORE(RetVStmt)
  };

  class CaseStmt : public FlowStmt {
protected:
    Expr *m_cond;
    Stmt *m_body;

public:
    CaseStmt(Expr *cond = nullptr, Stmt *body = nullptr) : m_cond(cond), m_body(body) {}
    virtual ~CaseStmt() override = default;

    Expr *get_cond() const;
    void set_cond(Expr *cond);

    Stmt *get_body() const;
    void set_body(Stmt *body);

    NODE_IMPL_CORE(CaseStmt)
  };

  class SwitchStmt : public FlowStmt {
protected:
    Expr *m_cond;
    std::vector<CaseStmt *, Arena<CaseStmt *>> m_cases;

public:
    SwitchStmt(Expr *cond = nullptr, std::initializer_list<CaseStmt *> cases = {})
        : m_cond(cond), m_cases(cases) {}
    SwitchStmt(Expr *cond, const std::vector<CaseStmt *, Arena<CaseStmt *>> &cases)
        : m_cond(cond), m_cases(cases) {}
    virtual ~SwitchStmt() override = default;

    Expr *get_cond() const;
    void set_cond(Expr *cond);

    const std::vector<CaseStmt *, Arena<CaseStmt *>> &get_cases() const;
    void add_case(CaseStmt *case_);
    void add_cases(std::initializer_list<CaseStmt *> cases);
    void clear_cases();
    void remove_case(CaseStmt *case_);

    NODE_IMPL_CORE(SwitchStmt)
  };

  ///=============================================================================

  class TypedefStmt : public Decl {
protected:
    virtual Type *infer_type_impl() const override final;

public:
    TypedefStmt(std::string_view name = "", Type *type = nullptr) : Decl(name, type) {}
    virtual ~TypedefStmt() override = default;

    NODE_IMPL_CORE(TypedefStmt)
  };

  class FnDecl : public Decl {
protected:
    virtual Type *infer_type_impl() const override final;

public:
    FnDecl(std::string_view name = "", FuncTy *type = nullptr) : Decl(name, type) {}
    virtual ~FnDecl() override = default;

    virtual FuncTy *get_type() const override;

    NODE_IMPL_CORE(FnDecl)
  };

  class FnDef : public FnDecl {
protected:
    Stmt *m_body;

public:
    FnDef(std::string_view name = "", FuncTy *type = nullptr, Stmt *body = nullptr)
        : FnDecl(name, type), m_body(body) {}
    virtual ~FnDef() override = default;

    std::string_view get_name() const;
    void set_name(std::string_view name);

    Stmt *get_body() const;
    void set_body(Stmt *body);

    NODE_IMPL_CORE(FnDef)
  };

  class CompositeField : public Decl {
protected:
    Expr *m_value;

    virtual Type *infer_type_impl() const override final;

public:
    CompositeField(std::string_view name = "", Type *type = nullptr, Expr *value = nullptr)
        : Decl(name, type), m_value(value) {}
    virtual ~CompositeField() override = default;

    Expr *get_value() const;
    void set_value(Expr *value);

    NODE_IMPL_CORE(CompositeField)
  };

  class StructDef : public Decl {
protected:
    std::vector<FnDecl *, Arena<FnDecl *>> m_methods;
    std::vector<FnDecl *, Arena<FnDecl *>> m_static_methods;
    std::vector<CompositeField *, Arena<CompositeField *>> m_fields;

    virtual Type *infer_type_impl() const override final;

public:
    StructDef(std::string_view name = "", StructTy *type = nullptr,
              std::initializer_list<CompositeField *> fields = {},
              std::initializer_list<FnDecl *> methods = {},
              std::initializer_list<FnDecl *> static_methods = {})
        : Decl(name, type), m_methods(methods), m_static_methods(static_methods), m_fields(fields) {
    }
    StructDef(std::string_view name, StructTy *type,
              const std::vector<CompositeField *, Arena<CompositeField *>> &fields,
              const std::vector<FnDecl *, Arena<FnDecl *>> &methods,
              const std::vector<FnDecl *, Arena<FnDecl *>> &static_methods)
        : Decl(name, type), m_methods(methods), m_static_methods(static_methods), m_fields(fields) {
    }
    virtual ~StructDef() override = default;

    virtual StructTy *get_type() const override;

    const std::vector<FnDecl *, Arena<FnDecl *>> &get_methods() const;
    void add_method(FnDecl *method);
    void add_methods(std::initializer_list<FnDecl *> methods);
    void clear_methods();
    void remove_method(FnDecl *method);

    const std::vector<FnDecl *, Arena<FnDecl *>> &get_static_methods() const;
    void add_static_method(FnDecl *method);
    void add_static_methods(std::initializer_list<FnDecl *> methods);
    void clear_static_methods();
    void remove_static_method(FnDecl *method);

    const std::vector<CompositeField *, Arena<CompositeField *>> &get_fields() const;
    void add_field(CompositeField *field);
    void add_fields(std::initializer_list<CompositeField *> fields);
    void clear_fields();
    void remove_field(CompositeField *field);

    NODE_IMPL_CORE(StructDef)
  };

  class GroupDef : public Decl {
protected:
    std::vector<FnDecl *, Arena<FnDecl *>> m_methods;
    std::vector<FnDecl *, Arena<FnDecl *>> m_static_methods;
    std::vector<CompositeField *, Arena<CompositeField *>> m_fields;

    virtual Type *infer_type_impl() const override final;

public:
    GroupDef(std::string_view name = "", GroupTy *type = nullptr,
             std::initializer_list<CompositeField *> fields = {},
             std::initializer_list<FnDecl *> methods = {},
             std::initializer_list<FnDecl *> static_methods = {})
        : Decl(name, type), m_methods(methods), m_static_methods(static_methods), m_fields(fields) {
    }
    GroupDef(std::string_view name, GroupTy *type,
             const std::vector<CompositeField *, Arena<CompositeField *>> &fields,
             const std::vector<FnDecl *, Arena<FnDecl *>> &methods,
             const std::vector<FnDecl *, Arena<FnDecl *>> &static_methods)
        : Decl(name, type), m_methods(methods), m_static_methods(static_methods), m_fields(fields) {
    }
    virtual ~GroupDef() override = default;

    virtual GroupTy *get_type() const override;

    const std::vector<FnDecl *, Arena<FnDecl *>> &get_methods() const;
    void add_method(FnDecl *method);
    void add_methods(std::initializer_list<FnDecl *> methods);
    void clear_methods();
    void remove_method(FnDecl *method);

    const std::vector<FnDecl *, Arena<FnDecl *>> &get_static_methods() const;
    void add_static_method(FnDecl *method);
    void add_static_methods(std::initializer_list<FnDecl *> methods);
    void clear_static_methods();
    void remove_static_method(FnDecl *method);

    const std::vector<CompositeField *, Arena<CompositeField *>> &get_fields() const;
    void add_field(CompositeField *field);
    void add_fields(std::initializer_list<CompositeField *> fields);
    void clear_fields();
    void remove_field(CompositeField *field);

    NODE_IMPL_CORE(GroupDef);
  };

  class RegionDef : public Decl {
protected:
    std::vector<FnDecl *, Arena<FnDecl *>> m_methods;
    std::vector<FnDecl *, Arena<FnDecl *>> m_static_methods;
    std::vector<CompositeField *, Arena<CompositeField *>> m_fields;

    virtual Type *infer_type_impl() const override final;

public:
    RegionDef(std::string_view name = "", RegionTy *type = nullptr,
              std::initializer_list<CompositeField *> fields = {},
              std::initializer_list<FnDecl *> methods = {},
              std::initializer_list<FnDecl *> static_methods = {})
        : Decl(name, type), m_methods(methods), m_static_methods(static_methods), m_fields(fields) {
    }
    RegionDef(std::string_view name, RegionTy *type,
              const std::vector<CompositeField *, Arena<CompositeField *>> &fields,
              const std::vector<FnDecl *, Arena<FnDecl *>> &methods,
              const std::vector<FnDecl *, Arena<FnDecl *>> &static_methods)
        : Decl(name, type), m_methods(methods), m_static_methods(static_methods), m_fields(fields) {
    }
    virtual ~RegionDef() override = default;

    virtual RegionTy *get_type() const override;

    const std::vector<FnDecl *, Arena<FnDecl *>> &get_methods() const;
    void add_method(FnDecl *method);
    void add_methods(std::initializer_list<FnDecl *> methods);
    void clear_methods();
    void remove_method(FnDecl *method);

    const std::vector<FnDecl *, Arena<FnDecl *>> &get_static_methods() const;
    void add_static_method(FnDecl *method);
    void add_static_methods(std::initializer_list<FnDecl *> methods);
    void clear_static_methods();
    void remove_static_method(FnDecl *method);

    const std::vector<CompositeField *, Arena<CompositeField *>> &get_fields() const;
    void add_field(CompositeField *field);
    void add_fields(std::initializer_list<CompositeField *> fields);
    void clear_fields();
    void remove_field(CompositeField *field);

    NODE_IMPL_CORE(RegionDef);
  };

  class UnionDef : public Decl {
protected:
    std::vector<FnDecl *, Arena<FnDecl *>> m_methods;
    std::vector<FnDecl *, Arena<FnDecl *>> m_static_methods;
    std::vector<CompositeField *, Arena<CompositeField *>> m_fields;

    virtual Type *infer_type_impl() const override final;

public:
    UnionDef(std::string_view name = "", UnionTy *type = nullptr,
             std::initializer_list<CompositeField *> fields = {},
             std::initializer_list<FnDecl *> methods = {},
             std::initializer_list<FnDecl *> static_methods = {})
        : Decl(name, type), m_methods(methods), m_static_methods(static_methods), m_fields(fields) {
    }
    UnionDef(std::string_view name, UnionTy *type,
             const std::vector<CompositeField *, Arena<CompositeField *>> &fields,
             const std::vector<FnDecl *, Arena<FnDecl *>> &methods,
             const std::vector<FnDecl *, Arena<FnDecl *>> &static_methods)
        : Decl(name, type), m_methods(methods), m_static_methods(static_methods), m_fields(fields) {
    }
    virtual ~UnionDef() override = default;

    virtual UnionTy *get_type() const override;

    const std::vector<FnDecl *, Arena<FnDecl *>> &get_methods() const;
    void add_method(FnDecl *method);
    void add_methods(std::initializer_list<FnDecl *> methods);
    void clear_methods();
    void remove_method(FnDecl *method);

    const std::vector<FnDecl *, Arena<FnDecl *>> &get_static_methods() const;
    void add_static_method(FnDecl *method);
    void add_static_methods(std::initializer_list<FnDecl *> methods);
    void clear_static_methods();
    void remove_static_method(FnDecl *method);

    const std::vector<CompositeField *, Arena<CompositeField *>> &get_fields() const;
    void add_field(CompositeField *field);
    void add_fields(std::initializer_list<CompositeField *> fields);
    void clear_fields();
    void remove_field(CompositeField *field);

    NODE_IMPL_CORE(RegionDef);
  };

  typedef std::pair<std::string_view, Type *> EnumItem;

  class EnumDef : public Decl {
protected:
    std::vector<EnumItem, Arena<EnumItem>> m_items;

    virtual Type *infer_type_impl() const override final;

public:
    EnumDef(std::string_view name = "", EnumTy *type = nullptr,
            std::initializer_list<EnumItem> items = {})
        : Decl(name, type), m_items(items) {}
    EnumDef(std::string_view name, EnumTy *type,
            const std::vector<EnumItem, Arena<EnumItem>> &items)
        : Decl(name, type), m_items(items) {}
    virtual ~EnumDef() override = default;

    virtual EnumTy *get_type() const override;

    const std::vector<EnumItem, Arena<EnumItem>> &get_items() const;
    void add_item(EnumItem item);
    void add_items(std::initializer_list<EnumItem> items);
    void clear_items();
    void remove_item(EnumItem item);

    NODE_IMPL_CORE(EnumDef)
  };

} // namespace libquixcc::qast

#endif // __QUIXCC_QAST_NODES_H__
