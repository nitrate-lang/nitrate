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
#include <nitrate-parser/ASTCommon.hh>

namespace npar {
  class npar_pack StmtExpr final : public Expr {
    Stmt *m_stmt;

  public:
    constexpr StmtExpr(Stmt *stmt) : Expr(QAST_SEXPR), m_stmt(stmt) {}

    constexpr auto get_stmt() const { return m_stmt; }
  };

  class npar_pack TypeExpr final : public Expr {
    Type *m_type;

  public:
    constexpr TypeExpr(Type *type) : Expr(QAST_TEXPR), m_type(type) {}

    constexpr auto get_type() const { return m_type; }
  };

  class npar_pack UnaryExpr final : public Expr {
    Expr *m_rhs;
    qlex_op_t m_op;

  public:
    constexpr UnaryExpr(qlex_op_t op, Expr *rhs)
        : Expr(QAST_UNEXPR), m_rhs(rhs), m_op(op) {}

    constexpr auto get_rhs() const { return m_rhs; }
    constexpr auto get_op() const { return m_op; }
  };

  class npar_pack BinExpr final : public Expr {
    Expr *m_lhs, *m_rhs;
    qlex_op_t m_op;

  public:
    constexpr BinExpr(Expr *lhs, qlex_op_t op, Expr *rhs)
        : Expr(QAST_BINEXPR), m_lhs(lhs), m_rhs(rhs), m_op(op) {}

    constexpr auto get_lhs() const { return m_lhs; }
    constexpr auto get_rhs() const { return m_rhs; }
    constexpr auto get_op() const { return m_op; }
  };

  class npar_pack PostUnaryExpr final : public Expr {
    Expr *m_lhs;
    qlex_op_t m_op;

  public:
    constexpr PostUnaryExpr(Expr *lhs, qlex_op_t op)
        : Expr(QAST_POST_UNEXPR), m_lhs(lhs), m_op(op) {}

    constexpr auto get_lhs() const { return m_lhs; }
    constexpr auto get_op() const { return m_op; }
  };

  class npar_pack TernaryExpr final : public Expr {
    Expr *m_cond, *m_lhs, *m_rhs;

  public:
    constexpr TernaryExpr(Expr *cond, Expr *lhs, Expr *rhs)
        : Expr(QAST_TEREXPR), m_cond(cond), m_lhs(lhs), m_rhs(rhs) {}

    constexpr auto get_cond() const { return m_cond; }
    constexpr auto get_lhs() const { return m_lhs; }
    constexpr auto get_rhs() const { return m_rhs; }
  };

  class npar_pack ConstInt final : public Expr {
    qcore::str_alias m_value;

  public:
    constexpr ConstInt(qcore::str_alias value)
        : Expr(QAST_INT), m_value(value) {}

    constexpr auto get_value() const { return m_value.get(); }
  };

  class npar_pack ConstFloat final : public Expr {
    qcore::str_alias m_value;

  public:
    constexpr ConstFloat(qcore::str_alias value)
        : Expr(QAST_FLOAT), m_value(value) {}

    constexpr auto get_value() const { return m_value.get(); }
  };

  class npar_pack ConstBool final : public Expr {
    bool m_value;

  public:
    constexpr ConstBool(bool value) : Expr(QAST_BOOL), m_value(value) {}

    constexpr auto get_value() const { return m_value; }
  };

  class npar_pack ConstString final : public Expr {
    qcore::str_alias m_value;

  public:
    constexpr ConstString(qcore::str_alias value)
        : Expr(QAST_STRING), m_value(value) {}

    constexpr auto get_value() const { return m_value.get(); }
  };

  class npar_pack ConstChar final : public Expr {
    uint8_t m_value;

  public:
    constexpr ConstChar(uint8_t value) : Expr(QAST_CHAR), m_value(value) {}

    constexpr auto get_value() const { return m_value; }
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
    std::span<CallArg> m_args;

  public:
    Call(Expr *func, CallArgs args)
        : Expr(QAST_CALL), m_func(func), m_args(args) {}

    constexpr auto get_func() const { return m_func; }
    constexpr auto get_args() const { return m_args; }
  };

  class TemplCall final : public Expr {
    Expr *m_func;
    std::span<CallArg> m_template_args, m_args;

  public:
    TemplCall(Expr *func, CallArgs args, CallArgs template_args)
        : Expr(QAST_TEMPL_CALL),
          m_func(func),
          m_template_args(template_args),
          m_args(args) {}

    constexpr auto get_func() const { return m_func; }
    constexpr auto get_template_args() const { return m_template_args; }
    constexpr auto get_args() const { return m_args; }
  };

  class List final : public Expr {
    std::span<Expr *> m_items;

  public:
    List(ExpressionList items) : Expr(QAST_LIST), m_items(items) {}

    constexpr auto get_items() const { return m_items; }
  };

  class npar_pack Assoc final : public Expr {
    Expr *m_key, *m_value;

  public:
    constexpr Assoc(Expr *key, Expr *value)
        : Expr(QAST_ASSOC), m_key(key), m_value(value) {}

    constexpr auto get_key() const { return m_key; }
    constexpr auto get_value() const { return m_value; }
  };

  class npar_pack Field final : public Expr {
    Expr *m_base;
    qcore::str_alias m_field;

  public:
    constexpr Field(Expr *base, qcore::str_alias field)
        : Expr(QAST_FIELD), m_base(base), m_field(field) {}

    constexpr auto get_base() const { return m_base; }
    constexpr auto get_field() const { return m_field.get(); }
  };

  class npar_pack Index final : public Expr {
    Expr *m_base, *m_index;

  public:
    constexpr Index(Expr *base, Expr *index)
        : Expr(QAST_INDEX), m_base(base), m_index(index) {}

    constexpr auto get_base() const { return m_base; }
    constexpr auto get_index() const { return m_index; }
  };

  class npar_pack Slice final : public Expr {
    Expr *m_base, *m_start, *m_end;

  public:
    constexpr Slice(Expr *base, Expr *start, Expr *end)
        : Expr(QAST_SLICE), m_base(base), m_start(start), m_end(end) {}

    constexpr auto get_base() const { return m_base; }
    constexpr auto get_start() const { return m_start; }
    constexpr auto get_end() const { return m_end; }
  };

  class FString final : public Expr {
    std::span<std::variant<qcore::str_alias, Expr *>> m_items;

  public:
    FString(FStringItems items) : Expr(QAST_FSTRING), m_items(items) {}

    constexpr let get_items() const { return m_items; }
  };

  class npar_pack Ident final : public Expr {
    qcore::str_alias m_name;

  public:
    constexpr Ident(qcore::str_alias name) : Expr(QAST_IDENT), m_name(name) {}

    constexpr auto get_name() const { return m_name.get(); }
  };

  class SeqPoint final : public Expr {
    std::span<Expr *> m_items;

  public:
    SeqPoint(ExpressionList items) : Expr(QAST_SEQ), m_items(items) {}

    constexpr let get_items() const { return m_items; }
  };

  constexpr bool Expr::is_stmt_expr(npar_ty_t type) const {
    return is(QAST_SEXPR) && as<npar::StmtExpr>()->get_stmt()->is(type);
  }
}  // namespace npar

#endif
