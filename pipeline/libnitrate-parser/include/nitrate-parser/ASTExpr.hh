////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///     .-----------------.    .----------------.     .----------------.     ///
///    | .--------------. |   | .--------------. |   | .--------------. |    ///
///    | | ____  _____  | |   | |     ____     | |   | |    ______    | |    ///
///    | ||_   _|_   _| | |   | |   .'    `.   | |   | |   / ____ `.  | |    ///
///    | |  |   \ | |   | |   | |  /  .--.  \  | |   | |   `'  __) |  | |    ///
///    | |  | |\ \| |   | |   | |  | |    | |  | |   | |   _  |__ '.  | |    ///
///    | | _| |_\   |_  | |   | |  \  `--'  /  | |   | |  | \____) |  | |    ///
///    | ||_____|\____| | |   | |   `.____.'   | |   | |   \______.'  | |    ///
///    | |              | |   | |              | |   | |              | |    ///
///    | '--------------' |   | '--------------' |   | '--------------' |    ///
///     '----------------'     '----------------'     '----------------'     ///
///                                                                          ///
///   * NITRATE TOOLCHAIN - The official toolchain for the Nitrate language. ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The Nitrate Toolchain is free software; you can redistribute it or     ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The Nitrate Toolcain is distributed in the hope that it will be        ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the Nitrate Toolchain; if not, see                  ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#ifndef __NITRATE_AST_ASTEXPR_H__
#define __NITRATE_AST_ASTEXPR_H__

#include <nitrate-parser/ASTBase.hh>

namespace npar {
  class npar_pack StmtExpr final : public Expr {
    Stmt *m_stmt;

  public:
    constexpr StmtExpr(Stmt *stmt) : Expr(QAST_SEXPR), m_stmt(stmt) {}

    auto get_stmt() const { return m_stmt; }
  };

  class npar_pack TypeExpr final : public Expr {
    Type *m_type;

  public:
    constexpr TypeExpr(Type *type) : Expr(QAST_TEXPR), m_type(type) {}

    auto get_type() const { return m_type; }
  };

  class npar_pack UnaryExpr final : public Expr {
    Expr *m_rhs;
    qlex_op_t m_op;

  public:
    constexpr UnaryExpr(qlex_op_t op, Expr *rhs)
        : Expr(QAST_UNEXPR), m_rhs(rhs), m_op(op) {}

    auto get_rhs() const { return m_rhs; }
    auto get_op() const { return m_op; }
  };

  class npar_pack BinExpr final : public Expr {
    Expr *m_lhs, *m_rhs;
    qlex_op_t m_op;

  public:
    constexpr BinExpr(Expr *lhs, qlex_op_t op, Expr *rhs)
        : Expr(QAST_BINEXPR), m_lhs(lhs), m_rhs(rhs), m_op(op) {}

    auto get_lhs() const { return m_lhs; }
    auto get_rhs() const { return m_rhs; }
    auto get_op() const { return m_op; }
  };

  class npar_pack PostUnaryExpr final : public Expr {
    Expr *m_lhs;
    qlex_op_t m_op;

  public:
    constexpr PostUnaryExpr(Expr *lhs, qlex_op_t op = qOpTernary)
        : Expr(QAST_POST_UNEXPR), m_lhs(lhs), m_op(op) {}

    auto get_lhs() const { return m_lhs; }
    auto get_op() const { return m_op; }
  };

  class npar_pack TernaryExpr final : public Expr {
    Expr *m_cond, *m_lhs, *m_rhs;

  public:
    constexpr TernaryExpr(Expr *cond, Expr *lhs, Expr *rhs)
        : Expr(QAST_TEREXPR), m_cond(cond), m_lhs(lhs), m_rhs(rhs) {}

    auto get_cond() const { return m_cond; }
    auto get_lhs() const { return m_lhs; }
    auto get_rhs() const { return m_rhs; }
  };

  class ConstInt final : public Expr {
    ASTString m_value;

  public:
    ConstInt(ASTString value) : Expr(QAST_INT), m_value(value) {}

    let get_value() const { return m_value.get(); }
  };

  class ConstFloat final : public Expr {
    ASTString m_value;

  public:
    ConstFloat(ASTString value) : Expr(QAST_FLOAT), m_value(value) {}

    let get_value() const { return m_value.get(); }
  };

  class npar_pack ConstBool final : public Expr {
    bool m_value;

  public:
    constexpr ConstBool(bool value = false) : Expr(QAST_BOOL), m_value(value) {}

    auto get_value() const { return m_value; }
  };

  class ConstString final : public Expr {
    ASTString m_value;

  public:
    ConstString(ASTString value) : Expr(QAST_STRING), m_value(value) {}

    let get_value() const { return m_value.get(); }
  };

  class npar_pack ConstChar final : public Expr {
    uint8_t m_value;

  public:
    constexpr ConstChar(uint8_t value = 0) : Expr(QAST_CHAR), m_value(value) {}

    auto get_value() const { return m_value; }
  };

  class npar_pack ConstNull final : public Expr {
  public:
    constexpr ConstNull() : Expr(QAST_NULL) {}
  };

  class npar_pack ConstUndef final : public Expr {
  public:
    constexpr ConstUndef() : Expr(QAST_UNDEF) {}
  };

  class Call final : public Expr {
    Expr *m_func;
    CallArgs m_args;

  public:
    Call(Expr *func, CallArgs args = {})
        : Expr(QAST_CALL), m_func(func), m_args(args) {}

    auto get_func() const { return m_func; }
    let get_args() const { return m_args; }
  };

  class TemplCall final : public Expr {
    Expr *m_func;
    CallArgs m_template_args, m_args;

  public:
    TemplCall(Expr *func, CallArgs args = {}, CallArgs template_args = {})
        : Expr(QAST_TEMPL_CALL),
          m_func(func),
          m_template_args(template_args),
          m_args(args) {}

    auto get_func() const { return m_func; }
    let get_template_args() const { return m_template_args; }
    let get_args() const { return m_args; }
  };

  class List final : public Expr {
    ExpressionList m_items;

  public:
    List(const ExpressionList &items) : Expr(QAST_LIST), m_items(items) {}

    let get_items() const { return m_items; }
  };

  class npar_pack Assoc final : public Expr {
    Expr *m_key, *m_value;

  public:
    constexpr Assoc(Expr *key, Expr *value)
        : Expr(QAST_ASSOC), m_key(key), m_value(value) {}

    auto get_key() const { return m_key; }
    auto get_value() const { return m_value; }
  };

  class Field final : public Expr {
    Expr *m_base;
    ASTString m_field;

  public:
    Field(Expr *base, ASTString field)
        : Expr(QAST_FIELD), m_base(base), m_field(field) {}

    auto get_base() const { return m_base; }
    let get_field() const { return m_field.get(); }
  };

  class npar_pack Index final : public Expr {
    Expr *m_base, *m_index;

  public:
    constexpr Index(Expr *base, Expr *index)
        : Expr(QAST_INDEX), m_base(base), m_index(index) {}

    auto get_base() const { return m_base; }
    auto get_index() const { return m_index; }
  };

  class npar_pack Slice final : public Expr {
    Expr *m_base, *m_start, *m_end;

  public:
    constexpr Slice(Expr *base, Expr *start, Expr *end)
        : Expr(QAST_SLICE), m_base(base), m_start(start), m_end(end) {}

    auto get_base() const { return m_base; }
    auto get_start() const { return m_start; }
    auto get_end() const { return m_end; }
  };

  class FString final : public Expr {
    FStringItems m_items;

  public:
    FString(FStringItems items = {}) : Expr(QAST_FSTRING), m_items(items) {}

    let get_items() const { return m_items; }
  };

  class Ident final : public Expr {
    ASTString m_name;

  public:
    Ident(ASTString name) : Expr(QAST_IDENT), m_name(name) {}

    let get_name() const { return m_name.get(); }
  };

  class SeqPoint final : public Expr {
    ExpressionList m_items;

  public:
    SeqPoint(const ExpressionList &items) : Expr(QAST_SEQ), m_items(items) {}

    let get_items() const { return m_items; }
  };

  constexpr bool Expr::is_stmt_expr(npar_ty_t type) const {
    return is(QAST_SEXPR) && as<npar::StmtExpr>()->get_stmt()->is(type);
  }
}  // namespace npar

#endif
