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

#include <quixcc/IR/Q/Expr.h>
#include <quixcc/IR/Q/QIR.h>

namespace libquixcc::ir::q {
  class IfElse : public Value {
protected:
    bool print_impl(std::ostream &os, PState &state) const override;
    boost::uuids::uuid hash_impl() const override;
    bool verify_impl() const override;

    IfElse(Expr *cond, Value *then, Value *els) : cond(cond), then(then), els(els) {
      ntype = (int)QType::IfElse;
    }

public:
    static IfElse *create(Expr *cond, Value *then, Value *els);

    Expr *cond;
    Value *then;
    Value *els;
  };

  class While : public Value {
protected:
    bool print_impl(std::ostream &os, PState &state) const override;
    boost::uuids::uuid hash_impl() const override;
    bool verify_impl() const override;

    While(Expr *cond, Value *body) : cond(cond), body(body) { ntype = (int)QType::While; }

public:
    static While *create(Expr *cond, Value *body);

    Expr *cond;
    Value *body;
  };

  class For : public Value {
protected:
    bool print_impl(std::ostream &os, PState &state) const override;
    boost::uuids::uuid hash_impl() const override;
    bool verify_impl() const override;

    For(Value *init, Expr *cond, Expr *step, Value *body)
        : init(init), cond(cond), step(step), body(body) {
      ntype = (int)QType::For;
    }

public:
    static For *create(Value *init, Expr *cond, Expr *step, Value *body);

    Value *init;
    Expr *cond;
    Expr *step;
    Value *body;
  };

  class Loop : public Value {
protected:
    bool print_impl(std::ostream &os, PState &state) const override;
    boost::uuids::uuid hash_impl() const override;
    bool verify_impl() const override;

    Loop(Value *body) : body(body) { ntype = (int)QType::Loop; }

public:
    static Loop *create(Value *body);

    Value *body;
  };

  class Break : public Value {
protected:
    bool print_impl(std::ostream &os, PState &state) const override;
    boost::uuids::uuid hash_impl() const override;
    bool verify_impl() const override;

    Break() { ntype = (int)QType::Break; }

public:
    static Break *create();
  };

  class Continue : public Value {
protected:
    bool print_impl(std::ostream &os, PState &state) const override;
    boost::uuids::uuid hash_impl() const override;
    bool verify_impl() const override;

    Continue() { ntype = (int)QType::Continue; }

public:
    static Continue *create();
  };

  class Ret : public Value {
protected:
    bool print_impl(std::ostream &os, PState &state) const override;
    boost::uuids::uuid hash_impl() const override;
    bool verify_impl() const override;

    Ret(Expr *value) : value(value) { ntype = (int)QType::Ret; }

public:
    static Ret *create(Expr *value);

    Expr *value;
  };

  class Throw : public Value {
protected:
    bool print_impl(std::ostream &os, PState &state) const override;
    boost::uuids::uuid hash_impl() const override;
    bool verify_impl() const override;

    Throw(Expr *value) : value(value) { ntype = (int)QType::Throw; }

public:
    static Throw *create(Expr *value);

    Expr *value;
  };

  class TryCatchFinally : public Value {
protected:
    bool print_impl(std::ostream &os, PState &state) const override;
    boost::uuids::uuid hash_impl() const override;
    bool verify_impl() const override;

    TryCatchFinally(Value *tryblock, std::vector<std::pair<Value *, Value *>> catchblocks,
                    Value *finallyblock)
        : tryblock(tryblock), catchblocks(catchblocks), finallyblock(finallyblock) {
      ntype = (int)QType::TryCatchFinally;
    }

public:
    static TryCatchFinally *create(Value *tryblock,
                                   std::vector<std::pair<Value *, Value *>> catchblocks,
                                   Value *finallyblock);

    Value *tryblock;
    std::vector<std::pair<Value *, Value *>> catchblocks;
    Value *finallyblock;
  };

  class Case : public Value {
protected:
    bool print_impl(std::ostream &os, PState &state) const override;
    boost::uuids::uuid hash_impl() const override;
    bool verify_impl() const override;

    Case(Expr *value, Value *body) : value(value), body(body) { ntype = (int)QType::Case; }

public:
    static Case *create(Expr *value, Value *body);

    Expr *value;
    Value *body;
  };

  class Switch : public Value {
protected:
    bool print_impl(std::ostream &os, PState &state) const override;
    boost::uuids::uuid hash_impl() const override;
    bool verify_impl() const override;

    Switch(Expr *value, std::vector<Case *> &cases, Value *defaultcase)
        : value(value), cases(cases), defaultcase(defaultcase) {
      ntype = (int)QType::Switch;
    }

public:
    static Switch *create(Expr *value, std::vector<Case *> &cases, Value *defaultcase);

    Expr *value;
    std::vector<Case *> cases;
    Value *defaultcase;
  };
} // namespace libquixcc::ir::q

#endif // __QUIXCC_IR_Q_NODES_CONTROL_H__