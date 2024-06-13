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

  Assign(const Expr *lhs, const Expr *rhs) : lhs(lhs), rhs(rhs) {
    ntype = (int)NodeType::Assign;
  }

 public:
  static const Assign *create(const Expr *lhs, const Expr *rhs);
  const Type *infer() const override;

  const Expr *lhs;
  const Expr *rhs;
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

  Member(const Value *lhs, size_t field, const Type *field_type)
      : field(field), lhs(lhs), field_type(field_type) {
    ntype = (int)NodeType::Member;
  }

 public:
  static const Member *create(const Value *lhs, size_t field,
                              const Type *field_type);
  const Type *infer() const override;

  size_t field;
  const Value *lhs;
  const Type *field_type;
};

class Index : public Expr {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Index(const Value *lhs, const Expr *index, const Type *type)
      : lhs(lhs), index(index), type(type) {
    ntype = (int)NodeType::Index;
  }

 public:
  static const Index *create(const Value *lhs, const Expr *index,
                             const Type *type);
  const Type *infer() const override;

  const Value *lhs;
  const Expr *index;
  const Type *type;
};
}  // namespace libquixcc::ir::q

#endif  // __QUIXCC_IR_Q_NODES_MEMORY_H__