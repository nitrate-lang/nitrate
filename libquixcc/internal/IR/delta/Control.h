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

#ifndef __QUIXCC_IR_DELTA_NODES_CONTROL_H__
#define __QUIXCC_IR_DELTA_NODES_CONTROL_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <IR/delta/DeltaIR.h>
#include <IR/delta/Expr.h>
#include <IR/delta/Segment.h>
#include <IR/delta/Type.h>
#include <IR/delta/Variable.h>

namespace libquixcc::ir::delta {
class IfElse : public Value {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  IfElse(const Expr *cond, const Value *then, const Value *els)
      : cond(cond), then(then), els(els) {
    ntype = (int)NodeType::IfElse;
  }

 public:
  static const IfElse *create(const Expr *cond, const Value *then,
                              const Value *els);

  const Expr *cond;
  const Value *then;
  const Value *els;
};

class While : public Value {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  While(const Expr *cond, const Value *body) : cond(cond), body(body) {
    ntype = (int)NodeType::While;
  }

 public:
  static const While *create(const Expr *cond, const Value *body);

  const Expr *cond;
  const Value *body;
};

class Jmp : public Value {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Jmp(std::string target) : target(target) { ntype = (int)NodeType::Jmp; }

 public:
  static const Jmp *create(std::string target);

  std::string target;
};

class Label : public Value {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Label(std::string name, const Value *code) : name(name), code(code) {
    ntype = (int)NodeType::Label;
  }

 public:
  static const Label *create(std::string name, const Value *code);

  std::string name;
  const Value *code;
};

class Ret : public Value {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Ret(const Expr *value) : value(value) { ntype = (int)NodeType::Ret; }

 public:
  static const Ret *create(const Expr *value);

  const Expr *value;
};

class Call : public Expr {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Call(std::string callee, std::vector<const Expr *> args, const FType *ftype)
      : callee(callee), args(args), ftype(ftype) {
    ntype = (int)NodeType::Call;
  }

 public:
  static const Call *create(std::string callee, std::vector<const Expr *> args,
                            const FType *ftype);
  const Type *infer() const override;

  std::string callee;
  std::vector<const Expr *> args;
  const FType *ftype;
};

class PtrCall : public Expr {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  PtrCall(const Expr *callee, std::vector<const Expr *> args)
      : callee(callee), args(args) {
    ntype = (int)NodeType::PtrCall;
  }

 public:
  static const PtrCall *create(const Expr *callee,
                               std::vector<const Expr *> args);
  const Type *infer() const override;

  const Expr *callee;
  std::vector<const Expr *> args;
};

class Halt : public Value {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Halt() { ntype = (int)NodeType::Halt; }

 public:
  static const Halt *create();
};

class Break : public Value {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Break() { ntype = (int)NodeType::Break; }

 public:
  static const Break *create();
};

class Continue : public Value {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Continue() { ntype = (int)NodeType::Continue; }

 public:
  static const Continue *create();
};

class Case : public Value {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Case(const Number *value, const Value *code) : value(value), code(code) {
    ntype = (int)NodeType::Case;
  }

 public:
  static const Case *create(const Number *value, const Value *code);

  const Number *value;
  const Value *code;
};

class Switch : public Value {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Switch(const Expr *cond, std::vector<const Case *> cases, const Value *def)
      : cond(cond), cases(cases), def(def) {
    ntype = (int)NodeType::Switch;
  }

 public:
  static const Switch *create(const Expr *cond, std::vector<const Case *> cases,
                              const Value *def);

  const Expr *cond;
  std::vector<const Case *> cases;
  const Value *def;
};

}  // namespace libquixcc::ir::delta

#endif  // __QUIXCC_IR_DELTA_NODES_CONTROL_H__