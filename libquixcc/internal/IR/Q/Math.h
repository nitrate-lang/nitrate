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

#ifndef __QUIXCC_IR_Q_NODES_MATH_H__
#define __QUIXCC_IR_Q_NODES_MATH_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <IR/Q/Expr.h>
#include <IR/Q/QIR.h>

namespace libquixcc::ir::q {
class Add : public Expr {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Add(Expr *lhs, Expr *rhs) : lhs(lhs), rhs(rhs) { ntype = (int)QType::Add; }

 public:
  static Add *create(Expr *lhs, Expr *rhs);
  Type *infer() const override;

  Expr *lhs;
  Expr *rhs;
};

class Sub : public Expr {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Sub(Expr *lhs, Expr *rhs) : lhs(lhs), rhs(rhs) { ntype = (int)QType::Sub; }

 public:
  static Sub *create(Expr *lhs, Expr *rhs);
  Type *infer() const override;

  Expr *lhs;
  Expr *rhs;
};

class Mul : public Expr {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Mul(Expr *lhs, Expr *rhs) : lhs(lhs), rhs(rhs) { ntype = (int)QType::Mul; }

 public:
  static Mul *create(Expr *lhs, Expr *rhs);
  Type *infer() const override;

  Expr *lhs;
  Expr *rhs;
};

class Div : public Expr {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Div(Expr *lhs, Expr *rhs) : lhs(lhs), rhs(rhs) { ntype = (int)QType::Div; }

 public:
  static Div *create(Expr *lhs, Expr *rhs);
  Type *infer() const override;

  Expr *lhs;
  Expr *rhs;
};

class Mod : public Expr {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Mod(Expr *lhs, Expr *rhs) : lhs(lhs), rhs(rhs) { ntype = (int)QType::Mod; }

 public:
  static Mod *create(Expr *lhs, Expr *rhs);
  Type *infer() const override;

  Expr *lhs;
  Expr *rhs;
};

class BitAnd : public Expr {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  BitAnd(Expr *lhs, Expr *rhs) : lhs(lhs), rhs(rhs) {
    ntype = (int)QType::BitAnd;
  }

 public:
  static BitAnd *create(Expr *lhs, Expr *rhs);
  Type *infer() const override;

  Expr *lhs;
  Expr *rhs;
};

class BitOr : public Expr {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  BitOr(Expr *lhs, Expr *rhs) : lhs(lhs), rhs(rhs) {
    ntype = (int)QType::BitOr;
  }

 public:
  static BitOr *create(Expr *lhs, Expr *rhs);
  Type *infer() const override;

  Expr *lhs;
  Expr *rhs;
};

class BitXor : public Expr {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  BitXor(Expr *lhs, Expr *rhs) : lhs(lhs), rhs(rhs) {
    ntype = (int)QType::BitXor;
  }

 public:
  static BitXor *create(Expr *lhs, Expr *rhs);
  Type *infer() const override;

  Expr *lhs;
  Expr *rhs;
};

class BitNot : public Expr {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  BitNot(Expr *operand) : operand(operand) { ntype = (int)QType::BitNot; }

 public:
  static BitNot *create(Expr *operand);
  Type *infer() const override;

  Expr *operand;
};

class Shl : public Expr {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Shl(Expr *lhs, Expr *rhs) : lhs(lhs), rhs(rhs) { ntype = (int)QType::Shl; }

 public:
  static Shl *create(Expr *lhs, Expr *rhs);
  Type *infer() const override;

  Expr *lhs;
  Expr *rhs;
};

class Shr : public Expr {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Shr(Expr *lhs, Expr *rhs) : lhs(lhs), rhs(rhs) { ntype = (int)QType::Shr; }

 public:
  static Shr *create(Expr *lhs, Expr *rhs);
  Type *infer() const override;

  Expr *lhs;
  Expr *rhs;
};

class Rotl : public Expr {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Rotl(Expr *lhs, Expr *rhs) : lhs(lhs), rhs(rhs) {
    ntype = (int)QType::Rotl;
  }

 public:
  static Rotl *create(Expr *lhs, Expr *rhs);
  Type *infer() const override;

  Expr *lhs;
  Expr *rhs;
};

class Rotr : public Expr {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Rotr(Expr *lhs, Expr *rhs) : lhs(lhs), rhs(rhs) {
    ntype = (int)QType::Rotr;
  }

 public:
  static Rotr *create(Expr *lhs, Expr *rhs);
  Type *infer() const override;

  Expr *lhs;
  Expr *rhs;
};

class Eq : public Expr {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Eq(Expr *lhs, Expr *rhs) : lhs(lhs), rhs(rhs) { ntype = (int)QType::Eq; }

 public:
  static Eq *create(Expr *lhs, Expr *rhs);
  Type *infer() const override;

  Expr *lhs;
  Expr *rhs;
};

class Ne : public Expr {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Ne(Expr *lhs, Expr *rhs) : lhs(lhs), rhs(rhs) { ntype = (int)QType::Ne; }

 public:
  static Ne *create(Expr *lhs, Expr *rhs);
  Type *infer() const override;

  Expr *lhs;
  Expr *rhs;
};

class Lt : public Expr {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Lt(Expr *lhs, Expr *rhs) : lhs(lhs), rhs(rhs) { ntype = (int)QType::Lt; }

 public:
  static Lt *create(Expr *lhs, Expr *rhs);
  Type *infer() const override;

  Expr *lhs;
  Expr *rhs;
};

class Gt : public Expr {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Gt(Expr *lhs, Expr *rhs) : lhs(lhs), rhs(rhs) { ntype = (int)QType::Gt; }

 public:
  static Gt *create(Expr *lhs, Expr *rhs);
  Type *infer() const override;

  Expr *lhs;
  Expr *rhs;
};

class Le : public Expr {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Le(Expr *lhs, Expr *rhs) : lhs(lhs), rhs(rhs) { ntype = (int)QType::Le; }

 public:
  static Le *create(Expr *lhs, Expr *rhs);
  Type *infer() const override;

  Expr *lhs;
  Expr *rhs;
};

class Ge : public Expr {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Ge(Expr *lhs, Expr *rhs) : lhs(lhs), rhs(rhs) { ntype = (int)QType::Ge; }

 public:
  static Ge *create(Expr *lhs, Expr *rhs);
  Type *infer() const override;

  Expr *lhs;
  Expr *rhs;
};

class And : public Expr {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  And(Expr *lhs, Expr *rhs) : lhs(lhs), rhs(rhs) { ntype = (int)QType::And; }

 public:
  static And *create(Expr *lhs, Expr *rhs);
  Type *infer() const override;

  Expr *lhs;
  Expr *rhs;
};

class Or : public Expr {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Or(Expr *lhs, Expr *rhs) : lhs(lhs), rhs(rhs) { ntype = (int)QType::Or; }

 public:
  static Or *create(Expr *lhs, Expr *rhs);
  Type *infer() const override;

  Expr *lhs;
  Expr *rhs;
};

class Not : public Expr {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Not(Expr *operand) : operand(operand) { ntype = (int)QType::Not; }

 public:
  static Not *create(Expr *operand);
  Type *infer() const override;

  Expr *operand;
};

class Xor : public Expr {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Xor(Expr *lhs, Expr *rhs) : lhs(lhs), rhs(rhs) { ntype = (int)QType::Xor; }

 public:
  static Xor *create(Expr *lhs, Expr *rhs);
  Type *infer() const override;

  Expr *lhs;
  Expr *rhs;
};
}  // namespace libquixcc::ir::q

#endif  // __QUIXCC_IR_Q_NODES_MATH_H__