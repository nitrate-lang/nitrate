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
  class ExprStmt final : public Stmt {
    FlowPtr<Expr> m_expr;

  public:
    constexpr ExprStmt(FlowPtr<Expr> expr) : Stmt(QAST_ESTMT), m_expr(expr) {}

    constexpr auto get_expr() const { return m_expr; }
  };

  class Block final : public Stmt {
    std::span<FlowPtr<Stmt>> m_items;
    SafetyMode m_safety;

  public:
    Block(BlockItems items, SafetyMode safety)
        : Stmt(QAST_BLOCK), m_items(items), m_safety(safety) {}

    constexpr auto get_items() const { return m_items; }
    constexpr auto get_safety() const { return m_safety; }
  };

  class VarDecl final : public Stmt {
    std::span<FlowPtr<Expr>> m_attributes;
    string m_name;
    NullableFlowPtr<Type> m_type;
    NullableFlowPtr<Expr> m_value;
    VarDeclType m_decl_type;

  public:
    VarDecl(string name, NullableFlowPtr<Type> type,
            NullableFlowPtr<Expr> value, VarDeclType decl_type,
            ExpressionList attributes)
        : Stmt(QAST_VAR),
          m_attributes(attributes),
          m_name(name),
          m_type(type),
          m_value(value),
          m_decl_type(decl_type) {}

    constexpr auto get_name() const { return m_name.get(); }

    constexpr auto get_type() const { return m_type; }
    constexpr auto get_value() const { return m_value; }
    constexpr auto get_decl_type() const { return m_decl_type; }
    constexpr auto get_attributes() const { return m_attributes; }
  };

  class InlineAsm final : public Stmt {
    string m_code;
    std::span<FlowPtr<Expr>> m_args;

  public:
    InlineAsm(string code, ExpressionList args)
        : Stmt(QAST_INLINE_ASM), m_code(code), m_args(args) {}

    constexpr auto get_code() const { return m_code.get(); }

    constexpr auto get_args() const { return m_args; }
  };

  class IfStmt final : public Stmt {
    FlowPtr<Expr> m_cond;
    FlowPtr<Stmt> m_then;
    NullableFlowPtr<Stmt> m_else;

  public:
    constexpr IfStmt(FlowPtr<Expr> cond, FlowPtr<Stmt> then,
                     NullableFlowPtr<Stmt> else_)
        : Stmt(QAST_IF), m_cond(cond), m_then(then), m_else(else_) {}

    constexpr auto get_cond() const { return m_cond; }
    constexpr auto get_then() const { return m_then; }
    constexpr auto get_else() const { return m_else; }
  };

  class WhileStmt final : public Stmt {
    FlowPtr<Expr> m_cond;
    FlowPtr<Stmt> m_body;

  public:
    constexpr WhileStmt(FlowPtr<Expr> cond, FlowPtr<Stmt> body)
        : Stmt(QAST_WHILE), m_cond(cond), m_body(body) {}

    constexpr auto get_cond() const { return m_cond; }
    constexpr auto get_body() const { return m_body; }
  };

  class ForStmt final : public Stmt {
    NullableFlowPtr<Stmt> m_init;
    NullableFlowPtr<Expr> m_cond, m_step;
    FlowPtr<Stmt> m_body;

  public:
    constexpr ForStmt(NullableFlowPtr<Stmt> init, NullableFlowPtr<Expr> cond,
                      NullableFlowPtr<Expr> step, FlowPtr<Stmt> body)
        : Stmt(QAST_FOR),
          m_init(init),
          m_cond(cond),
          m_step(step),
          m_body(body) {}

    constexpr auto get_init() const { return m_init; }
    constexpr auto get_cond() const { return m_cond; }
    constexpr auto get_step() const { return m_step; }
    constexpr auto get_body() const { return m_body; }
  };

  class ForeachStmt final : public Stmt {
    string m_idx_ident;
    string m_val_ident;
    FlowPtr<Expr> m_expr;
    FlowPtr<Stmt> m_body;

  public:
    ForeachStmt(string idx_ident, string val_ident, FlowPtr<Expr> expr,
                FlowPtr<Stmt> body)
        : Stmt(QAST_FOREACH),
          m_idx_ident(idx_ident),
          m_val_ident(val_ident),
          m_expr(expr),
          m_body(body) {}

    constexpr auto get_idx_ident() const { return m_idx_ident.get(); }
    constexpr auto get_val_ident() const { return m_val_ident.get(); }

    constexpr auto get_expr() const { return m_expr; }
    constexpr auto get_body() const { return m_body; }
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
    NullableFlowPtr<Expr> m_value;

  public:
    constexpr ReturnStmt(NullableFlowPtr<Expr> value)
        : Stmt(QAST_RETURN), m_value(value) {}

    constexpr auto get_value() const { return m_value; }
  };

  class ReturnIfStmt final : public Stmt {
    FlowPtr<Expr> m_cond;
    FlowPtr<Expr> m_value;

  public:
    constexpr ReturnIfStmt(FlowPtr<Expr> cond, FlowPtr<Expr> value)
        : Stmt(QAST_RETIF), m_cond(cond), m_value(value) {}

    constexpr auto get_cond() const { return m_cond; }
    constexpr auto get_value() const { return m_value; }
  };

  class CaseStmt final : public Stmt {
    FlowPtr<Expr> m_cond;
    FlowPtr<Stmt> m_body;

  public:
    constexpr CaseStmt(FlowPtr<Expr> cond, FlowPtr<Stmt> body)
        : Stmt(QAST_CASE), m_cond(cond), m_body(body) {}

    constexpr auto get_cond() const { return m_cond; }
    constexpr auto get_body() const { return m_body; }
  };

  class SwitchStmt final : public Stmt {
    FlowPtr<Expr> m_cond;
    std::span<FlowPtr<CaseStmt>> m_cases;
    NullableFlowPtr<Stmt> m_default;

  public:
    SwitchStmt(FlowPtr<Expr> cond, SwitchCases cases,
               NullableFlowPtr<Stmt> default_)
        : Stmt(QAST_SWITCH),
          m_cond(cond),
          m_cases(cases),
          m_default(default_) {}

    constexpr auto get_cond() const { return m_cond; }
    constexpr auto get_cases() const { return m_cases; }
    constexpr auto get_default() const { return m_default; }
  };

  class ExportStmt final : public Stmt {
    std::span<FlowPtr<Expr>> m_attrs;
    string m_abi_name;
    FlowPtr<Stmt> m_body;
    Vis m_vis;

  public:
    ExportStmt(FlowPtr<Stmt> content, string abi_name, Vis vis,
               ExpressionList attrs)
        : Stmt(QAST_EXPORT),
          m_attrs(attrs),
          m_abi_name(abi_name),
          m_body(content),
          m_vis(vis) {}

    constexpr auto get_abi_name() const { return m_abi_name.get(); }
    constexpr auto get_body() const { return m_body; }
    constexpr auto get_vis() const { return m_vis; }
    constexpr auto get_attrs() const { return m_attrs; }
  };

  class ScopeStmt final : public Stmt {
    std::span<string> m_deps;
    string m_name;
    FlowPtr<Stmt> m_body;

  public:
    ScopeStmt(string name, FlowPtr<Stmt> body, ScopeDeps deps = {})
        : Stmt(QAST_SCOPE), m_deps(deps), m_name(name), m_body(body) {}

    constexpr auto get_name() const { return m_name.get(); }
    constexpr auto get_body() const { return m_body; }
    constexpr auto get_deps() const { return m_deps; }
  };

  class TypedefStmt final : public Stmt {
    string m_name;
    FlowPtr<Type> m_type;

  public:
    TypedefStmt(string name, FlowPtr<Type> type)
        : Stmt(QAST_TYPEDEF), m_name(name), m_type(type) {}

    constexpr auto get_name() const { return m_name.get(); }
    constexpr auto get_type() const { return m_type; }
  };

  class EnumDef final : public Stmt {
    std::span<EnumItem> m_items;
    string m_name;
    NullableFlowPtr<Type> m_type;

  public:
    EnumDef(string name, NullableFlowPtr<Type> type, EnumDefItems items)
        : Stmt(QAST_ENUM), m_items(items), m_name(name), m_type(type) {}

    constexpr auto get_name() const { return m_name.get(); }
    constexpr auto get_items() const { return m_items; }
    constexpr auto get_type() const { return m_type; }
  };

  class Function final : public Stmt {
    std::span<FlowPtr<Expr>> m_attributes;
    FuncPurity m_purity;
    std::span<std::pair<string, bool>> m_captures;
    string m_name;
    std::optional<TemplateParameters> m_template_parameters;
    std::span<FuncParam> m_params;
    FlowPtr<Type> m_return;
    NullableFlowPtr<Expr> m_precond, m_postcond;
    NullableFlowPtr<Stmt> m_body;
    bool m_variadic;

  public:
    Function(ExpressionList attributes, FuncPurity purity, FnCaptures captures,
             string name, std::optional<TemplateParameters> params,
             FuncParams fn_params, bool variadic, FlowPtr<Type> return_type,
             NullableFlowPtr<Expr> precond, NullableFlowPtr<Expr> postcond,
             NullableFlowPtr<Stmt> body)
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
          m_body(body),
          m_variadic(variadic) {}

    constexpr auto get_name() const { return m_name.get(); }
    constexpr auto get_attributes() const { return m_attributes; }
    constexpr auto get_purity() const { return m_purity; }
    constexpr auto get_captures() const { return m_captures; }
    constexpr auto get_template_params() const { return m_template_parameters; }
    constexpr auto get_params() const { return m_params; }
    constexpr auto get_variadic() const { return m_variadic; }
    constexpr auto get_return() const { return m_return; }
    constexpr auto get_precond() const { return m_precond; }
    constexpr auto get_postcond() const { return m_postcond; }
    constexpr auto get_body() const { return m_body; }

    bool is_decl() const { return !m_body.has_value(); }
    bool is_def() const { return m_body.has_value(); }
  };

  class StructDef final : public Stmt {
    CompositeType m_comp_type;
    std::span<FlowPtr<Expr>> m_attributes;
    string m_name;
    std::optional<TemplateParameters> m_template_parameters;
    std::span<string> m_names;
    std::span<StructField> m_fields;
    std::span<StructFunction> m_methods;
    std::span<StructFunction> m_static_methods;

  public:
    StructDef(CompositeType comp_type, ExpressionList attributes, string name,
              std::optional<TemplateParameters> params, StructDefNames names,
              StructDefFields fields, StructDefMethods methods,
              StructDefStaticMethods static_methods)
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
    constexpr auto get_composite_type() const { return m_comp_type; }
    constexpr auto get_attributes() const { return m_attributes; }
    constexpr auto get_template_params() const { return m_template_parameters; }
    constexpr auto get_names() const { return m_names; }
    constexpr auto get_fields() const { return m_fields; }
    constexpr auto get_methods() const { return m_methods; }
    constexpr auto get_static_methods() const { return m_static_methods; }
  };

  constexpr bool Stmt::is_expr_stmt(npar_ty_t type) const {
    return is(QAST_ESTMT) && as<ExprStmt>()->get_expr()->is(type);
  }
}  // namespace ncc::parse

#endif
