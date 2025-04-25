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

#ifndef __NITRATE_AST_EXPR_H__
#define __NITRATE_AST_EXPR_H__

#include <nitrate-lexer/Enums.hh>
#include <nitrate-parser/ASTBase.hh>
#include <nitrate-parser/ASTData.hh>
#include <span>

namespace ncc::parse {
  class Unary final : public Expr {
    FlowPtr<Expr> m_rhs;
    lex::Operator m_op;
    bool m_is_postfix;

  public:
    constexpr Unary(auto op, auto rhs, bool is_postfix)
        : Expr(AST_eUNARY), m_rhs(std::move(rhs)), m_op(op), m_is_postfix(is_postfix) {}

    [[nodiscard, gnu::pure]] constexpr auto GetRHS() const { return m_rhs; }
    [[nodiscard, gnu::pure]] constexpr auto GetRHS() { return m_rhs; }
    [[nodiscard, gnu::pure]] constexpr auto GetOp() const { return m_op; }
    [[nodiscard, gnu::pure]] constexpr auto GetOp() { return m_op; }
    [[nodiscard, gnu::pure]] constexpr auto IsPostfix() const { return m_is_postfix; }
    [[nodiscard, gnu::pure]] constexpr auto IsPostfix() { return m_is_postfix; }
  };

  class Binary final : public Expr {
    FlowPtr<Expr> m_lhs, m_rhs;
    lex::Operator m_op;

  public:
    constexpr Binary(auto lhs, auto op, auto rhs)
        : Expr(AST_eBIN), m_lhs(std::move(lhs)), m_rhs(std::move(rhs)), m_op(op) {}

    [[nodiscard, gnu::pure]] constexpr auto GetLHS() const { return m_lhs; }
    [[nodiscard, gnu::pure]] constexpr auto GetLHS() { return m_lhs; }
    [[nodiscard, gnu::pure]] constexpr auto GetRHS() const { return m_rhs; }
    [[nodiscard, gnu::pure]] constexpr auto GetRHS() { return m_rhs; }
    [[nodiscard, gnu::pure]] constexpr auto GetOp() const { return m_op; }
    [[nodiscard, gnu::pure]] constexpr auto GetOp() { return m_op; }
  };

  class Integer final : public Expr {
    string m_value;

  public:
    constexpr Integer(auto value) : Expr(AST_eINT), m_value(value) {}

    [[nodiscard, gnu::pure]] constexpr auto GetValue() const { return m_value; }
    [[nodiscard, gnu::pure]] constexpr auto GetValue() { return m_value; }
  };

  class Float final : public Expr {
    string m_value;

  public:
    constexpr Float(auto value) : Expr(AST_eFLOAT), m_value(value) {}

    [[nodiscard, gnu::pure]] constexpr auto GetValue() const { return m_value; }
    [[nodiscard, gnu::pure]] constexpr auto GetValue() { return m_value; }
  };

  class Boolean final : public Expr {
    bool m_value;

  public:
    constexpr Boolean(auto value) : Expr(AST_eBOOL), m_value(value) {}

    [[nodiscard, gnu::pure]] constexpr auto GetValue() const { return m_value; }
    [[nodiscard, gnu::pure]] constexpr auto GetValue() { return m_value; }
  };

  class String final : public Expr {
    string m_value;

  public:
    constexpr String(auto value) : Expr(AST_eSTRING), m_value(value) {}

    [[nodiscard, gnu::pure]] constexpr auto GetValue() const { return m_value; }
    [[nodiscard, gnu::pure]] constexpr auto GetValue() { return m_value; }
  };

  class Character final : public Expr {
    uint8_t m_value;

  public:
    constexpr Character(auto value) : Expr(AST_eCHAR), m_value(value) {}

    [[nodiscard, gnu::pure]] constexpr auto GetValue() const { return m_value; }
    [[nodiscard, gnu::pure]] constexpr auto GetValue() { return m_value; }
  };

  class Call final : public Expr {
    FlowPtr<Expr> m_func;
    std::span<CallArg> m_args;

  public:
    constexpr Call(auto func, auto args) : Expr(AST_eCALL), m_func(std::move(func)), m_args(args) {}

    [[nodiscard, gnu::pure]] constexpr auto GetFunc() const { return m_func; }
    [[nodiscard, gnu::pure]] constexpr auto GetFunc() { return m_func; }
    [[nodiscard, gnu::pure]] constexpr auto GetArgs() const { return m_args; }
    [[nodiscard, gnu::pure]] constexpr auto GetArgs() { return m_args; }
  };

  class TemplateCall final : public Expr {
    FlowPtr<Expr> m_func;
    std::span<CallArg> m_template_args, m_args;

  public:
    constexpr TemplateCall(auto func, auto args, auto template_args)
        : Expr(AST_eTEMPLATE_CALL), m_func(std::move(func)), m_template_args(template_args), m_args(args) {}

    [[nodiscard, gnu::pure]] constexpr auto GetFunc() const { return m_func; }
    [[nodiscard, gnu::pure]] constexpr auto GetFunc() { return m_func; }
    [[nodiscard, gnu::pure]] constexpr auto GetTemplateArgs() const { return m_template_args; }
    [[nodiscard, gnu::pure]] constexpr auto GetTemplateArgs() { return m_template_args; }
    [[nodiscard, gnu::pure]] constexpr auto GetArgs() const { return m_args; }
    [[nodiscard, gnu::pure]] constexpr auto GetArgs() { return m_args; }
  };

  class Import final : public Expr {
    FlowPtr<Expr> m_subtree;
    string m_name;
    ImportMode m_mode;

  public:
    constexpr Import(auto name, auto mode, auto subtree)
        : Expr(AST_eIMPORT), m_subtree(std::move(subtree)), m_name(name), m_mode(mode) {}

    [[nodiscard, gnu::pure]] constexpr auto GetSubtree() const { return m_subtree; }
    [[nodiscard, gnu::pure]] constexpr auto GetSubtree() { return m_subtree; }
    [[nodiscard, gnu::pure]] constexpr auto GetName() const { return m_name; }
    [[nodiscard, gnu::pure]] constexpr auto GetName() { return m_name; }
    [[nodiscard, gnu::pure]] constexpr auto GetMode() const { return m_mode; }
    [[nodiscard, gnu::pure]] constexpr auto GetMode() { return m_mode; }
  };

  class List final : public Expr {
    std::span<FlowPtr<Expr>> m_items;

  public:
    constexpr List(auto items) : Expr(AST_eLIST), m_items(items) {}

    [[nodiscard, gnu::pure]] constexpr auto GetItems() const { return m_items; }
    [[nodiscard, gnu::pure]] constexpr auto GetItems() { return m_items; }
  };

  class Assoc final : public Expr {
    FlowPtr<Expr> m_key, m_value;

  public:
    constexpr Assoc(auto key, auto value) : Expr(AST_ePAIR), m_key(std::move(key)), m_value(std::move(value)) {}

    [[nodiscard, gnu::pure]] constexpr auto GetKey() const { return m_key; }
    [[nodiscard, gnu::pure]] constexpr auto GetKey() { return m_key; }
    [[nodiscard, gnu::pure]] constexpr auto GetValue() const { return m_value; }
    [[nodiscard, gnu::pure]] constexpr auto GetValue() { return m_value; }
  };

  class Index final : public Expr {
    FlowPtr<Expr> m_base, m_index;

  public:
    constexpr Index(auto base, auto index) : Expr(AST_eINDEX), m_base(std::move(base)), m_index(std::move(index)) {}

    [[nodiscard, gnu::pure]] constexpr auto GetBase() const { return m_base; }
    [[nodiscard, gnu::pure]] constexpr auto GetBase() { return m_base; }
    [[nodiscard, gnu::pure]] constexpr auto GetIndex() const { return m_index; }
    [[nodiscard, gnu::pure]] constexpr auto GetIndex() { return m_index; }
  };

  class Slice final : public Expr {
    FlowPtr<Expr> m_base, m_start, m_end;

  public:
    constexpr Slice(auto base, auto start, auto end)
        : Expr(AST_eSLICE), m_base(std::move(base)), m_start(std::move(start)), m_end(std::move(end)) {}

    [[nodiscard, gnu::pure]] constexpr auto GetBase() const { return m_base; }
    [[nodiscard, gnu::pure]] constexpr auto GetBase() { return m_base; }
    [[nodiscard, gnu::pure]] constexpr auto GetStart() const { return m_start; }
    [[nodiscard, gnu::pure]] constexpr auto GetStart() { return m_start; }
    [[nodiscard, gnu::pure]] constexpr auto GetEnd() const { return m_end; }
    [[nodiscard, gnu::pure]] constexpr auto GetEnd() { return m_end; }
  };

  class FString final : public Expr {
    std::span<std::variant<string, FlowPtr<Expr>>> m_items;

  public:
    constexpr FString(auto items) : Expr(AST_eFSTRING), m_items(items) {}

    [[nodiscard, gnu::pure]] constexpr auto GetItems() const { return m_items; }
    [[nodiscard, gnu::pure]] constexpr auto GetItems() { return m_items; }
  };

  class Identifier final : public Expr {
    string m_name;

  public:
    constexpr Identifier(auto name) : Expr(AST_eIDENT), m_name(name) {
      qcore_assert(!m_name->empty(), "Identifier name cannot be empty");
    }

    [[nodiscard, gnu::pure]] constexpr auto GetName() const { return m_name; }
    [[nodiscard, gnu::pure]] constexpr auto GetName() { return m_name; }
  };
}  // namespace ncc::parse

#endif
