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

#ifndef __QUIXCC_IR_Q_NODES_CONTROL_H__
#define __QUIXCC_IR_Q_NODES_CONTROL_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <IR/Q/Expr.h>
#include <IR/Q/QIR.h>

namespace libquixcc::ir::q {
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

class For : public Value {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  For(const Expr *init, const Expr *cond, const Expr *step, const Value *body)
      : init(init), cond(cond), step(step), body(body) {
    ntype = (int)NodeType::For;
  }

 public:
  static const For *create(const Expr *init, const Expr *cond, const Expr *step,
                           const Value *body);

  const Expr *init;
  const Expr *cond;
  const Expr *step;
  const Value *body;
};

class Loop : public Value {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Loop(const Value *body) : body(body) { ntype = (int)NodeType::Loop; }

 public:
  static const Loop *create(const Value *body);

  const Value *body;
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

class Throw : public Value {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Throw(const Expr *value) : value(value) { ntype = (int)NodeType::Throw; }

 public:
  static const Throw *create(const Expr *value);

  const Expr *value;
};

class TryCatchFinally : public Value {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  TryCatchFinally(
      const Value *tryblock,
      std::vector<std::pair<const Value *, const Value *>> catchblocks,
      const Value *finallyblock)
      : tryblock(tryblock),
        catchblocks(catchblocks),
        finallyblock(finallyblock) {
    ntype = (int)NodeType::TryCatchFinally;
  }

 public:
  static const TryCatchFinally *create(
      const Value *tryblock,
      std::vector<std::pair<const Value *, const Value *>> catchblocks,
      const Value *finallyblock);

  const Value *tryblock;
  std::vector<std::pair<const Value *, const Value *>> catchblocks;
  const Value *finallyblock;
};

class Case : public Value {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Case(const Expr *value, const Value *body) : value(value), body(body) {
    ntype = (int)NodeType::Case;
  }

 public:
  static const Case *create(const Expr *value, const Value *body);

  const Expr *value;
  const Value *body;
};

class Switch : public Value {
 protected:
  bool print_impl(std::ostream &os, PState &state) const override;
  boost::uuids::uuid hash_impl() const override;
  bool verify_impl() const override;

  Switch(const Expr *value, const std::set<const Case *> &cases,
         const Value *defaultcase)
      : value(value), cases(cases), defaultcase(defaultcase) {
    ntype = (int)NodeType::Switch;
  }

 public:
  static const Switch *create(const Expr *value,
                              const std::set<const Case *> &cases,
                              const Value *defaultcase);

  const Expr *value;
  const std::set<const Case *> cases;
  const Value *defaultcase;
};
}  // namespace libquixcc::ir::q

#endif  // __QUIXCC_IR_Q_NODES_CONTROL_H__