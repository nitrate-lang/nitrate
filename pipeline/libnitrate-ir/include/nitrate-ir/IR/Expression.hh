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

#ifndef __NITRATE_IR_GRAPH_EXPR_H__
#define __NITRATE_IR_GRAPH_EXPR_H__

#include <nitrate-ir/IR/Base.hh>
#include <nitrate-lexer/Token.hh>

#include "nitrate-core/FlowPtr.hh"

namespace ncc::ir {
  template <class A>
  class IR_Vertex_BinExpr final : public IR_Vertex_Expr<A> {
    friend A;

    FlowPtr<IR_Vertex_Expr<A>> m_lhs, m_rhs;
    lex::Operator m_op;

  public:
    constexpr IR_Vertex_BinExpr(auto lhs, auto rhs, auto op)
        : IR_Vertex_Expr<A>(IR_eBIN), m_lhs(lhs), m_rhs(rhs), m_op(op) {}

    constexpr auto getLHS() const { return m_lhs; }
    constexpr auto getRHS() const { return m_rhs; }
    constexpr auto getOp() const { return m_op; }

    constexpr void SetLHS(auto lhs) { m_lhs = lhs; }
    constexpr void SetRHS(auto rhs) { m_rhs = rhs; }
    constexpr void SetOp(auto op) { m_op = op; }
  };

  template <class A>
  class IR_Vertex_Unary final : public IR_Vertex_Expr<A> {
    friend A;

    FlowPtr<IR_Vertex_Expr<A>> m_expr;
    lex::Operator m_op;
    bool m_postfix;

  public:
    constexpr IR_Vertex_Unary(auto expr, auto op, auto is_postfix)
        : IR_Vertex_Expr<A>(IR_eUNARY),
          m_expr(expr),
          m_op(op),
          m_postfix(is_postfix) {}

    constexpr auto getExpr() const { return m_expr; }
    constexpr auto getOp() const { return m_op; }
    constexpr auto isPostfix() const { return m_postfix; }

    constexpr void SetExpr(auto expr) { m_expr = expr; }
    constexpr void SetOp(auto op) { m_op = op; }
    constexpr void SetPostfix(auto is_postfix) { m_postfix = is_postfix; }
  };

  template <class A>
  class IR_Vertex_Int final : public IR_Vertex_Expr<A> {
    friend A;

    uint128_t m_value;
    uint8_t m_size;

    static constexpr uint128_t str2u128(std::string_view s) {
      uint128_t x = 0;

      for (char c : s) {
        if (!std::isdigit(c)) [[unlikely]] {
          qcore_panicf("Failed to convert string `%s` to uint128_t", s.data());
        }

        // Check for overflow
        if (x > (std::numeric_limits<uint128_t>::max() - (c - '0')) / 10)
            [[unlikely]] {
          qcore_panicf("Overflow when converting string `%s` to uint128_t",
                       s.data());
        }

        x = x * 10 + (c - '0');
      }

      return x;
    }

  public:
    constexpr IR_Vertex_Int(auto val, auto size)
        : IR_Vertex_Expr<A>(IR_eINT), m_value(val), m_size(size) {}

    constexpr IR_Vertex_Int(std::string_view str, uint8_t size)
        : IR_Vertex_Expr<A>(IR_eINT), m_value(str2u128(str)) {
      m_size = size;
    }

    constexpr auto getSize() const { return m_size; }
    constexpr uint128_t getValue() const { return m_value; }
    std::string getValueString() const { return m_value.str(); }
  };

  template <class A>
  class IR_Vertex_Float final : public IR_Vertex_Expr<A> {
    friend A;

    double m_data;
    uint8_t m_size;

    static_assert(sizeof(double) == 8);

  public:
    constexpr IR_Vertex_Float(auto dec, auto size)
        : IR_Vertex_Expr<A>(IR_eFLOAT), m_data{dec}, m_size(size) {}

    constexpr IR_Vertex_Float(string str) : IR_Vertex_Expr<A>(IR_eFLOAT) {
      m_data = std::stod(std::string(str));
      if (str->ends_with("f128")) {
        m_size = 128;
      } else if (str->ends_with("f32")) {
        m_size = 32;
      } else if (str->ends_with("f16")) {
        m_size = 16;
      } else {
        m_size = 64;
      }
    }

    constexpr auto getSize() const { return m_size; }
    constexpr auto getValue() const { return m_data; }
  };

  template <class A>
  class IR_Vertex_List final : public IR_Vertex_Expr<A> {
    friend A;

    std::span<FlowPtr<IR_Vertex_Expr<A>>> m_items;
    bool m_is_homogenous;

  public:
    constexpr IR_Vertex_List(auto items, auto is_homogenous)
        : IR_Vertex_Expr<A>(IR_eLIST),
          m_items(items),
          m_is_homogenous(is_homogenous) {}

    constexpr auto begin() const { return m_items.begin(); }
    constexpr auto end() const { return m_items.end(); }
    constexpr auto size() const { return m_items.size(); }
    constexpr auto empty() const { return m_items.empty(); }

    constexpr auto operator[](size_t idx) const { return m_items[idx]; }
    constexpr auto at(size_t idx) const { return m_items[idx]; }
    constexpr bool isHomogenous() const { return m_is_homogenous; }
  };

  template <class A>
  class IR_Vertex_Call final : public IR_Vertex_Expr<A> {
    friend A;

    std::span<FlowPtr<IR_Vertex_Expr<A>>> m_args;
    /* Possibly cyclic reference to the callee. */
    NullableFlowPtr<IR_Vertex_Expr<A>> m_iref;

  public:
    constexpr IR_Vertex_Call(auto ref, auto args)
        : IR_Vertex_Expr<A>(IR_eCALL), m_args(args), m_iref(ref) {}

    constexpr auto getTarget() const { return m_iref; }
    constexpr auto getNumArgs() { return m_args.size(); }
    constexpr auto getArgs() const { return m_args; }

    constexpr void SetTarget(auto ref) { m_iref = ref; }
    void setArgs(auto args) { m_args = args; }
  };

  template <class A>
  class IR_Vertex_Seq final : public IR_Vertex_Expr<A> {
    friend A;

    std::span<FlowPtr<IR_Vertex_Expr<A>>> m_items;

  public:
    constexpr IR_Vertex_Seq(auto items)
        : IR_Vertex_Expr<A>(IR_eSEQ), m_items(items) {}

    constexpr auto getItems() const { return m_items; }
    constexpr auto begin() const { return m_items.begin(); }
    constexpr auto end() const { return m_items.end(); }
    constexpr auto size() const { return m_items.size(); }
    constexpr auto empty() const { return m_items.empty(); }

    constexpr void SetItems(auto items) { m_items = items; }
  };

  template <class A>
  class IR_Vertex_Index final : public IR_Vertex_Expr<A> {
    friend A;

    FlowPtr<IR_Vertex_Expr<A>> m_expr, m_index;

  public:
    constexpr IR_Vertex_Index(auto expr, auto index)
        : IR_Vertex_Expr<A>(IR_eINDEX), m_expr(expr), m_index(index) {}

    constexpr auto getExpr() const { return m_expr; }
    constexpr auto getIndex() const { return m_index; }

    constexpr void SetIndex(auto index) { m_index = index; }
    constexpr void SetExpr(auto expr) { m_expr = expr; }
  };

  template <class A>
  class IR_Vertex_Ident final : public IR_Vertex_Expr<A> {
    friend A;

    NullableFlowPtr<IR_Vertex_Expr<A>> m_what;
    string m_name;

  public:
    constexpr IR_Vertex_Ident(auto name, auto what)
        : IR_Vertex_Expr<A>(IR_eIDENT), m_what(what), m_name(name) {}

    constexpr auto getWhat() const { return m_what; }
    constexpr auto getName() const { return m_name.Get(); }

    constexpr void SetWhat(auto what) { m_what = what; }
    constexpr void SetName(auto name) { m_name = name; }
  };

  template <class A>
  class IR_Vertex_Extern final : public IR_Vertex_Expr<A> {
    friend A;

    FlowPtr<IR_Vertex_Expr<A>> m_value;
    string m_abi_name;

  public:
    constexpr IR_Vertex_Extern(auto value, auto abi_name)
        : IR_Vertex_Expr<A>(IR_eEXTERN), m_value(value), m_abi_name(abi_name) {}

    constexpr auto getAbiName() const { return m_abi_name.Get(); }
    constexpr auto getValue() const { return m_value; }

    constexpr void SetValue(auto value) { m_value = value; }
    constexpr void SetAbiName(auto abi_name) { m_abi_name = abi_name; }
  };

  template <class A>
  class IR_Vertex_Local final : public IR_Vertex_Expr<A> {
    friend A;

    FlowPtr<IR_Vertex_Expr<A>> m_value;
    string m_name, m_abi_name;
    StorageClass m_storage;
    bool m_readonly;

  public:
    constexpr IR_Vertex_Local(
        auto name, auto value, auto abi_name, auto readonly = false,
        auto storage_class = StorageClass::LLVM_StackAlloa)
        : IR_Vertex_Expr<A>(IR_eLOCAL),
          m_value(value),
          m_name(name),
          m_abi_name(abi_name),
          m_storage(storage_class),
          m_readonly(readonly) {}

    constexpr auto getName() const { return m_name.Get(); }
    constexpr auto getValue() const { return m_value; }
    constexpr auto getAbiName() const { return m_abi_name; }
    constexpr auto getStorageClass() const { return m_storage; }
    constexpr auto isReadonly() const { return m_readonly; }

    constexpr void SetAbiName(auto abi_name) { m_abi_name = abi_name; }
    constexpr void SetValue(auto value) { m_value = value; }
    constexpr void SetName(auto name) { m_name = name; }
    constexpr void SetStorageClass(auto storage) { m_storage = storage; }
    constexpr void SetReadonly(auto readonly) { m_readonly = readonly; }
  };

  template <class A>
  class IR_Vertex_Ret final : public IR_Vertex_Expr<A> {
    friend A;

    FlowPtr<IR_Vertex_Expr<A>> m_expr;

  public:
    constexpr IR_Vertex_Ret(auto expr)
        : IR_Vertex_Expr<A>(IR_eRET), m_expr(expr) {}

    constexpr auto getExpr() const { return m_expr; }
    constexpr void SetExpr(auto expr) { m_expr = expr; }
  };

  template <class A>
  class IR_Vertex_Brk final : public IR_Vertex_Expr<A> {
    friend A;

  public:
    constexpr IR_Vertex_Brk() : IR_Vertex_Expr<A>(IR_eBRK) {}
  };

  template <class A>
  class IR_Vertex_Cont final : public IR_Vertex_Expr<A> {
    friend A;

  public:
    constexpr IR_Vertex_Cont() : IR_Vertex_Expr<A>(IR_eSKIP) {}
  };

  template <class A>
  class IR_Vertex_If final : public IR_Vertex_Expr<A> {
    friend A;

    FlowPtr<IR_Vertex_Expr<A>> m_cond, m_then, m_else;

  public:
    constexpr IR_Vertex_If(auto cond, auto then, auto ele)
        : IR_Vertex_Expr<A>(IR_eIF), m_cond(cond), m_then(then), m_else(ele) {}

    constexpr auto getCond() const { return m_cond; }
    constexpr auto getThen() const { return m_then; }
    constexpr auto getElse() const { return m_else; }

    constexpr void SetCond(auto cond) { m_cond = cond; }
    constexpr void SetThen(auto then) { m_then = then; }
    constexpr void SetElse(auto ele) { m_else = ele; }
  };

  template <class A>
  class IR_Vertex_While final : public IR_Vertex_Expr<A> {
    friend A;

    FlowPtr<IR_Vertex_Expr<A>> m_cond;
    FlowPtr<IR_Vertex_Seq<A>> m_body;

  public:
    constexpr IR_Vertex_While(auto cond, auto body)
        : IR_Vertex_Expr<A>(IR_eWHILE), m_cond(cond), m_body(body) {}

    constexpr auto getCond() const { return m_cond; }
    constexpr auto getBody() const { return m_body; }

    constexpr void SetCond(auto cond) { m_cond = cond; }
    constexpr void SetBody(auto body) { m_body = body; }
  };

  template <class A>
  class IR_Vertex_For final : public IR_Vertex_Expr<A> {
    friend A;

    FlowPtr<IR_Vertex_Expr<A>> m_init, m_cond, m_step, m_body;

  public:
    constexpr IR_Vertex_For(auto init, auto cond, auto step, auto body)
        : IR_Vertex_Expr<A>(IR_eFOR),
          m_init(init),
          m_cond(cond),
          m_step(step),
          m_body(body) {}

    constexpr auto getInit() const { return m_init; }
    constexpr auto getCond() const { return m_cond; }
    constexpr auto getStep() const { return m_step; }
    constexpr auto getBody() const { return m_body; }

    constexpr void SetInit(auto init) { m_init = init; }
    constexpr void SetCond(auto cond) { m_cond = cond; }
    constexpr void SetStep(auto step) { m_step = step; }
    constexpr void SetBody(auto body) { m_body = body; }
  };

  template <class A>
  class IR_Vertex_Case final : public IR_Vertex_Expr<A> {
    friend A;

    FlowPtr<IR_Vertex_Expr<A>> m_cond, m_body;

  public:
    constexpr IR_Vertex_Case(auto cond, auto body)
        : IR_Vertex_Expr<A>(IR_eCASE), m_cond(cond), m_body(body) {}

    constexpr auto getCond() { return m_cond; }
    constexpr auto getBody() { return m_body; }

    constexpr void SetCond(auto cond) { m_cond = cond; }
    constexpr void SetBody(auto body) { m_body = body; }
  };

  template <class A>
  class IR_Vertex_Switch final : public IR_Vertex_Expr<A> {
    friend A;

    FlowPtr<IR_Vertex_Expr<A>> m_cond;
    NullableFlowPtr<IR_Vertex_Expr<A>> m_default;
    std::span<FlowPtr<IR_Vertex_Case<A>>> m_cases;

  public:
    constexpr IR_Vertex_Switch(auto cond, auto cases, auto default_)
        : IR_Vertex_Expr<A>(IR_eSWITCH),
          m_cond(cond),
          m_default(default_),
          m_cases(cases) {}

    constexpr auto getCond() const { return m_cond; }
    constexpr auto getCases() const { return m_cases; }
    constexpr auto getDefault() const { return m_default; }

    constexpr void SetCond(auto cond) { m_cond = cond; }
    constexpr void SetDefault(auto default_) { m_default = default_; }
    constexpr void SetCases(auto cases) { m_cases = cases; }
  };

  template <class A>
  class IR_Vertex_Function final : public IR_Vertex_Expr<A> {
    friend A;

    std::span<std::pair<FlowPtr<IR_Vertex_Type<A>>, string>> m_params;
    NullableFlowPtr<IR_Vertex_Seq<A>> m_body;
    FlowPtr<IR_Vertex_Type<A>> m_return;
    string m_name, m_abi_name;
    bool m_variadic;

  public:
    constexpr IR_Vertex_Function(auto name, auto params, auto ret_ty, auto body,
                                 auto variadic, auto abi_name)
        : IR_Vertex_Expr<A>(IR_eFUNCTION),
          m_params(params),
          m_body(body),
          m_return(ret_ty),
          m_name(name),
          m_abi_name(abi_name),
          m_variadic(variadic) {}

    constexpr auto getParams() const { return m_params; }
    constexpr auto getReturn() const { return m_return; }
    constexpr auto getBody() const { return m_body; }
    constexpr auto isVariadic() const { return m_variadic; }
    constexpr auto getAbiName() const { return m_abi_name; }
    constexpr auto getName() const { return m_name.Get(); }

    constexpr void SetName(auto name) { m_name = name; }
    constexpr void SetParams(auto params) { m_params = params; }
    constexpr void SetReturn(auto ret_ty) { m_return = ret_ty; }
    constexpr void SetBody(auto body) { m_body = body; }
    constexpr void SetVariadic(auto variadic) { m_variadic = variadic; }
    constexpr void SetAbiName(auto abi_name) { m_abi_name = abi_name; }
  };

  template <class A>
  class IR_Vertex_Asm final : public IR_Vertex_Expr<A> {
    friend A;

  public:
    constexpr IR_Vertex_Asm() : IR_Vertex_Expr<A>(IR_eASM) {}
  };

}  // namespace ncc::ir

#endif
