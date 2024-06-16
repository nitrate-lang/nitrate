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

#ifndef __QUIXCC_IR_Q_NODES_TYPE_H__
#define __QUIXCC_IR_Q_NODES_TYPE_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <IR/Q/QIR.h>

#include <cmath>

namespace libquixcc::ir::q {
class Type : public Value {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override = 0;
  boost::uuids::uuid hash_impl() const override = 0;
  bool verify_impl() const override = 0;

 public:
  virtual size_t bitcount() const = 0;
  size_t size() const;
  bool is_ptr() const;
  bool is_integer() const;
  bool is_float() const;
  bool is_void() const;
  bool is_signed() const;
  bool is_unsigned() const;
  bool is_primitive() const;
};

class I1 : public Type {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  I1() { ntype = (int)NodeType::I1; }

 public:
  static I1 *create();
  size_t bitcount() const override { return 1; }
};

class I8 : public Type {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  I8() { ntype = (int)NodeType::I8; }

 public:
  static I8 *create();
  size_t bitcount() const override { return 8; }
};

class I16 : public Type {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  I16() { ntype = (int)NodeType::I16; }

 public:
  static I16 *create();
  size_t bitcount() const override { return 16; }
};

class I32 : public Type {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  I32() { ntype = (int)NodeType::I32; }

 public:
  static I32 *create();
  size_t bitcount() const override { return 32; }
};

class I64 : public Type {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  I64() { ntype = (int)NodeType::I64; }

 public:
  static I64 *create();
  size_t bitcount() const override { return 64; }
};

class I128 : public Type {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  I128() { ntype = (int)NodeType::I128; }

 public:
  static I128 *create();
  size_t bitcount() const override { return 128; }
};

class U8 : public Type {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  U8() { ntype = (int)NodeType::U8; }

 public:
  static U8 *create();
  size_t bitcount() const override { return 8; }
};

class U16 : public Type {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  U16() { ntype = (int)NodeType::U16; }

 public:
  static U16 *create();
  size_t bitcount() const override { return 16; }
};

class U32 : public Type {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  U32() { ntype = (int)NodeType::U32; }

 public:
  static U32 *create();
  size_t bitcount() const override { return 32; }
};

class U64 : public Type {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  U64() { ntype = (int)NodeType::U64; }

 public:
  static U64 *create();
  size_t bitcount() const override { return 64; }
};

class U128 : public Type {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  U128() { ntype = (int)NodeType::U128; }

 public:
  static U128 *create();
  size_t bitcount() const override { return 128; }
};

class F32 : public Type {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  F32() { ntype = (int)NodeType::F32; }

 public:
  static F32 *create();
  size_t bitcount() const override { return 32; }
};

class F64 : public Type {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  F64() { ntype = (int)NodeType::F64; }

 public:
  static F64 *create();
  size_t bitcount() const override { return 64; }
};

class Void : public Type {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Void() { ntype = (int)NodeType::Void; }

 public:
  static Void *create();
  size_t bitcount() const override { return 0; }
};

class Ptr : public Type {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Ptr(Type *type) : type(type) { ntype = (int)NodeType::Ptr; }

 public:
  static Ptr *create(Type *type);
  size_t bitcount() const override {
    /// TODO: This is a temporary solution. We need to find a better way to
    /// handle this.
    return 64;
  }

  Type *type;
};

class Array : public Type {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Array(Type *type, uint64_t size) : type(type), size(size) {
    ntype = (int)NodeType::Array;
  }

 public:
  static Array *create(Type *type, uint64_t size);
  size_t bitcount() const override { return type->bitcount() * size; }

  Type *type;
  uint64_t size;
};

class Vector : public Type {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Vector(Type *type) : type(type) { ntype = (int)NodeType::Vector; }

 public:
  static Vector *create(Type *type);
  size_t bitcount() const override {
    throw std::runtime_error("Cannot get bitcount of vector type");
  }

  Type *type;
};

class FType : public Type {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  FType(std::vector<Type *> params, Type *ret, bool variadic, bool pure,
        bool thread_safe, bool foreign, bool nothrow)
      : params(params),
        ret(ret),
        m_variadic(variadic),
        m_pure(pure),
        m_thread_safe(thread_safe),
        m_foreign(foreign),
        m_noexcept(nothrow) {
    ntype = (int)NodeType::FType;
  }

 public:
  static FType *create(std::vector<Type *> params, Type *ret,
                       bool variadic = false, bool pure = false,
                       bool thread_safe = false, bool foreign = false,
                       bool nothrow = false);
  size_t bitcount() const override {
    return Ptr::create(Void::create())->bitcount();
  }

  std::vector<Type *> params;
  Type *ret;
  bool m_variadic;
  bool m_pure;
  bool m_thread_safe;
  bool m_foreign;
  bool m_noexcept;
};

class Region : public Type {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Region(std::string name, std::vector<Type *> fields)
      : name(name), fields(fields) {
    ntype = (int)NodeType::Region;
  }

 public:
  static Region *create(std::string name, std::vector<Type *> fields);
  size_t bitcount() const override {
    throw std::runtime_error("Cannot get bitcount of region type");
  }

  std::string name;
  std::vector<Type *> fields;
};

class Group : public Type {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Group(std::string name, std::vector<Type *> fields)
      : name(name), fields(fields) {
    ntype = (int)NodeType::Group;
  }

 public:
  static Group *create(std::string name, std::vector<Type *> fields);
  size_t bitcount() const override {
    throw std::runtime_error("Cannot get bitcount of group type");
  }

  std::string name;
  std::vector<Type *> fields;
};

class Union : public Type {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Union(std::string name, std::vector<Type *> fields)
      : name(name), fields(fields) {
    ntype = (int)NodeType::Union;
  }

 public:
  static Union *create(std::string name, std::vector<Type *> fields);
  size_t bitcount() const override {
    throw std::runtime_error("Cannot get bitcount of union type");
  }

  std::string name;
  std::vector<Type *> fields;
};

class Opaque : public Type {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Opaque(std::string name) : name(name) { ntype = (int)NodeType::Opaque; }

 public:
  static Opaque *create(std::string name);
  size_t bitcount() const override {
    throw std::runtime_error("Cannot get bitcount of opaque type");
  }

  std::string name;
};
}  // namespace libquixcc::ir::q

#endif  // __QUIXCC_IR_Q_NODES_TYPE_H__