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

#ifndef __QUIXCC_IR_Q_NODES_MEMORY_H__
#define __QUIXCC_IR_Q_NODES_MEMORY_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <IR/Q/Expr.h>
#include <IR/Q/QIR.h>

namespace libquixcc::ir::q {
class Assign : public Expr {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Assign(Expr *lhs, Expr *rhs) : lhs(lhs), rhs(rhs) {
    ntype = (int)NodeType::Assign;
  }

 public:
  static Assign *create(Expr *lhs, Expr *rhs);
  Type *infer() const override;

  Expr *lhs;
  Expr *rhs;
};

class PostInc : public Expr {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  PostInc(Expr *lhs) : lhs(lhs) { ntype = (int)NodeType::PostInc; }

 public:
  static PostInc *create(Expr *lhs);
  Type *infer() const override;

  Expr *lhs;
};

class PostDec : public Expr {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  PostDec(Expr *lhs) : lhs(lhs) { ntype = (int)NodeType::PostDec; }

 public:
  static PostDec *create(Expr *lhs);
  Type *infer() const override;

  Expr *lhs;
};

class AddressOf : public Expr {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  AddressOf(Expr *lhs) : lhs(lhs) { ntype = (int)NodeType::AddressOf; }

 public:
  static AddressOf *create(Expr *lhs);
  Type *infer() const override;

  Expr *lhs;
};

class Deref : public Expr {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Deref(Expr *lhs) : lhs(lhs) { ntype = (int)NodeType::Deref; }

 public:
  static Deref *create(Expr *lhs);
  Type *infer() const override;

  Expr *lhs;
};

class Member : public Expr {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Member(Value *lhs, size_t field, Type *field_type)
      : field(field), lhs(lhs), field_type(field_type) {
    ntype = (int)NodeType::Member;
  }

 public:
  static Member *create(Value *lhs, size_t field, Type *field_type);
  Type *infer() const override;

  size_t field;
  Value *lhs;
  Type *field_type;
};

class Index : public Expr {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Index(Value *lhs, Expr *index, Type *type)
      : lhs(lhs), index(index), type(type) {
    ntype = (int)NodeType::Index;
  }

 public:
  static Index *create(Value *lhs, Expr *index, Type *type);
  Type *infer() const override;

  Value *lhs;
  Expr *index;
  Type *type;
};
}  // namespace libquixcc::ir::q

#endif  // __QUIXCC_IR_Q_NODES_MEMORY_H__