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

#ifndef __NITRATE_AST_ASTSTMT_H__
#define __NITRATE_AST_ASTSTMT_H__

#include <nitrate-parser/ASTBase.hh>
#include <span>

namespace ncc::parse {
  class npar_pack ExprStmt final : public Stmt {
    RefNode<Expr> m_expr;

  public:
    constexpr ExprStmt(RefNode<Expr> expr) : Stmt(QAST_ESTMT), m_expr(expr) {}

    constexpr let get_expr() const { return m_expr; }
  };

  class Block final : public Stmt {
    std::span<RefNode<Stmt>> m_items;
    SafetyMode m_safety;

  public:
    Block(BlockItems items, SafetyMode safety)
        : Stmt(QAST_BLOCK), m_items(items), m_safety(safety) {}

    constexpr let get_items() const { return m_items; }
    constexpr let get_safety() const { return m_safety; }
  };

  class VarDecl final : public Stmt {
    std::span<RefNode<Expr>> m_attributes;
    core::str_alias m_name;
    RefNode<Type> m_type;
    RefNode<Expr> m_value;
    VarDeclType m_decl_type;

  public:
    VarDecl(core::str_alias name, RefNode<Type> type, RefNode<Expr> value,
            VarDeclType decl_type, ExpressionList attributes)
        : Stmt(QAST_VAR),
          m_attributes(attributes),
          m_name(name),
          m_type(type),
          m_value(value),
          m_decl_type(decl_type) {}

    constexpr auto get_name() const { return m_name.get(); }

    constexpr let get_type() const { return m_type; }
    constexpr let get_value() const { return m_value; }
    constexpr let get_decl_type() const { return m_decl_type; }
    constexpr let get_attributes() const { return m_attributes; }
  };

  class InlineAsm final : public Stmt {
    core::str_alias m_code;
    std::span<RefNode<Expr>> m_args;

  public:
    InlineAsm(core::str_alias code, ExpressionList args)
        : Stmt(QAST_INLINE_ASM), m_code(code), m_args(args) {}

    constexpr auto get_code() const { return m_code.get(); }

    constexpr let get_args() const { return m_args; }
  };

  class npar_pack IfStmt final : public Stmt {
    RefNode<Expr> m_cond;
    RefNode<Stmt> m_then, m_else;

  public:
    constexpr IfStmt(RefNode<Expr> cond, RefNode<Stmt> then,
                     RefNode<Stmt> else_)
        : Stmt(QAST_IF), m_cond(cond), m_then(then), m_else(else_) {}

    constexpr let get_cond() const { return m_cond; }
    constexpr let get_then() const { return m_then; }
    constexpr let get_else() const { return m_else; }
  };

  class WhileStmt final : public Stmt {
    RefNode<Expr> m_cond;
    RefNode<Stmt> m_body;

  public:
    constexpr WhileStmt(RefNode<Expr> cond, RefNode<Stmt> body)
        : Stmt(QAST_WHILE), m_cond(cond), m_body(body) {}

    constexpr let get_cond() const { return m_cond; }
    constexpr let get_body() const { return m_body; }
  };

  class ForStmt final : public Stmt {
    std::optional<RefNode<Stmt>> m_init;
    std::optional<RefNode<Expr>> m_cond, m_step;
    RefNode<Stmt> m_body;

  public:
    constexpr ForStmt(std::optional<RefNode<Stmt>> init,
                      std::optional<RefNode<Expr>> cond,
                      std::optional<RefNode<Expr>> step, RefNode<Stmt> body)
        : Stmt(QAST_FOR),
          m_init(init),
          m_cond(cond),
          m_step(step),
          m_body(body) {}

    constexpr let get_init() const { return m_init; }
    constexpr let get_cond() const { return m_cond; }
    constexpr let get_step() const { return m_step; }
    constexpr let get_body() const { return m_body; }
  };

  class ForeachStmt final : public Stmt {
    core::str_alias m_idx_ident;
    core::str_alias m_val_ident;
    RefNode<Expr> m_expr;
    RefNode<Stmt> m_body;

  public:
    ForeachStmt(core::str_alias idx_ident, core::str_alias val_ident,
                RefNode<Expr> expr, RefNode<Stmt> body)
        : Stmt(QAST_FOREACH),
          m_idx_ident(idx_ident),
          m_val_ident(val_ident),
          m_expr(expr),
          m_body(body) {}

    constexpr auto get_idx_ident() const { return m_idx_ident.get(); }
    constexpr auto get_val_ident() const { return m_val_ident.get(); }

    constexpr let get_expr() const { return m_expr; }
    constexpr let get_body() const { return m_body; }
  };

  class BreakStmt final : public Stmt {
  public:
    constexpr BreakStmt() : Stmt(QAST_BREAK){};
  };

  class ContinueStmt final : public Stmt {
  public:
    constexpr ContinueStmt() : Stmt(QAST_CONTINUE){};
  };

  class ReturnStmt final : public Stmt {
    std::optional<RefNode<Expr>> m_value;

  public:
    constexpr ReturnStmt(std::optional<RefNode<Expr>> value)
        : Stmt(QAST_RETURN), m_value(value) {}

    constexpr let get_value() const { return m_value; }
  };

  class ReturnIfStmt final : public Stmt {
    RefNode<Expr> m_cond;
    RefNode<Expr> m_value;

  public:
    constexpr ReturnIfStmt(RefNode<Expr> cond, RefNode<Expr> value)
        : Stmt(QAST_RETIF), m_cond(cond), m_value(value) {}

    constexpr let get_cond() const { return m_cond; }
    constexpr let get_value() const { return m_value; }
  };

  class CaseStmt final : public Stmt {
    RefNode<Expr> m_cond;
    RefNode<Stmt> m_body;

  public:
    constexpr CaseStmt(RefNode<Expr> cond, RefNode<Stmt> body)
        : Stmt(QAST_CASE), m_cond(cond), m_body(body) {}

    constexpr let get_cond() const { return m_cond; }
    constexpr let get_body() const { return m_body; }
  };

  class SwitchStmt final : public Stmt {
    RefNode<Expr> m_cond;
    std::span<RefNode<CaseStmt>> m_cases;
    RefNode<Stmt> m_default;

  public:
    SwitchStmt(RefNode<Expr> cond, SwitchCases cases, RefNode<Stmt> default_)
        : Stmt(QAST_SWITCH),
          m_cond(cond),
          m_cases(cases),
          m_default(default_) {}

    constexpr let get_cond() const { return m_cond; }
    constexpr let get_cases() const { return m_cases; }
    constexpr let get_default() const { return m_default; }
  };

  class ExportStmt final : public Stmt {
    std::span<RefNode<Expr>> m_attrs;
    core::str_alias m_abi_name;
    RefNode<Stmt> m_body;
    Vis m_vis;

  public:
    ExportStmt(RefNode<Stmt> content, core::str_alias abi_name, Vis vis,
               ExpressionList attrs)
        : Stmt(QAST_EXPORT),
          m_attrs(attrs),
          m_abi_name(abi_name),
          m_body(content),
          m_vis(vis) {}

    constexpr auto get_abi_name() const { return m_abi_name.get(); }

    constexpr let get_body() const { return m_body; }
    constexpr let get_vis() const { return m_vis; }
    constexpr let get_attrs() const { return m_attrs; }
  };

  class ScopeStmt final : public Stmt {
    ScopeDeps m_deps;
    core::str_alias m_name;
    RefNode<Stmt> m_body;

  public:
    ScopeStmt(core::str_alias name, RefNode<Stmt> body, ScopeDeps deps = {})
        : Stmt(QAST_SCOPE), m_deps(deps), m_name(name), m_body(body) {}

    constexpr auto get_name() const { return m_name.get(); }

    constexpr let get_body() const { return m_body; }
    constexpr let get_deps() const { return m_deps; }
  };

  class TypedefStmt final : public Stmt {
    core::str_alias m_name;
    RefNode<Type> m_type;

  public:
    TypedefStmt(core::str_alias name, RefNode<Type> type)
        : Stmt(QAST_TYPEDEF), m_name(name), m_type(type) {}

    constexpr auto get_name() const { return m_name.get(); }

    constexpr let get_type() const { return m_type; }
  };

  class EnumDef final : public Stmt {
    std::span<EnumItem> m_items;
    core::str_alias m_name;
    RefNode<Type> m_type;

  public:
    EnumDef(core::str_alias name, RefNode<Type> type, EnumDefItems items)
        : Stmt(QAST_ENUM), m_items(items), m_name(name), m_type(type) {}

    constexpr auto get_name() const { return m_name.get(); }

    constexpr let get_items() const { return m_items; }
    constexpr let get_type() const { return m_type; }
  };

  class Function final : public Stmt {
    std::span<RefNode<Expr>> m_attributes;
    FuncPurity m_purity;
    std::span<std::pair<core::str_alias, bool>> m_captures;
    core::str_alias m_name;
    std::optional<TemplateParameters> m_template_parameters;
    FuncParams m_params;
    RefNode<Type> m_return;
    std::optional<RefNode<Expr>> m_precond, m_postcond;
    std::optional<RefNode<Stmt>> m_body;

  public:
    Function(ExpressionList attributes, FuncPurity purity, FnCaptures captures,
             core::str_alias name, std::optional<TemplateParameters> params,
             FuncParams fn_params, RefNode<Type> return_type,
             std::optional<RefNode<Expr>> precond,
             std::optional<RefNode<Expr>> postcond,
             std::optional<RefNode<Stmt>> body)
        : Stmt(QAST_FUNCTION),
          m_attributes(attributes),
          m_purity(purity),
          m_captures(captures),
          m_name(name),
          m_template_parameters(params),
          m_params(fn_params),
          m_return(return_type),
          m_precond(precond),
          m_postcond(postcond),
          m_body(body) {}

    constexpr auto get_name() const { return m_name.get(); }

    constexpr let get_attributes() const { return m_attributes; }
    constexpr let get_purity() const { return m_purity; }
    constexpr let get_captures() const { return m_captures; }
    constexpr let get_template_params() const { return m_template_parameters; }
    constexpr let get_params() const { return m_params; }
    constexpr let get_return() const { return m_return; }
    constexpr let get_precond() const { return m_precond; }
    constexpr let get_postcond() const { return m_postcond; }
    constexpr let get_body() const { return m_body; }

    bool is_decl() const { return !m_body.has_value(); }
    bool is_def() const { return m_body.has_value(); }
  };

  class StructDef final : public Stmt {
    CompositeType m_comp_type;
    std::span<RefNode<Expr>> m_attributes;
    core::str_alias m_name;
    std::optional<TemplateParameters> m_template_parameters;
    std::span<core::str_alias> m_names;
    std::span<StructField> m_fields;
    std::span<StructFunction> m_methods;
    std::span<StructFunction> m_static_methods;

  public:
    StructDef(CompositeType comp_type, ExpressionList attributes,
              core::str_alias name, std::optional<TemplateParameters> params,
              StructDefNames names, StructDefFields fields,
              StructDefMethods methods, StructDefStaticMethods static_methods)
        : Stmt(QAST_STRUCT),
          m_comp_type(comp_type),
          m_attributes(attributes),
          m_name(name),
          m_template_parameters(params),
          m_names(names),
          m_fields(fields),
          m_methods(methods),
          m_static_methods(static_methods) {}

    constexpr auto get_name() const { return m_name.get(); }

    constexpr let get_composite_type() const { return m_comp_type; }
    constexpr let get_attributes() const { return m_attributes; }
    constexpr let get_template_params() const { return m_template_parameters; }
    constexpr let get_names() const { return m_names; }
    constexpr let get_fields() const { return m_fields; }
    constexpr let get_methods() const { return m_methods; }
    constexpr let get_static_methods() const { return m_static_methods; }
  };

  constexpr bool Stmt::is_expr_stmt(npar_ty_t type) const {
    return is(QAST_ESTMT) && as<ExprStmt>()->get_expr()->is(type);
  }
}  // namespace ncc::parse

#endif
