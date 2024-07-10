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

#ifndef __QUIXCC_IR_Q_NODES_CALL_H__
#define __QUIXCC_IR_Q_NODES_CALL_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <quixcc/IR/Q/Expr.h>
#include <quixcc/IR/Q/Function.h>
#include <quixcc/IR/Q/QIR.h>
#include <quixcc/IR/Q/Variable.h>

namespace libquixcc::ir::q {
class Call : public Expr {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Call(Global *func, std::vector<Expr *> args) : func(func), args(args) {
    ntype = (int)QType::Call;
  }

 public:
  static Call *create(Global *func, std::vector<Expr *> args);
  Type *infer() const override;

  Global *func;
  std::vector<Expr *> args;
};

class CallIndirect : public Expr {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  CallIndirect(Segment *exprfunc, std::vector<Expr *> args)
      : exprfunc(exprfunc), args(args) {
    ntype = (int)QType::CallIndirect;
  }

 public:
  static CallIndirect *create(Segment *exprfunc, std::vector<Expr *> args);
  Type *infer() const override;

  Segment *exprfunc;
  std::vector<Expr *> args;
};

enum class QIntrinsic {
  Malloc,
  Free,

  ToString,
};

class IntrinsicCall : public Expr {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  IntrinsicCall(QIntrinsic name, std::vector<Expr *> args)
      : name(name), args(args) {
    ntype = (int)QType::IntrinsicCall;
  }

 public:
  static IntrinsicCall *create(QIntrinsic name, std::vector<Expr *> args);
  Type *infer() const override;

  QIntrinsic name;
  std::vector<Expr *> args;
};

}  // namespace libquixcc::ir::q

#endif  // __QUIXCC_IR_Q_NODES_CALL_H__