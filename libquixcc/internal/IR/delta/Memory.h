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

#ifndef __QUIXCC_IR_DELTA_NODES_MEMORY_H__
#define __QUIXCC_IR_DELTA_NODES_MEMORY_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <IR/delta/DeltaIR.h>
#include <IR/delta/Expr.h>
#include <IR/delta/Type.h>

namespace libquixcc::ir::delta {
class Assign : public Expr {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Assign(const Expr *var, const Expr *value, uint64_t rank)
      : rank(rank), var(var), value(value) {
    ntype = (int)NodeType::Assign;
  }

 public:
  static const Assign *create(const Expr *var, const Expr *value,
                              uint64_t rank = 0);
  const Type *infer() const override;

  uint64_t rank; /* How many levels of dereferencing are to be done */
  const Expr *var;
  const Expr *value;
};

class PostInc : public Expr {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  PostInc(const Expr *var) : var(var) { ntype = (int)NodeType::PostInc; }

 public:
  static const PostInc *create(const Expr *var);
  const Type *infer() const override;

  const Expr *var;
};

class PostDec : public Expr {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  PostDec(const Expr *var) : var(var) { ntype = (int)NodeType::PostDec; }

 public:
  static const PostDec *create(const Expr *var);
  const Type *infer() const override;

  const Expr *var;
};

class AddressOf : public Expr {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  AddressOf(const Expr *lhs) : lhs(lhs) { ntype = (int)NodeType::AddressOf; }

 public:
  static const AddressOf *create(const Expr *lhs);
  const Type *infer() const override;

  const Expr *lhs;
};

class Deref : public Expr {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Deref(const Expr *lhs) : lhs(lhs) { ntype = (int)NodeType::Deref; }

 public:
  static const Deref *create(const Expr *lhs);
  const Type *infer() const override;

  const Expr *lhs;
};

class Member : public Expr {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Member(const Expr *lhs, size_t field, const Type *field_type)
      : field(field), lhs(lhs), field_type(field_type) {
    ntype = (int)NodeType::Member;
  }

 public:
  static const Member *create(const Expr *lhs, size_t field,
                              const Type *field_type);
  const Type *infer() const override;

  size_t field;
  const Expr *lhs;
  const Type *field_type;
};

class Index : public Expr {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Index(const Expr *expr, const Expr *index, const Type *type)
      : expr(expr), index(index), type(type) {
    ntype = (int)NodeType::Index;
  }

 public:
  static const Index *create(const Expr *expr, const Expr *index,
                             const Type *type);
  const Type *infer() const override;

  const Expr *expr;
  const Expr *index;
  const Type *type;
};
}  // namespace libquixcc::ir::delta

#endif  // __QUIXCC_IR_DELTA_NODES_MEMORY_H__