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

#ifndef __NITRATE_PARSER_ASTEXPR_H__
#define __NITRATE_PARSER_ASTEXPR_H__

#ifndef __cplusplus
#error "This code requires c++"
#endif

#include <nitrate-parser/ASTBase.hh>

namespace npar {
  class StmtExpr : public Expr {
    Stmt *m_stmt;

  public:
    constexpr StmtExpr(Stmt *stmt) : Expr(QAST_SEXPR), m_stmt(stmt) {}

    let get_stmt() const { return m_stmt; }
  };

  class TypeExpr : public Expr {
    Type *m_type;

  public:
    constexpr TypeExpr(Type *type) : Expr(QAST_TEXPR), m_type(type) {}

    let get_type() const { return m_type; }
  };

  class UnaryExpr : public Expr {
    Expr *m_rhs;
    qlex_op_t m_op;

  public:
    constexpr UnaryExpr(qlex_op_t op, Expr *rhs)
        : Expr(QAST_UNEXPR), m_rhs(rhs), m_op(op) {}

    let get_rhs() const { return m_rhs; }
    qlex_op_t get_op() const { return m_op; }
  };

  class BinExpr : public Expr {
    Expr *m_lhs;
    Expr *m_rhs;
    qlex_op_t m_op;

  public:
    constexpr BinExpr(Expr *lhs, qlex_op_t op, Expr *rhs)
        : Expr(QAST_BINEXPR), m_lhs(lhs), m_rhs(rhs), m_op(op) {}

    let get_lhs() const { return m_lhs; }
    let get_rhs() const { return m_rhs; }
    qlex_op_t get_op() const { return m_op; }
  };

  class PostUnaryExpr : public Expr {
    Expr *m_lhs;
    qlex_op_t m_op;

  public:
    constexpr PostUnaryExpr(Expr *lhs, qlex_op_t op = qOpTernary)
        : Expr(QAST_POST_UNEXPR), m_lhs(lhs), m_op(op) {}

    let get_lhs() const { return m_lhs; }
    let get_op() const { return m_op; }
  };

  class TernaryExpr : public Expr {
    Expr *m_cond;
    Expr *m_lhs;
    Expr *m_rhs;

  public:
    constexpr TernaryExpr(Expr *cond, Expr *lhs, Expr *rhs)
        : Expr(QAST_TEREXPR), m_cond(cond), m_lhs(lhs), m_rhs(rhs) {}

    let get_cond() const { return m_cond; }
    let get_lhs() const { return m_lhs; }
    let get_rhs() const { return m_rhs; }
  };

  class ConstInt : public Expr {
    SmallString m_value;

  public:
    ConstInt(SmallString value) : Expr(QAST_INT), m_value(value) {}

    let get_value() const { return m_value; }
  };

  class ConstFloat : public Expr {
    SmallString m_value;

  public:
    ConstFloat(SmallString value) : Expr(QAST_FLOAT), m_value(value) {}

    let get_value() const { return m_value; }
  };

  class ConstBool : public Expr {
    bool m_value;

  public:
    constexpr ConstBool(bool value = false) : Expr(QAST_BOOL), m_value(value) {}

    bool get_value() const { return m_value; }
  };

  class ConstString : public Expr {
    SmallString m_value;

  public:
    ConstString(SmallString value) : Expr(QAST_STRING), m_value(value) {}

    let get_value() const { return m_value; }
  };

  class ConstChar : public Expr {
    uint8_t m_value;

  public:
    constexpr ConstChar(uint8_t value = 0) : Expr(QAST_CHAR), m_value(value) {}

    uint8_t get_value() const { return m_value; }
  };

  class ConstNull : public Expr {
  public:
    constexpr ConstNull() : Expr(QAST_NULL) {}
  };

  class ConstUndef : public Expr {
  public:
    constexpr ConstUndef() : Expr(QAST_UNDEF) {}
  };

  class Call : public Expr {
    Expr *m_func;
    CallArgs m_args;

  public:
    Call(Expr *func, CallArgs args = {})
        : Expr(QAST_CALL), m_func(func), m_args(args) {}

    let get_func() const { return m_func; }
    let get_args() const { return m_args; }
  };

  class TemplCall : public Expr {
    TemplateArgs m_template_args;
    Expr *m_func;
    CallArgs m_args;

  public:
    TemplCall(Expr *func, CallArgs args = {}, TemplateArgs template_args = {})
        : Expr(QAST_TEMPL_CALL),
          m_template_args(template_args),
          m_func(func),
          m_args(args) {}

    let get_func() const { return m_func; }
    let get_template_args() const { return m_template_args; }
    let get_args() const { return m_args; }
  };

  class List : public Expr {
    ExpressionList m_items;

  public:
    List(const ExpressionList &items) : Expr(QAST_LIST), m_items(items) {}

    let get_items() const { return m_items; }
  };

  class Assoc : public Expr {
    Expr *m_key;
    Expr *m_value;

  public:
    constexpr Assoc(Expr *key, Expr *value)
        : Expr(QAST_ASSOC), m_key(key), m_value(value) {}

    let get_key() const { return m_key; }
    let get_value() const { return m_value; }
  };

  class Field : public Expr {
    Expr *m_base;
    SmallString m_field;

  public:
    Field(Expr *base, SmallString field)
        : Expr(QAST_FIELD), m_base(base), m_field(field) {}

    let get_base() const { return m_base; }
    let get_field() const { return m_field; }
  };

  class Index : public Expr {
    Expr *m_base;
    Expr *m_index;

  public:
    constexpr Index(Expr *base, Expr *index)
        : Expr(QAST_INDEX), m_base(base), m_index(index) {}

    let get_base() const { return m_base; }
    let get_index() const { return m_index; }
  };

  class Slice : public Expr {
    Expr *m_base;
    Expr *m_start;
    Expr *m_end;

  public:
    constexpr Slice(Expr *base, Expr *start, Expr *end)
        : Expr(QAST_SLICE), m_base(base), m_start(start), m_end(end) {}

    let get_base() const { return m_base; }
    let get_start() const { return m_start; }
    let get_end() const { return m_end; }
  };

  class FString : public Expr {
    FStringItems m_items;

  public:
    FString(FStringItems items = {}) : Expr(QAST_FSTRING), m_items(items) {}

    let get_items() const { return m_items; }
  };

  class Ident : public Expr {
    SmallString m_name;

  public:
    Ident(SmallString name) : Expr(QAST_IDENT), m_name(name) {}

    let get_name() const { return m_name; }
  };

  class SeqPoint : public Expr {
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
