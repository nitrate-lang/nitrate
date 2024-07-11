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
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <tuple>
#include <vector>

#define NODE_IMPL_CORE()                                               \
 protected:                                                            \
  virtual bool verify_impl(std::ostream &os) const override;           \
                                                                       \
 protected:                                                            \
  virtual void canonicalize_impl() override;                           \
                                                                       \
 protected:                                                            \
  virtual void dump_impl(std::ostream &os, bool debug) const override; \
                                                                       \
 public:                                                               \
  uint32_t size_of() const override;                                   \
                                                                       \
 public:                                                               \
  quixcc_ast_ntype_t type_of() const override;                         \
                                                                       \
 public:                                                               \
  const char *type_name() const override

class quixcc_ast_node_t : public libquixcc::core::IDumpable {
 public:
  quixcc_ast_node_t() = default;
  virtual ~quixcc_ast_node_t() = default;
};

namespace libquixcc::qast {
enum class ExprOp {
  UNKNOWN,
};

class Node : public quixcc_ast_node_t {
 protected:
  virtual bool verify_impl(std::ostream &os) const = 0;
  virtual void canonicalize_impl() = 0;
  virtual void dump_impl(std::ostream &os, bool debug) const override = 0;

 public:
  Node() = default;
  virtual ~Node() override = default;

  virtual uint32_t size_of() const = 0;
  virtual quixcc_ast_ntype_t type_of() const = 0;
  virtual const char *type_name() const = 0;

  size_t children_count(bool recursive = true) const;
  bool inherits_from(quixcc_ast_ntype_t type) const;

  bool is_type() const;
  bool is_stmt() const;
  bool is_decl() const;
  bool is_expr() const;
  bool is_const_expr() const;

  template <typename T>
  const T *as() const {
#if !defined(NDEBUG)
    auto p = dynamic_cast<const T *>(this);

    if (!p) {
      const char *this_str = typeid(*this).name();
      const char *other_str = typeid(T).name();

      quixcc_panicf(
          "quixcc_ast_node_t::as(const %s *this): Invalid cast from `%s` to "
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

      quixcc_panicf(
          "quixcc_ast_node_t::as(%s *this): Invalid cast from `%s` to `%s`.",
          this_str, this_str, other_str);
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

  bool is(const quixcc_ast_ntype_t type) const;
  bool verify(std::ostream &os = std::cerr) const;
  void canonicalize();
  void dump(std::ostream &os, bool debug = false) const;
};

class Stmt : public Node {
 public:
  Stmt() = default;
  virtual ~Stmt() override = default;
};

class Type : public Node {
 protected:
  virtual size_t infer_size_bits_impl() const = 0;

 public:
  Type() = default;
  virtual ~Type() override = default;

  size_t infer_size_bits() const;
  size_t infer_size() const;

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
  bool is_string() const;
};

class Decl : public Node {
 protected:
  std::set<std::string> m_tags;

  virtual std::optional<std::string> get_name_impl() const = 0;

 public:
  Decl() = default;
  virtual ~Decl() override = default;

  bool decl_has_name() const;
  std::string get_decl_name() const;

  const std::set<std::string> &get_tags() const;
  void add_tag(std::string_view tag);
  void add_tags(std::initializer_list<std::string_view> tags);
  void clear_tags();
  void remove_tag(std::string_view tag);

  bool has_type() const;
  virtual Type *infer_type() const = 0;
};

class Expr : public Node {
 protected:
  Type *m_type;

 public:
  Expr() : m_type(nullptr) {}
  virtual ~Expr() override = default;

  virtual Type *infer_type() const = 0;
  virtual bool is_const() const = 0;
  virtual bool is_stochastic() const = 0;
};

class ConstExpr : public Expr {
  template <typename>
  struct is_std_vector : std::false_type {};

  template <typename T, typename A>
  struct is_std_vector<std::vector<T, A>> : std::true_type {};

  template <typename T>
  struct is_std_map : std::false_type {};

  template <typename K, typename V, typename C, typename A>
  struct is_std_map<std::map<K, V, C, A>> : std::true_type {};

  template <typename T>
  struct is_std_set : std::false_type {};

  template <typename K, typename C, typename A>
  struct is_std_set<std::set<K, C, A>> : std::true_type {};

  template <typename>
  struct is_std_tuple : std::false_type {};

  template <typename... T>
  struct is_std_tuple<std::tuple<T...>> : std::true_type {};

  template <typename T>
  T do_eval() const {
    /// TODO: Implement this.
    quixcc_panicf("ConstExpr::do_eval<T>(): Not implemented.");
  }

 public:
  ConstExpr() = default;
  virtual ~ConstExpr() override = default;

  virtual bool is_const() const override final;
  virtual bool is_stochastic() const override final;

  template <typename T>
  T eval_as() const {
    if constexpr (std::is_same_v<T, bool>) {
      assert(infer_type()->is_bool());
      return do_eval<bool>();
    } else if constexpr (std::is_integral_v<T>) {
      assert(infer_type()->is_integral());
      return do_eval<T>();
    } else if constexpr (std::is_floating_point_v<T>) {
      assert(infer_type()->is_floating_point());
      return do_eval<T>();
    } else if constexpr (std::is_same_v<T, std::string>) {
      assert(infer_type()->is_string());
      return do_eval<std::string>();
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
      quixcc_panicf("ConstExpr::eval_as<T>(): Invalid type `%s`.",
                    typeid(T).name());
      __builtin_unreachable();
    }
  }
};

class LitExpr : public ConstExpr {
 public:
  LitExpr() = default;
  virtual ~LitExpr() override = default;
};

class FlowStmt : public Stmt {
 public:
  FlowStmt() = default;
  virtual ~FlowStmt() override = default;
};

class DeclStmt : public Stmt {
 public:
  DeclStmt() = default;
  virtual ~DeclStmt() override = default;
};

class TypeBuiltin : public Type {
 public:
  TypeBuiltin() = default;
  virtual ~TypeBuiltin() override = default;
};

class TypeComplex : public Type {
 public:
  TypeComplex() = default;
  virtual ~TypeComplex() override = default;
};

class TypeComposite : public Type {
 public:
  TypeComposite() = default;
  virtual ~TypeComposite() override = default;
};

class UnresolvedType : public Type {
  std::string m_name;

  virtual size_t infer_size_bits_impl() const override final;

 public:
  UnresolvedType() = default;
  virtual ~UnresolvedType() override = default;

  NODE_IMPL_CORE();
};

class I1 : public TypeBuiltin {
  virtual size_t infer_size_bits_impl() const override final;

 public:
  I1() = default;
  virtual ~I1() override = default;

  NODE_IMPL_CORE();
};

class I8 : public TypeBuiltin {
  virtual size_t infer_size_bits_impl() const override final;

 public:
  I8() = default;
  virtual ~I8() override = default;

  NODE_IMPL_CORE();
};

class I16 : public TypeBuiltin {
  virtual size_t infer_size_bits_impl() const override final;

 public:
  I16() = default;
  virtual ~I16() override = default;

  NODE_IMPL_CORE();
};

class I32 : public TypeBuiltin {
  virtual size_t infer_size_bits_impl() const override final;

 public:
  I32() = default;
  virtual ~I32() override = default;

  NODE_IMPL_CORE();
};

class I64 : public TypeBuiltin {
  virtual size_t infer_size_bits_impl() const override final;

 public:
  I64() = default;
  virtual ~I64() override = default;

  NODE_IMPL_CORE();
};

class I128 : public TypeBuiltin {
  virtual size_t infer_size_bits_impl() const override final;

 public:
  I128() = default;
  virtual ~I128() override = default;

  NODE_IMPL_CORE();
};

class U8 : public TypeBuiltin {
  virtual size_t infer_size_bits_impl() const override final;

 public:
  U8() = default;
  virtual ~U8() override = default;

  NODE_IMPL_CORE();
};

class U16 : public TypeBuiltin {
  virtual size_t infer_size_bits_impl() const override final;

 public:
  U16() = default;
  virtual ~U16() override = default;

  NODE_IMPL_CORE();
};

class U32 : public TypeBuiltin {
  virtual size_t infer_size_bits_impl() const override final;

 public:
  U32() = default;
  virtual ~U32() override = default;

  NODE_IMPL_CORE();
};

class U64 : public TypeBuiltin {
  virtual size_t infer_size_bits_impl() const override final;

 public:
  U64() = default;
  virtual ~U64() override = default;

  NODE_IMPL_CORE();
};

class U128 : public TypeBuiltin {
  virtual size_t infer_size_bits_impl() const override final;

 public:
  U128() = default;
  virtual ~U128() override = default;

  NODE_IMPL_CORE();
};

class F32 : public TypeBuiltin {
  virtual size_t infer_size_bits_impl() const override final;

 public:
  F32() = default;
  virtual ~F32() override = default;

  NODE_IMPL_CORE();
};

class F64 : public TypeBuiltin {
  virtual size_t infer_size_bits_impl() const override final;

 public:
  F64() = default;
  virtual ~F64() override = default;

  NODE_IMPL_CORE();
};

class Void : public TypeBuiltin {
  virtual size_t infer_size_bits_impl() const override final;

 public:
  Void() = default;
  virtual ~Void() override = default;

  NODE_IMPL_CORE();
};

class StringTy : public TypeBuiltin {
  virtual size_t infer_size_bits_impl() const override final;

 public:
  StringTy() = default;
  virtual ~StringTy() override = default;

  NODE_IMPL_CORE();
};

class Ptr : public TypeComplex {
  Type *m_target;
  bool m_is_volatile;

  virtual size_t infer_size_bits_impl() const override final;

 public:
  Ptr(Type *target, bool is_volatile = false)
      : m_target(target), m_is_volatile(is_volatile) {}
  virtual ~Ptr() override = default;

  Type *get_target() const;
  void set_target(Type *target);

  bool is_volatile() const;
  void set_volatile(bool is_volatile);

  NODE_IMPL_CORE();
};

class OpaqueTy : public TypeComplex {
  std::string m_name;

  virtual size_t infer_size_bits_impl() const override final;

 public:
  OpaqueTy(std::string_view name = "") : m_name(name) {}
  virtual ~OpaqueTy() override = default;

  const std::string &get_name() const;
  void set_name(std::string_view name);

  NODE_IMPL_CORE();
};

/*
TYPE_ENUM
TYPE_STRUCT
TYPE_GROUP
TYPE_REGION
TYPE_UNION
TYPE_FUNCTION
TYPE_USER
*/

class VectorTy : public TypeComposite {
  Type *m_element;

  virtual size_t infer_size_bits_impl() const override final;

 public:
  VectorTy(Type *element = nullptr) : m_element(element) {}
  virtual ~VectorTy() override = default;

  Type *get_element() const;
  void set_element(Type *element);

  NODE_IMPL_CORE();
};

class SetTy : public TypeComposite {
  Type *m_element;

  virtual size_t infer_size_bits_impl() const override final;

 public:
  SetTy(Type *element = nullptr) : m_element(element) {}
  virtual ~SetTy() override = default;

  Type *get_element() const;
  void set_element(Type *element);

  NODE_IMPL_CORE();
};

class MapTy : public TypeComposite {
  Type *m_key;
  Type *m_value;

  virtual size_t infer_size_bits_impl() const override final;

 public:
  MapTy(Type *key = nullptr, Type *value = nullptr)
      : m_key(key), m_value(value) {}
  virtual ~MapTy() override = default;

  Type *get_key() const;
  void set_key(Type *key);

  Type *get_value() const;
  void set_value(Type *value);

  NODE_IMPL_CORE();
};

class TupleTy : public TypeComposite {
  std::vector<Type *> m_elements;

  virtual size_t infer_size_bits_impl() const override final;

 public:
  TupleTy() = default;
  virtual ~TupleTy() override = default;

  const std::vector<Type *> &get_elements() const;
  void add_element(Type *element);
  void add_elements(std::initializer_list<Type *> elements);
  void clear_elements();
  void remove_element(Type *element);

  NODE_IMPL_CORE();
};

class ResultTy : public TypeComposite {
  Type *m_element;

  virtual size_t infer_size_bits_impl() const override final;

 public:
  ResultTy(Type *element = nullptr) : m_element(element) {}
  virtual ~ResultTy() override = default;

  Type *get_element() const;
  void set_element(Type *element);

  NODE_IMPL_CORE();
};

class ArrayTy : public TypeComposite {
  Type *m_element;
  ConstExpr *m_size;

  virtual size_t infer_size_bits_impl() const override final;

 public:
  ArrayTy(Type *element = nullptr, ConstExpr *size = nullptr)
      : m_element(element), m_size(size) {}
  virtual ~ArrayTy() override = default;

  Type *get_element() const;
  void set_element(Type *element);

  ConstExpr *get_size() const;
  void set_size(ConstExpr *size);

  NODE_IMPL_CORE();
};

class UnaryExpr : public Expr {
 protected:
  Expr *m_rhs;
  ExprOp m_op;

 public:
  UnaryExpr() : m_rhs(nullptr), m_op(ExprOp::UNKNOWN) {}
  virtual ~UnaryExpr() override = default;

  virtual Type *infer_type() const override final;

  Expr *get_rhs() const;
  void set_rhs(Expr *rhs);

  ExprOp get_op() const;
  void set_op(ExprOp op);

  NODE_IMPL_CORE();
};

class BinExpr : public Expr {
 protected:
  Expr *m_lhs;
  Expr *m_rhs;
  ExprOp m_op;

 public:
  BinExpr() : m_lhs(nullptr), m_rhs(nullptr), m_op(ExprOp::UNKNOWN) {}
  virtual ~BinExpr() override = default;

  virtual Type *infer_type() const override final;

  Expr *get_lhs() const;
  void set_lhs(Expr *lhs);

  Expr *get_rhs() const;
  void set_rhs(Expr *rhs);

  ExprOp get_op() const;
  void set_op(ExprOp op);

  NODE_IMPL_CORE();
};

class IntLitExpr : public LitExpr {
  std::string m_value;
  uint16_t m_value_log2;

  void calculate_log2();

 protected:
  virtual bool verify_impl(std::ostream &os) const override final;
  virtual void canonicalize_impl() override final;
  virtual void dump_impl(std::ostream &os, bool debug) const override final;

 public:
  IntLitExpr(std::string_view value = "") : m_value(value), m_value_log2(0) {}
  IntLitExpr(auto value) {
    if constexpr (std::is_integral_v<decltype(value)>) {
      m_value = std::to_string(value);
    } else {
      m_value = value;
    }
  }
  virtual ~IntLitExpr() override = default;

  virtual uint32_t size_of() const override final;
  virtual quixcc_ast_ntype_t type_of() const override final;
  virtual const char *type_name() const override final;

  virtual Type *infer_type() const override final;

  void set_value(std::string_view value);
  const std::string &get_value() const;

  uint16_t get_log2();
};
}  // namespace libquixcc::qast

#endif  // __QUIXCC_QAST_NODES_H__
