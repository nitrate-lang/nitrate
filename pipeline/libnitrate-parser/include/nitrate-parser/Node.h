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

#ifndef __NITRATE_PARSER_NODE_H__
#define __NITRATE_PARSER_NODE_H__

#ifndef __cplusplus
#error "This code requires c++"
#endif

#include <nitrate-core/Macro.h>
#include <nitrate-lexer/Token.h>

#include <nitrate-parser/ASTBase.hh>
#include <nitrate-parser/ASTData.hh>
#include <nitrate-parser/ASTTypes.hh>
#include <optional>

namespace npar {
  class Stmt : public npar_node_t {
  public:
    constexpr Stmt(npar_ty_t ty) : npar_node_t(ty){};

    constexpr bool is_expr_stmt(npar_ty_t type) const;
  };

  class Type : public npar_node_t {
    std::pair<Expr *, Expr *> m_range;
    Expr *m_width;

  public:
    constexpr Type(npar_ty_t ty)
        : npar_node_t(ty), m_range({nullptr, nullptr}), m_width(nullptr) {}

    constexpr bool is_primitive() const {
      switch (getKind()) {
        case QAST_U1:
        case QAST_U8:
        case QAST_U16:
        case QAST_U32:
        case QAST_U64:
        case QAST_U128:
        case QAST_I8:
        case QAST_I16:
        case QAST_I32:
        case QAST_I64:
        case QAST_I128:
        case QAST_F16:
        case QAST_F32:
        case QAST_F64:
        case QAST_F128:
        case QAST_VOID:
          return true;
        default:
          return false;
      }
    }
    constexpr bool is_array() const { return getKind() == QAST_ARRAY; };
    constexpr bool is_tuple() const { return getKind() == QAST_TUPLE; }
    constexpr bool is_pointer() const { return getKind() == QAST_PTR; }
    constexpr bool is_function() const { return getKind() == QAST_FUNCTOR; }
    constexpr bool is_composite() const { return is_array() || is_tuple(); }
    constexpr bool is_numeric() const {
      return getKind() >= QAST_U1 && getKind() <= QAST_F128;
    }
    constexpr bool is_integral() const {
      return getKind() >= QAST_U1 && getKind() <= QAST_I128;
    }
    constexpr bool is_floating_point() const {
      return getKind() >= QAST_F16 && getKind() <= QAST_F128;
    }
    constexpr bool is_signed() const {
      return getKind() >= QAST_I8 && getKind() <= QAST_I128;
    }
    constexpr bool is_unsigned() const {
      return getKind() >= QAST_U1 && getKind() <= QAST_U128;
    }
    constexpr bool is_void() const { return getKind() == QAST_VOID; }
    constexpr bool is_bool() const { return getKind() == QAST_U1; }
    constexpr bool is_ref() const { return getKind() == QAST_REF; }
    bool is_ptr_to(Type *type) const;

    constexpr let get_width() const { return m_width; }
    constexpr let get_range() const { return m_range; }

    constexpr void set_range(Expr *start, Expr *end) { m_range = {start, end}; }
    constexpr void set_width(Expr *width) { m_width = width; }
  };

  class Expr : public npar_node_t {
  public:
    constexpr Expr(npar_ty_t ty) : npar_node_t(ty) {}

    constexpr bool is_stmt_expr(npar_ty_t type) const;
  };

  class ExprStmt : public Stmt {
    Expr *m_expr;

  public:
    constexpr ExprStmt(Expr *expr) : Stmt(QAST_ESTMT), m_expr(expr) {}

    let get_expr() const { return m_expr; }
  };

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

  class NamedTy : public Type {
    SmallString m_name;

  public:
    NamedTy(SmallString name) : Type(QAST_NAMED), m_name(name) {}

    let get_name() const { return m_name; }
  };

  class InferTy : public Type {
  public:
    constexpr InferTy() : Type(QAST_INFER) {}
  };

  class TemplType : public Type {
    Type *m_template;
    ExpressionList m_args;

  public:
    TemplType(Type *templ, const ExpressionList &args)
        : Type(QAST_TEMPLATE), m_template(templ), m_args(args) {}

    let get_template() const { return m_template; }
    let get_args() const { return m_args; }
  };

  class U1 : public Type {
  public:
    constexpr U1() : Type(QAST_U1){};
  };

  class U8 : public Type {
  public:
    constexpr U8() : Type(QAST_U8){};
  };

  class U16 : public Type {
  public:
    constexpr U16() : Type(QAST_U16){};
  };

  class U32 : public Type {
  public:
    constexpr U32() : Type(QAST_U32){};
  };

  class U64 : public Type {
  public:
    constexpr U64() : Type(QAST_U64){};
  };

  class U128 : public Type {
  public:
    constexpr U128() : Type(QAST_U128){};
  };

  class I8 : public Type {
  public:
    constexpr I8() : Type(QAST_I8){};
  };

  class I16 : public Type {
  public:
    constexpr I16() : Type(QAST_I16){};
  };

  class I32 : public Type {
  public:
    constexpr I32() : Type(QAST_I32){};
  };

  class I64 : public Type {
  public:
    constexpr I64() : Type(QAST_I64){};
  };

  class I128 : public Type {
  public:
    constexpr I128() : Type(QAST_I128){};
  };

  class F16 : public Type {
  public:
    constexpr F16() : Type(QAST_F16){};
  };

  class F32 : public Type {
  public:
    constexpr F32() : Type(QAST_F32){};
  };

  class F64 : public Type {
  public:
    constexpr F64() : Type(QAST_F64){};
  };

  class F128 : public Type {
  public:
    constexpr F128() : Type(QAST_F128){};
  };

  class VoidTy : public Type {
  public:
    constexpr VoidTy() : Type(QAST_VOID) {}
  };

  class PtrTy : public Type {
    Type *m_item;
    bool m_is_volatile;

  public:
    constexpr PtrTy(Type *item, bool is_volatile = false)
        : Type(QAST_PTR), m_item(item), m_is_volatile(is_volatile) {}

    let get_item() const { return m_item; }
    bool is_volatile() const { return m_is_volatile; }
  };

  class OpaqueTy : public Type {
    SmallString m_name;

  public:
    OpaqueTy(SmallString name) : Type(QAST_OPAQUE), m_name(name) {}

    let get_name() const { return m_name; }
  };

  class TupleTy : public Type {
    TupleTyItems m_items;

  public:
    TupleTy(const TupleTyItems &items) : Type(QAST_TUPLE), m_items(items) {}

    let get_items() const { return m_items; }
  };

  class ArrayTy : public Type {
    Type *m_item;
    Expr *m_size;

  public:
    constexpr ArrayTy(Type *item, Expr *size)
        : Type(QAST_ARRAY), m_item(item), m_size(size) {}

    let get_item() const { return m_item; }
    let get_size() const { return m_size; }
  };

  class RefTy : public Type {
    Type *m_item;

  public:
    constexpr RefTy(Type *item) : Type(QAST_REF), m_item(item) {}

    let get_item() const { return m_item; }
  };

  class FuncTy : public Type {
    FuncParams m_params;
    Type *m_return;
    FuncPurity m_purity;
    bool m_variadic;
    bool m_is_foreign;
    bool m_noreturn;

  public:
    FuncTy()
        : Type(QAST_FUNCTOR),
          m_return(nullptr),
          m_purity(FuncPurity::IMPURE_THREAD_UNSAFE),
          m_variadic(false),
          m_is_foreign(false),
          m_noreturn(false) {}

    FuncTy(Type *return_type, FuncParams parameters, bool variadic = false,
           FuncPurity purity = FuncPurity::IMPURE_THREAD_UNSAFE,
           bool is_foreign = false, bool noreturn = false)
        : Type(QAST_FUNCTOR),
          m_params(parameters),
          m_return(return_type),
          m_purity(purity),
          m_variadic(variadic),
          m_is_foreign(is_foreign),
          m_noreturn(noreturn) {
      assert(!noreturn || (purity == FuncPurity::IMPURE_THREAD_UNSAFE ||
                           purity == FuncPurity::IMPURE_THREAD_SAFE));
    }
    FuncTy(Type *return_type, std::vector<Type *, Arena<Type *>> parameters,
           bool variadic = false,
           FuncPurity purity = FuncPurity::IMPURE_THREAD_UNSAFE,
           bool is_foreign = false, bool noreturn = false)
        : Type(QAST_FUNCTOR),
          m_return(return_type),
          m_purity(purity),
          m_variadic(variadic),
          m_is_foreign(is_foreign),
          m_noreturn(noreturn) {
      assert(!noreturn || (purity == FuncPurity::IMPURE_THREAD_UNSAFE ||
                           purity == FuncPurity::IMPURE_THREAD_SAFE));

      for (size_t i = 0; i < parameters.size(); i++) {
        m_params.push_back(
            FuncParam("_" + std::to_string(i), parameters[i], nullptr));
      }
    }

    let is_noreturn() const { return m_noreturn; }
    let get_return_ty() const { return m_return; }
    let get_params() const { return m_params; }
    let get_purity() const { return m_purity; }
    let is_variadic() const { return m_variadic; }
    let is_foreign() const { return m_is_foreign; }

    void set_return_ty(Type *return_ty) { m_return = return_ty; }
    void set_purity(FuncPurity purity) { m_purity = purity; }
    void set_variadic(bool variadic) { m_variadic = variadic; }
    void set_foreign(bool is_foreign) { m_is_foreign = is_foreign; }
    auto &get_params() { return m_params; }
  };

  ///=============================================================================

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

  ///=============================================================================

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

  ///=============================================================================

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

  ///=============================================================================

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
    VarDeclAttributes m_attributes;
    SmallString m_name;
    Type *m_type;
    Expr *m_value;
    VarDeclType m_decl_type;

  public:
    VarDecl(SmallString name, Type *type, Expr *value, VarDeclType decl_type,
            VarDeclAttributes attributes)
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
    SymbolAttributes m_attrs;
    SmallString m_abi_name;
    Stmt *m_body;
    Vis m_vis;

  public:
    ExportStmt(Stmt *content, SmallString abi_name, Vis vis,
               SymbolAttributes attrs)
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

  class StructField : public Stmt {
    SmallString m_name;
    Type *m_type;
    Expr *m_value;
    Vis m_visibility;

  public:
    StructField(SmallString name, Type *type, Expr *value, Vis visibility)
        : Stmt(QAST_STRUCT_FIELD),
          m_name(name),
          m_type(type),
          m_value(value),
          m_visibility(visibility) {}

    let get_name() const { return m_name; }
    let get_type() const { return m_type; }
    let get_value() const { return m_value; }
    let get_visibility() const { return m_visibility; }

    void set_visibility(Vis visibility) { m_visibility = visibility; }
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

  class FnDef : public Stmt {
    FnCaptures m_captures;
    SmallString m_name;
    std::optional<TemplateParameters> m_template_parameters;
    FuncTy *m_type;
    Expr *m_precond;
    Expr *m_postcond;
    std::optional<Stmt *> m_body;

  public:
    FnDef(SmallString name, FuncTy *type, const FnCaptures &captures = {},
          std::optional<TemplateParameters> params = std::nullopt,
          Expr *precond = nullptr, Expr *postcond = nullptr,
          std::optional<Stmt *> body = std::nullopt)
        : Stmt(QAST_FUNCTION),
          m_captures(captures),
          m_name(name),
          m_template_parameters(params),
          m_type(type),
          m_precond(precond),
          m_postcond(postcond),
          m_body(body) {}

    let get_captures() const { return m_captures; }
    let get_name() const { return m_name; }
    let get_body() const { return m_body; }
    let get_precond() const { return m_precond; }
    let get_postcond() const { return m_postcond; }
    let get_type() const { return m_type; }
    let get_template_params() const { return m_template_parameters; }

    bool is_decl() const { return !m_body.has_value(); }
    bool is_def() const { return m_body.has_value(); }

    void set_body(std::optional<Stmt *> body) { m_body = body; }
    void set_precond(Expr *precond) { m_precond = precond; }
    void set_postcond(Expr *postcond) { m_postcond = postcond; }
    void set_name(SmallString name) { m_name = name; }
    void set_type(FuncTy *type) { m_type = type; }
    void set_captures(FnCaptures captures) { m_captures = captures; }

    std::optional<TemplateParameters> &get_template_params() {
      return m_template_parameters;
    }
  };

  class StructDef : public Stmt {
    StructDefMethods m_methods;
    StructDefStaticMethods m_static_methods;
    StructDefFields m_fields;
    std::optional<TemplateParameters> m_template_parameters;
    CompositeType m_comp_type;
    SmallString m_name;

  public:
    StructDef(SmallString name, const StructDefFields &fields = {},
              const StructDefMethods &methods = {},
              const StructDefStaticMethods &static_methods = {},
              std::optional<TemplateParameters> params = std::nullopt,
              CompositeType t = CompositeType::Struct)
        : Stmt(QAST_STRUCT),
          m_methods(methods),
          m_static_methods(static_methods),
          m_fields(fields),
          m_template_parameters(params),
          m_comp_type(t),
          m_name(name) {}

    let get_name() const { return m_name; }
    let get_methods() const { return m_methods; }
    let get_static_methods() const { return m_static_methods; }
    let get_fields() const { return m_fields; }
    let get_composite_type() const { return m_comp_type; }
    let get_template_params() const { return m_template_parameters; }

    void set_name(SmallString name) { m_name = name; }
    void set_composite_type(CompositeType t) { m_comp_type = t; }
    auto &get_methods() { return m_methods; }
    auto &get_static_methods() { return m_static_methods; }
    auto &get_fields() { return m_fields; }
    auto &get_template_params() { return m_template_parameters; }
  };

  constexpr bool Stmt::is_expr_stmt(npar_ty_t type) const {
    return is(QAST_ESTMT) && as<npar::ExprStmt>()->get_expr()->is(type);
  }

  constexpr bool Expr::is_stmt_expr(npar_ty_t type) const {
    return is(QAST_SEXPR) && as<npar::StmtExpr>()->get_stmt()->is(type);
  }

  template <typename T, typename... Args>
  static inline T *make(Args &&...args) {
    T *new_obj = new (Arena<T>().allocate(1)) T(std::forward<Args>(args)...);

    //  /// TODO: Cache nodes

    return new_obj;
  }

  ///=============================================================================

  Stmt *mock_stmt(npar_ty_t expected);
  Expr *mock_expr(npar_ty_t expected);
  Type *mock_type();
}  // namespace npar

#endif  // __NITRATE_PARSER_NODE_H__
