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
#include <span>

namespace ncc::parse {
  class StmtExpr final : public Expr {
    FlowPtr<Stmt> m_stmt;

  public:
    constexpr StmtExpr(auto stmt) : Expr(QAST_SEXPR), m_stmt(stmt) {}

    constexpr auto get_stmt() const { return m_stmt; }
  };

  class TypeExpr final : public Expr {
    FlowPtr<Type> m_type;

  public:
    constexpr TypeExpr(auto type) : Expr(QAST_TEXPR), m_type(type) {}

    constexpr auto get_type() const { return m_type; }
  };

  class UnaryExpr final : public Expr {
    FlowPtr<Expr> m_rhs;
    lex::Operator m_op;

  public:
    constexpr UnaryExpr(auto op, auto rhs)
        : Expr(QAST_UNEXPR), m_rhs(rhs), m_op(op) {}

    constexpr auto get_rhs() const { return m_rhs; }
    constexpr auto get_op() const { return m_op; }
  };

  class BinExpr final : public Expr {
    FlowPtr<Expr> m_lhs, m_rhs;
    lex::Operator m_op;

  public:
    constexpr BinExpr(auto lhs, auto op, auto rhs)
        : Expr(QAST_BINEXPR), m_lhs(lhs), m_rhs(rhs), m_op(op) {}

    constexpr auto get_lhs() const { return m_lhs; }
    constexpr auto get_rhs() const { return m_rhs; }
    constexpr auto get_op() const { return m_op; }
  };

  class PostUnaryExpr final : public Expr {
    FlowPtr<Expr> m_lhs;
    lex::Operator m_op;

  public:
    constexpr PostUnaryExpr(auto lhs, auto op)
        : Expr(QAST_POST_UNEXPR), m_lhs(lhs), m_op(op) {}

    constexpr auto get_lhs() const { return m_lhs; }
    constexpr auto get_op() const { return m_op; }
  };

  class TernaryExpr final : public Expr {
    FlowPtr<Expr> m_cond, m_lhs, m_rhs;

  public:
    constexpr TernaryExpr(auto cond, auto lhs, auto rhs)
        : Expr(QAST_TEREXPR), m_cond(cond), m_lhs(lhs), m_rhs(rhs) {}

    constexpr auto get_cond() const { return m_cond; }
    constexpr auto get_lhs() const { return m_lhs; }
    constexpr auto get_rhs() const { return m_rhs; }
  };

  class ConstInt final : public Expr {
    string m_value;

  public:
    constexpr ConstInt(auto value) : Expr(QAST_INT), m_value(value) {}

    constexpr auto get_value() const { return m_value; }
  };

  class ConstFloat final : public Expr {
    string m_value;

  public:
    constexpr ConstFloat(auto value) : Expr(QAST_FLOAT), m_value(value) {}

    constexpr auto get_value() const { return m_value; }
  };

  class ConstBool final : public Expr {
    bool m_value;

  public:
    constexpr ConstBool(auto value) : Expr(QAST_BOOL), m_value(value) {}

    constexpr auto get_value() const { return m_value; }
  };

  class ConstString final : public Expr {
    string m_value;

  public:
    constexpr ConstString(auto value) : Expr(QAST_STRING), m_value(value) {}

    constexpr auto get_value() const { return m_value; }
  };

  class ConstChar final : public Expr {
    uint8_t m_value;

  public:
    constexpr ConstChar(auto value) : Expr(QAST_CHAR), m_value(value) {}

    constexpr auto get_value() const { return m_value; }
  };

  class ConstNull final : public Expr {
  public:
    constexpr ConstNull() : Expr(QAST_NULL) {}
  };

  class ConstUndef final : public Expr {
  public:
    constexpr ConstUndef() : Expr(QAST_UNDEF) {}
  };

  class Call final : public Expr {
    FlowPtr<Expr> m_func;
    std::span<const CallArg> m_args;

  public:
    constexpr Call(auto func, auto args)
        : Expr(QAST_CALL), m_func(func), m_args(args) {}

    constexpr auto get_func() const { return m_func; }
    constexpr auto get_args() const { return m_args; }
  };

  class TemplCall final : public Expr {
    FlowPtr<Expr> m_func;
    std::span<CallArg> m_template_args, m_args;

  public:
    constexpr TemplCall(auto func, auto args, auto template_args)
        : Expr(QAST_TEMPL_CALL),
          m_func(func),
          m_template_args(template_args),
          m_args(args) {}

    constexpr auto get_func() const { return m_func; }
    constexpr auto get_template_args() const { return m_template_args; }
    constexpr auto get_args() const { return m_args; }
  };

  class List final : public Expr {
    std::span<FlowPtr<Expr>> m_items;

  public:
    constexpr List(auto items) : Expr(QAST_LIST), m_items(items) {}

    constexpr auto get_items() const { return m_items; }
  };

  class Assoc final : public Expr {
    FlowPtr<Expr> m_key, m_value;

  public:
    constexpr Assoc(auto key, auto value)
        : Expr(QAST_ASSOC), m_key(key), m_value(value) {}

    constexpr auto get_key() const { return m_key; }
    constexpr auto get_value() const { return m_value; }
  };

  class Index final : public Expr {
    FlowPtr<Expr> m_base, m_index;

  public:
    constexpr Index(auto base, auto index)
        : Expr(QAST_INDEX), m_base(base), m_index(index) {}

    constexpr auto get_base() const { return m_base; }
    constexpr auto get_index() const { return m_index; }
  };

  class Slice final : public Expr {
    FlowPtr<Expr> m_base, m_start, m_end;

  public:
    constexpr Slice(auto base, auto start, auto end)
        : Expr(QAST_SLICE), m_base(base), m_start(start), m_end(end) {}

    constexpr auto get_base() const { return m_base; }
    constexpr auto get_start() const { return m_start; }
    constexpr auto get_end() const { return m_end; }
  };

  class FString final : public Expr {
    std::span<std::variant<string, FlowPtr<Expr>>> m_items;

  public:
    constexpr FString(auto items) : Expr(QAST_FSTRING), m_items(items) {}

    constexpr auto get_items() const { return m_items; }
  };

  class Ident final : public Expr {
    string m_name;

  public:
    constexpr Ident(auto name) : Expr(QAST_IDENT), m_name(name) {}

    constexpr auto get_name() const { return m_name; }
  };

  class SeqPoint final : public Expr {
    std::span<FlowPtr<Expr>> m_items;

  public:
    constexpr SeqPoint(auto items) : Expr(QAST_SEQ), m_items(items) {}

    constexpr auto get_items() const { return m_items; }
  };

  constexpr bool Expr::is_stmt_expr(npar_ty_t type) const {
    return is(QAST_SEXPR) && as<StmtExpr>()->get_stmt()->is(type);
  }
}  // namespace ncc::parse

#endif
