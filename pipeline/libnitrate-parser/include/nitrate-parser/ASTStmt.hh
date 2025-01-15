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
    constexpr ExprStmt(auto expr) : Stmt(QAST_ESTMT), m_expr(expr) {}

    [[nodiscard]] constexpr auto GetExpr() const { return m_expr; }
  };

  class Block final : public Stmt {
    std::span<FlowPtr<Stmt>> m_items;
    SafetyMode m_safety;

  public:
    constexpr Block(auto items, auto safety)
        : Stmt(QAST_BLOCK), m_items(items), m_safety(safety) {}

    [[nodiscard]] constexpr auto GetItems() const { return m_items; }
    [[nodiscard]] constexpr auto GetSafety() const { return m_safety; }
  };

  class VarDecl final : public Stmt {
    std::span<FlowPtr<Expr>> m_attributes;
    NullableFlowPtr<Type> m_type;
    NullableFlowPtr<Expr> m_value;
    VarDeclType m_decl_type;
    string m_name;

  public:
    constexpr VarDecl(auto name, auto type, auto value, auto decl_type,
                      auto attributes)
        : Stmt(QAST_VAR),
          m_attributes(attributes),
          m_type(type),
          m_value(value),
          m_decl_type(decl_type),
          m_name(name) {}

    [[nodiscard]] constexpr auto GetName() const { return m_name; }
    [[nodiscard]] constexpr auto GetType() const { return m_type; }
    [[nodiscard]] constexpr auto GetValue() const { return m_value; }
    [[nodiscard]] constexpr auto GetDeclType() const { return m_decl_type; }
    [[nodiscard]] constexpr auto GetAttributes() const { return m_attributes; }
  };

  class InlineAsm final : public Stmt {
    std::span<FlowPtr<Expr>> m_args;
    string m_code;

  public:
    constexpr InlineAsm(auto code, auto args)
        : Stmt(QAST_INLINE_ASM), m_args(args), m_code(code) {}

    [[nodiscard]] constexpr auto GetCode() const { return m_code; }
    [[nodiscard]] constexpr auto GetArgs() const { return m_args; }
  };

  class IfStmt final : public Stmt {
    FlowPtr<Expr> m_cond;
    FlowPtr<Stmt> m_then;
    NullableFlowPtr<Stmt> m_else;

  public:
    constexpr IfStmt(auto cond, auto then, auto ele)
        : Stmt(QAST_IF), m_cond(cond), m_then(then), m_else(ele) {}

    [[nodiscard]] constexpr auto GetCond() const { return m_cond; }
    [[nodiscard]] constexpr auto GetThen() const { return m_then; }
    [[nodiscard]] constexpr auto GetElse() const { return m_else; }
  };

  class WhileStmt final : public Stmt {
    FlowPtr<Expr> m_cond;
    FlowPtr<Stmt> m_body;

  public:
    constexpr WhileStmt(auto cond, auto body)
        : Stmt(QAST_WHILE), m_cond(cond), m_body(body) {}

    [[nodiscard]] constexpr auto GetCond() const { return m_cond; }
    [[nodiscard]] constexpr auto GetBody() const { return m_body; }
  };

  class ForStmt final : public Stmt {
    NullableFlowPtr<Stmt> m_init;
    NullableFlowPtr<Expr> m_cond, m_step;
    FlowPtr<Stmt> m_body;

  public:
    constexpr ForStmt(auto init, auto cond, auto step, auto body)
        : Stmt(QAST_FOR),
          m_init(init),
          m_cond(cond),
          m_step(step),
          m_body(body) {}

    [[nodiscard]] constexpr auto GetInit() const { return m_init; }
    [[nodiscard]] constexpr auto GetCond() const { return m_cond; }
    [[nodiscard]] constexpr auto GetStep() const { return m_step; }
    [[nodiscard]] constexpr auto GetBody() const { return m_body; }
  };

  class ForeachStmt final : public Stmt {
    FlowPtr<Expr> m_expr;
    FlowPtr<Stmt> m_body;
    string m_idx_ident, m_val_ident;

  public:
    constexpr ForeachStmt(auto idx_ident, auto val_ident, auto expr, auto body)
        : Stmt(QAST_FOREACH),
          m_expr(expr),
          m_body(body),
          m_idx_ident(idx_ident),
          m_val_ident(val_ident) {}

    [[nodiscard]] constexpr auto GetIdxIdent() const { return m_idx_ident; }
    [[nodiscard]] constexpr auto GetValIdent() const { return m_val_ident; }
    [[nodiscard]] constexpr auto GetExpr() const { return m_expr; }
    [[nodiscard]] constexpr auto GetBody() const { return m_body; }
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
    constexpr ReturnStmt(auto value) : Stmt(QAST_RETURN), m_value(value) {}

    [[nodiscard]] constexpr auto GetValue() const { return m_value; }
  };

  class ReturnIfStmt final : public Stmt {
    FlowPtr<Expr> m_cond, m_value;

  public:
    constexpr ReturnIfStmt(auto cond, auto value)
        : Stmt(QAST_RETIF), m_cond(cond), m_value(value) {}

    [[nodiscard]] constexpr auto GetCond() const { return m_cond; }
    [[nodiscard]] constexpr auto GetValue() const { return m_value; }
  };

  class CaseStmt final : public Stmt {
    FlowPtr<Expr> m_cond;
    FlowPtr<Stmt> m_body;

  public:
    constexpr CaseStmt(auto cond, auto body)
        : Stmt(QAST_CASE), m_cond(cond), m_body(body) {}

    [[nodiscard]] constexpr auto GetCond() const { return m_cond; }
    [[nodiscard]] constexpr auto GetBody() const { return m_body; }
  };

  class SwitchStmt final : public Stmt {
    std::span<FlowPtr<CaseStmt>> m_cases;
    FlowPtr<Expr> m_cond;
    NullableFlowPtr<Stmt> m_default;

  public:
    constexpr SwitchStmt(auto cond, auto cases, auto def)
        : Stmt(QAST_SWITCH), m_cases(cases), m_cond(cond), m_default(def) {}

    [[nodiscard]] constexpr auto GetCond() const { return m_cond; }
    [[nodiscard]] constexpr auto GetCases() const { return m_cases; }
    [[nodiscard]] constexpr auto GetDefault() const { return m_default; }
  };

  class ExportStmt final : public Stmt {
    std::span<FlowPtr<Expr>> m_attrs;
    FlowPtr<Stmt> m_body;
    string m_abi_name;
    Vis m_vis;

  public:
    constexpr ExportStmt(auto content, auto abi_name, auto vis, auto attrs)
        : Stmt(QAST_EXPORT),
          m_attrs(attrs),
          m_body(content),
          m_abi_name(abi_name),
          m_vis(vis) {}

    [[nodiscard]] constexpr auto GetAbiName() const { return m_abi_name; }
    [[nodiscard]] constexpr auto GetBody() const { return m_body; }
    [[nodiscard]] constexpr auto GetVis() const { return m_vis; }
    [[nodiscard]] constexpr auto GetAttrs() const { return m_attrs; }
  };

  class ScopeStmt final : public Stmt {
    std::span<string> m_deps;
    FlowPtr<Stmt> m_body;
    string m_name;

  public:
    constexpr ScopeStmt(auto name, auto body, auto deps)
        : Stmt(QAST_SCOPE), m_deps(deps), m_body(body), m_name(name) {}

    [[nodiscard]] constexpr auto GetName() const { return m_name; }
    [[nodiscard]] constexpr auto GetBody() const { return m_body; }
    [[nodiscard]] constexpr auto GetDeps() const { return m_deps; }
  };

  class TypedefStmt final : public Stmt {
    FlowPtr<Type> m_type;
    string m_name;

  public:
    constexpr TypedefStmt(auto name, auto type)
        : Stmt(QAST_TYPEDEF), m_type(type), m_name(name) {}

    [[nodiscard]] constexpr auto GetName() const { return m_name; }
    [[nodiscard]] constexpr auto GetType() const { return m_type; }
  };

  class EnumDef final : public Stmt {
    std::span<EnumItem> m_items;
    NullableFlowPtr<Type> m_type;
    string m_name;

  public:
    constexpr EnumDef(auto name, auto type, auto items)
        : Stmt(QAST_ENUM), m_items(items), m_type(type), m_name(name) {}

    [[nodiscard]] constexpr auto GetName() const { return m_name; }
    [[nodiscard]] constexpr auto GetItems() const { return m_items; }
    [[nodiscard]] constexpr auto GetType() const { return m_type; }
  };

  class Function final : public Stmt {
    std::optional<std::span<TemplateParameter>> m_template_parameters;
    std::span<FlowPtr<Expr>> m_attributes;
    std::span<std::pair<string, bool>> m_captures;
    std::span<FuncParam> m_params;
    FlowPtr<Type> m_return;
    NullableFlowPtr<Expr> m_precond, m_postcond;
    NullableFlowPtr<Stmt> m_body;
    string m_name;
    Purity m_purity;
    bool m_variadic;

  public:
    constexpr Function(auto attributes, auto purity, auto captures, auto name,
                       auto params, auto fn_params, auto variadic,
                       auto return_type, auto precond, auto postcond, auto body)
        : Stmt(QAST_FUNCTION),
          m_attributes(attributes),
          m_captures(captures),
          m_params(fn_params),
          m_return(return_type),
          m_precond(precond),
          m_postcond(postcond),
          m_body(body),
          m_name(name),
          m_purity(purity),
          m_variadic(variadic) {
      if (params.has_value()) {
        m_template_parameters = params.value();
      }
    }

    [[nodiscard]] constexpr auto GetName() const { return m_name; }
    [[nodiscard]] constexpr auto GetAttributes() const { return m_attributes; }
    [[nodiscard]] constexpr auto GetPurity() const { return m_purity; }
    [[nodiscard]] constexpr auto GetCaptures() const { return m_captures; }
    [[nodiscard]] constexpr auto GetTemplateParams() const {
      return m_template_parameters;
    }
    [[nodiscard]] constexpr auto GetParams() const { return m_params; }
    [[nodiscard]] constexpr auto GetReturn() const { return m_return; }
    [[nodiscard]] constexpr auto GetPrecond() const { return m_precond; }
    [[nodiscard]] constexpr auto GetPostcond() const { return m_postcond; }
    [[nodiscard]] constexpr auto GetBody() const { return m_body; }

    [[nodiscard]] constexpr auto IsVariadic() const { return m_variadic; }
    [[nodiscard]] constexpr bool IsDeclaration() const {
      return !m_body.has_value();
    }
    [[nodiscard]] constexpr bool IsDefinition() const {
      return m_body.has_value();
    }

    [[nodiscard]] constexpr bool HasContract() const {
      return m_precond.has_value() || m_postcond.has_value();
    }
  };

  class StructDef final : public Stmt {
    std::span<FlowPtr<Expr>> m_attributes;
    std::optional<std::span<TemplateParameter>> m_template_parameters;
    std::span<string> m_names;
    std::span<StructField> m_fields;
    std::span<StructFunction> m_methods;
    std::span<StructFunction> m_static_methods;
    CompositeType m_comp_type;
    string m_name;

  public:
    constexpr StructDef(auto comp_type, auto attributes, auto name, auto params,
                        auto names, auto fields, auto methods,
                        auto static_methods)
        : Stmt(QAST_STRUCT),
          m_attributes(attributes),
          m_names(names),
          m_fields(fields),
          m_methods(methods),
          m_static_methods(static_methods),
          m_comp_type(comp_type),
          m_name(name) {
      if (params.has_value()) {
        m_template_parameters = params.value();
      }
    }

    [[nodiscard]] constexpr auto GetName() const { return m_name; }
    [[nodiscard]] constexpr auto GetCompositeType() const {
      return m_comp_type;
    }
    [[nodiscard]] constexpr auto GetAttributes() const { return m_attributes; }
    [[nodiscard]] constexpr auto GetTemplateParams() const {
      return m_template_parameters;
    }
    [[nodiscard]] constexpr auto GetNames() const { return m_names; }
    [[nodiscard]] constexpr auto GetFields() const { return m_fields; }
    [[nodiscard]] constexpr auto GetMethods() const { return m_methods; }
    [[nodiscard]] constexpr auto GetStaticMethods() const {
      return m_static_methods;
    }
  };

  constexpr bool Stmt::IsExprStmt(npar_ty_t type) const {
    return is(QAST_ESTMT) && as<ExprStmt>()->GetExpr()->is(type);
  }
}  // namespace ncc::parse

#endif
