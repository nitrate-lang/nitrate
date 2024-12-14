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

namespace npar {
  class ExprStmt : public Stmt {
    Expr *m_expr;

  public:
    constexpr ExprStmt(Expr *expr) : Stmt(QAST_ESTMT), m_expr(expr) {}

    let get_expr() const { return m_expr; }
  };

  class Block : public Stmt {
    BlockItems m_items;
    SafetyMode m_safety;

  public:
    Block(const BlockItems &items = {}, SafetyMode safety = SafetyMode::Unknown)
        : Stmt(QAST_BLOCK), m_items(items), m_safety(safety) {}

    let get_items() const { return m_items; }
    let get_safety() const { return m_safety; }

    void set_safety(SafetyMode safety) { m_safety = safety; }
    auto &get_items() { return m_items; }
  };

  class VarDecl : public Stmt {
    ExpressionList m_attributes;
    SmallString m_name;
    Type *m_type;
    Expr *m_value;
    VarDeclType m_decl_type;

  public:
    VarDecl(SmallString name, Type *type, Expr *value, VarDeclType decl_type,
            ExpressionList attributes)
        : Stmt(QAST_VAR),
          m_attributes(attributes),
          m_name(name),
          m_type(type),
          m_value(value),
          m_decl_type(decl_type) {}

    let get_name() const { return m_name; }
    let get_type() const { return m_type; }
    let get_value() const { return m_value; }
    let get_decl_type() const { return m_decl_type; }
    let get_attributes() const { return m_attributes; }
  };

  class InlineAsm : public Stmt {
    SmallString m_code;
    ExpressionList m_args;

  public:
    InlineAsm(SmallString code, const ExpressionList &args)
        : Stmt(QAST_INLINE_ASM), m_code(code), m_args(args) {}

    let get_code() const { return m_code; }
    let get_args() const { return m_args; }
  };

  class IfStmt : public Stmt {
    Expr *m_cond;
    Stmt *m_then;
    Stmt *m_else;

  public:
    constexpr IfStmt(Expr *cond, Stmt *then, Stmt *else_)
        : Stmt(QAST_IF), m_cond(cond), m_then(then), m_else(else_) {}

    let get_cond() const { return m_cond; }
    let get_then() const { return m_then; }
    let get_else() const { return m_else; }
  };

  class WhileStmt : public Stmt {
    Expr *m_cond;
    Stmt *m_body;

  public:
    constexpr WhileStmt(Expr *cond, Stmt *body)
        : Stmt(QAST_WHILE), m_cond(cond), m_body(body) {}

    let get_cond() const { return m_cond; }
    let get_body() const { return m_body; }
  };

  class ForStmt : public Stmt {
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

    let get_init() const { return m_init; }
    let get_cond() const { return m_cond; }
    let get_step() const { return m_step; }
    let get_body() const { return m_body; }
  };

  class ForeachStmt : public Stmt {
    SmallString m_idx_ident;
    SmallString m_val_ident;
    Expr *m_expr;
    Stmt *m_body;

  public:
    ForeachStmt(SmallString idx_ident, SmallString val_ident, Expr *expr,
                Stmt *body)
        : Stmt(QAST_FOREACH),
          m_idx_ident(idx_ident),
          m_val_ident(val_ident),
          m_expr(expr),
          m_body(body) {}

    let get_idx_ident() const { return m_idx_ident; }
    let get_val_ident() const { return m_val_ident; }
    let get_expr() const { return m_expr; }
    let get_body() const { return m_body; }
  };

  class BreakStmt : public Stmt {
  public:
    constexpr BreakStmt() : Stmt(QAST_BREAK){};
  };

  class ContinueStmt : public Stmt {
  public:
    constexpr ContinueStmt() : Stmt(QAST_CONTINUE){};
  };

  class ReturnStmt : public Stmt {
    std::optional<Expr *> m_value;

  public:
    constexpr ReturnStmt(std::optional<Expr *> value)
        : Stmt(QAST_RETURN), m_value(value) {}

    let get_value() const { return m_value; }
  };

  class ReturnIfStmt : public Stmt {
    Expr *m_cond;
    Expr *m_value;

  public:
    constexpr ReturnIfStmt(Expr *cond, Expr *value)
        : Stmt(QAST_RETIF), m_cond(cond), m_value(value) {}

    let get_cond() const { return m_cond; }
    let get_value() const { return m_value; }
  };

  class CaseStmt : public Stmt {
    Expr *m_cond;
    Stmt *m_body;

  public:
    constexpr CaseStmt(Expr *cond, Stmt *body)
        : Stmt(QAST_CASE), m_cond(cond), m_body(body) {}

    let get_cond() const { return m_cond; }
    let get_body() const { return m_body; }
  };

  class SwitchStmt : public Stmt {
    Expr *m_cond;
    SwitchCases m_cases;
    Stmt *m_default;

  public:
    SwitchStmt(Expr *cond, const SwitchCases &cases, Stmt *default_)
        : Stmt(QAST_SWITCH),
          m_cond(cond),
          m_cases(cases),
          m_default(default_) {}

    let get_cond() const { return m_cond; }
    let get_cases() const { return m_cases; }
    let get_default() const { return m_default; }
  };

  class ExportStmt : public Stmt {
    ExpressionList m_attrs;
    SmallString m_abi_name;
    Stmt *m_body;
    Vis m_vis;

  public:
    ExportStmt(Stmt *content, SmallString abi_name, Vis vis,
               ExpressionList attrs)
        : Stmt(QAST_EXPORT),
          m_attrs(attrs),
          m_abi_name(abi_name),
          m_body(content),
          m_vis(vis) {}

    let get_body() const { return m_body; }
    let get_abi_name() const { return m_abi_name; }
    let get_vis() const { return m_vis; }
    let get_attrs() const { return m_attrs; }
  };

  class ScopeStmt : public Stmt {
    ScopeDeps m_deps;
    SmallString m_name;
    Stmt *m_body;

  public:
    ScopeStmt(SmallString name, Stmt *body, ScopeDeps deps = {})
        : Stmt(QAST_SCOPE), m_deps(deps), m_name(name), m_body(body) {}

    let get_name() const { return m_name; }
    let get_body() const { return m_body; }
    let get_deps() const { return m_deps; }
  };

  class TypedefStmt : public Stmt {
    SmallString m_name;
    Type *m_type;

  public:
    TypedefStmt(SmallString name, Type *type)
        : Stmt(QAST_TYPEDEF), m_name(name), m_type(type) {}

    let get_name() const { return m_name; }
    let get_type() const { return m_type; }
  };

  class EnumDef : public Stmt {
    EnumDefItems m_items;
    SmallString m_name;
    Type *m_type;

  public:
    EnumDef(SmallString name, Type *type, const EnumDefItems &items)
        : Stmt(QAST_ENUM), m_items(items), m_name(name), m_type(type) {}

    let get_items() const { return m_items; }
    let get_name() const { return m_name; }
    let get_type() const { return m_type; }
  };

  class Function : public Stmt {
    ExpressionList m_attributes;
    FuncPurity m_purity;
    FnCaptures m_captures;
    SmallString m_name;
    std::optional<TemplateParameters> m_template_parameters;
    FuncParams m_params;
    Type *m_return;
    std::optional<Expr *> m_precond, m_postcond;
    std::optional<Stmt *> m_body;

  public:
    Function(ExpressionList attributes, FuncPurity purity, FnCaptures captures,
             SmallString name, std::optional<TemplateParameters> params,
             FuncParams fn_params, Type *return_type,
             std::optional<Expr *> precond, std::optional<Expr *> postcond,
             std::optional<Stmt *> body)
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

    let get_attributes() const { return m_attributes; }
    let get_purity() const { return m_purity; }
    let get_captures() const { return m_captures; }
    let get_name() const { return m_name; }
    let get_template_params() const { return m_template_parameters; }
    let get_params() const { return m_params; }
    let get_return() const { return m_return; }
    let get_precond() const { return m_precond; }
    let get_postcond() const { return m_postcond; }
    let get_body() const { return m_body; }

    bool is_decl() const { return !m_body.has_value(); }
    bool is_def() const { return m_body.has_value(); }
  };

  class StructDef : public Stmt {
    CompositeType m_comp_type;
    ExpressionList m_attributes;
    SmallString m_name;
    std::optional<TemplateParameters> m_template_parameters;
    StructDefNames m_names;
    StructDefFields m_fields;
    StructDefMethods m_methods;
    StructDefStaticMethods m_static_methods;

  public:
    StructDef(CompositeType comp_type, ExpressionList attributes,
              SmallString name, std::optional<TemplateParameters> params,
              const StructDefNames &names, const StructDefFields &fields,
              const StructDefMethods &methods,
              const StructDefStaticMethods &static_methods)
        : Stmt(QAST_STRUCT),
          m_comp_type(comp_type),
          m_attributes(attributes),
          m_name(name),
          m_template_parameters(params),
          m_names(names),
          m_fields(fields),
          m_methods(methods),
          m_static_methods(static_methods) {}

    let get_composite_type() const { return m_comp_type; }
    let get_attributes() const { return m_attributes; }
    let get_name() const { return m_name; }
    let get_template_params() const { return m_template_parameters; }
    let get_names() const { return m_names; }
    let get_fields() const { return m_fields; }
    let get_methods() const { return m_methods; }
    let get_static_methods() const { return m_static_methods; }
  };

  constexpr bool Stmt::is_expr_stmt(npar_ty_t type) const {
    return is(QAST_ESTMT) && as<npar::ExprStmt>()->get_expr()->is(type);
  }
}  // namespace npar

#endif
