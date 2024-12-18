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

#include "nitrate-core/String.hh"
#include "nitrate-parser/ASTData.hh"

namespace npar {
  class npar_pack ExprStmt final : public Stmt {
    Expr *m_expr;

  public:
    constexpr ExprStmt(Expr *expr) : Stmt(QAST_ESTMT), m_expr(expr) {}

    auto get_expr() const { return m_expr; }
  };

  class Block final : public Stmt {
    std::span<Stmt *> m_items;
    SafetyMode m_safety;

  public:
    Block(BlockItems items, SafetyMode safety)
        : Stmt(QAST_BLOCK), m_items(items), m_safety(safety) {}

    auto get_items() const { return m_items; }
    auto get_safety() const { return m_safety; }
  };

  class VarDecl final : public Stmt {
    std::span<Expr *> m_attributes;
    std::string_view m_name;
    Type *m_type;
    Expr *m_value;
    VarDeclType m_decl_type;

  public:
    VarDecl(ncc::core::str_alias name, Type *type, Expr *value,
            VarDeclType decl_type, ExpressionList attributes)
        : Stmt(QAST_VAR),
          m_attributes(attributes),
          m_name(name.get()),
          m_type(type),
          m_value(value),
          m_decl_type(decl_type) {}

    auto get_name() const { return m_name; }
    auto get_type() const { return m_type; }
    auto get_value() const { return m_value; }
    auto get_decl_type() const { return m_decl_type; }
    auto get_attributes() const { return m_attributes; }
  };

  class InlineAsm final : public Stmt {
    std::string_view m_code;
    std::span<Expr *> m_args;

  public:
    InlineAsm(ncc::core::str_alias code, ExpressionList args)
        : Stmt(QAST_INLINE_ASM), m_code(code.get()), m_args(args) {}

    auto get_code() const { return m_code; }
    auto get_args() const { return m_args; }
  };

  class npar_pack IfStmt final : public Stmt {
    Expr *m_cond;
    Stmt *m_then, *m_else;

  public:
    constexpr IfStmt(Expr *cond, Stmt *then, Stmt *else_)
        : Stmt(QAST_IF), m_cond(cond), m_then(then), m_else(else_) {}

    auto get_cond() const { return m_cond; }
    auto get_then() const { return m_then; }
    auto get_else() const { return m_else; }
  };

  class WhileStmt final : public Stmt {
    Expr *m_cond;
    Stmt *m_body;

  public:
    constexpr WhileStmt(Expr *cond, Stmt *body)
        : Stmt(QAST_WHILE), m_cond(cond), m_body(body) {}

    auto get_cond() const { return m_cond; }
    auto get_body() const { return m_body; }
  };

  class ForStmt final : public Stmt {
    std::optional<Stmt *> m_init;
    std::optional<Expr *> m_cond, m_step;
    Stmt *m_body;

  public:
    constexpr ForStmt(std::optional<Stmt *> init, std::optional<Expr *> cond,
                      std::optional<Expr *> step, Stmt *body)
        : Stmt(QAST_FOR),
          m_init(init),
          m_cond(cond),
          m_step(step),
          m_body(body) {}

    auto get_init() const { return m_init; }
    auto get_cond() const { return m_cond; }
    auto get_step() const { return m_step; }
    auto get_body() const { return m_body; }
  };

  class ForeachStmt final : public Stmt {
    std::string_view m_idx_ident;
    std::string_view m_val_ident;
    Expr *m_expr;
    Stmt *m_body;

  public:
    ForeachStmt(ncc::core::str_alias idx_ident, ncc::core::str_alias val_ident,
                Expr *expr, Stmt *body)
        : Stmt(QAST_FOREACH),
          m_idx_ident(idx_ident.get()),
          m_val_ident(val_ident.get()),
          m_expr(expr),
          m_body(body) {}

    auto get_idx_ident() const { return m_idx_ident; }
    auto get_val_ident() const { return m_val_ident; }
    auto get_expr() const { return m_expr; }
    auto get_body() const { return m_body; }
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
    std::optional<Expr *> m_value;

  public:
    constexpr ReturnStmt(std::optional<Expr *> value)
        : Stmt(QAST_RETURN), m_value(value) {}

    auto get_value() const { return m_value; }
  };

  class ReturnIfStmt final : public Stmt {
    Expr *m_cond;
    Expr *m_value;

  public:
    constexpr ReturnIfStmt(Expr *cond, Expr *value)
        : Stmt(QAST_RETIF), m_cond(cond), m_value(value) {}

    auto get_cond() const { return m_cond; }
    auto get_value() const { return m_value; }
  };

  class CaseStmt final : public Stmt {
    Expr *m_cond;
    Stmt *m_body;

  public:
    constexpr CaseStmt(Expr *cond, Stmt *body)
        : Stmt(QAST_CASE), m_cond(cond), m_body(body) {}

    auto get_cond() const { return m_cond; }
    auto get_body() const { return m_body; }
  };

  class SwitchStmt final : public Stmt {
    Expr *m_cond;
    std::span<CaseStmt *> m_cases;
    Stmt *m_default;

  public:
    SwitchStmt(Expr *cond, SwitchCases cases, Stmt *default_)
        : Stmt(QAST_SWITCH),
          m_cond(cond),
          m_cases(cases),
          m_default(default_) {}

    auto get_cond() const { return m_cond; }
    auto get_cases() const { return m_cases; }
    auto get_default() const { return m_default; }
  };

  class ExportStmt final : public Stmt {
    std::span<Expr *> m_attrs;
    std::string_view m_abi_name;
    Stmt *m_body;
    Vis m_vis;

  public:
    ExportStmt(Stmt *content, ncc::core::str_alias abi_name, Vis vis,
               ExpressionList attrs)
        : Stmt(QAST_EXPORT),
          m_attrs(attrs),
          m_abi_name(abi_name.get()),
          m_body(content),
          m_vis(vis) {}

    auto get_body() const { return m_body; }
    auto get_abi_name() const { return m_abi_name; }
    auto get_vis() const { return m_vis; }
    auto get_attrs() const { return m_attrs; }
  };

  class ScopeStmt final : public Stmt {
    ScopeDeps m_deps;
    std::string_view m_name;
    Stmt *m_body;

  public:
    ScopeStmt(ncc::core::str_alias name, Stmt *body, ScopeDeps deps = {})
        : Stmt(QAST_SCOPE), m_deps(deps), m_name(name.get()), m_body(body) {}

    auto get_name() const { return m_name; }
    auto get_body() const { return m_body; }
    let get_deps() const { return m_deps; }
  };

  class TypedefStmt final : public Stmt {
    std::string_view m_name;
    Type *m_type;

  public:
    TypedefStmt(ncc::core::str_alias name, Type *type)
        : Stmt(QAST_TYPEDEF), m_name(name.get()), m_type(type) {}

    auto get_name() const { return m_name; }
    auto get_type() const { return m_type; }
  };

  class EnumDef final : public Stmt {
    std::span<EnumItem> m_items;
    std::string_view m_name;
    Type *m_type;

  public:
    EnumDef(ncc::core::str_alias name, Type *type, EnumDefItems items)
        : Stmt(QAST_ENUM), m_items(items), m_name(name.get()), m_type(type) {}

    auto get_items() const { return m_items; }
    auto get_name() const { return m_name; }
    auto get_type() const { return m_type; }
  };

  class Function final : public Stmt {
    std::span<Expr *> m_attributes;
    FuncPurity m_purity;
    std::span<std::pair<ncc::core::str_alias, bool>> m_captures;
    std::string_view m_name;
    std::optional<TemplateParameters> m_template_parameters;
    FuncParams m_params;
    Type *m_return;
    std::optional<Expr *> m_precond, m_postcond;
    std::optional<Stmt *> m_body;

  public:
    Function(ExpressionList attributes, FuncPurity purity, FnCaptures captures,
             ncc::core::str_alias name,
             std::optional<TemplateParameters> params, FuncParams fn_params,
             Type *return_type, std::optional<Expr *> precond,
             std::optional<Expr *> postcond, std::optional<Stmt *> body)
        : Stmt(QAST_FUNCTION),
          m_attributes(attributes),
          m_purity(purity),
          m_captures(captures),
          m_name(name.get()),
          m_template_parameters(params),
          m_params(fn_params),
          m_return(return_type),
          m_precond(precond),
          m_postcond(postcond),
          m_body(body) {}

    auto get_attributes() const { return m_attributes; }
    auto get_purity() const { return m_purity; }
    auto get_captures() const { return m_captures; }
    auto get_name() const { return m_name; }
    let get_template_params() const { return m_template_parameters; }
    let get_params() const { return m_params; }
    auto get_return() const { return m_return; }
    auto get_precond() const { return m_precond; }
    auto get_postcond() const { return m_postcond; }
    auto get_body() const { return m_body; }

    bool is_decl() const { return !m_body.has_value(); }
    bool is_def() const { return m_body.has_value(); }
  };

  class StructDef final : public Stmt {
    CompositeType m_comp_type;
    std::span<Expr *> m_attributes;
    std::string_view m_name;
    std::optional<TemplateParameters> m_template_parameters;
    std::span<ncc::core::str_alias> m_names;
    std::span<StructField> m_fields;
    std::span<StructFunction> m_methods;
    std::span<StructFunction> m_static_methods;

  public:
    StructDef(CompositeType comp_type, ExpressionList attributes,
              ncc::core::str_alias name,
              std::optional<TemplateParameters> params, StructDefNames names,
              StructDefFields fields, StructDefMethods methods,
              StructDefStaticMethods static_methods)
        : Stmt(QAST_STRUCT),
          m_comp_type(comp_type),
          m_attributes(attributes),
          m_name(name.get()),
          m_template_parameters(params),
          m_names(names),
          m_fields(fields),
          m_methods(methods),
          m_static_methods(static_methods) {}

    auto get_composite_type() const { return m_comp_type; }
    auto get_attributes() const { return m_attributes; }
    auto get_name() const { return m_name; }
    let get_template_params() const { return m_template_parameters; }
    auto get_names() const { return m_names; }
    auto get_fields() const { return m_fields; }
    auto get_methods() const { return m_methods; }
    auto get_static_methods() const { return m_static_methods; }
  };

  constexpr bool Stmt::is_expr_stmt(npar_ty_t type) const {
    return is(QAST_ESTMT) && as<npar::ExprStmt>()->get_expr()->is(type);
  }
}  // namespace npar

#endif
