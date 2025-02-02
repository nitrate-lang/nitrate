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

namespace ncc::ir {
  template <class A>
  class GenericBinary final : public GenericExpr<A> {
    friend A;

    FlowPtr<GenericExpr<A>> m_lhs, m_rhs;
    lex::Operator m_op;

  public:
    constexpr GenericBinary(auto lhs, auto rhs, auto op)
        : GenericExpr<A>(IR_eBIN), m_lhs(lhs), m_rhs(rhs), m_op(op) {}

    [[nodiscard]] constexpr auto GetLHS() const { return m_lhs; }
    [[nodiscard]] constexpr auto GetRHS() const { return m_rhs; }
    [[nodiscard]] constexpr auto GetOp() const { return m_op; }

    constexpr void SetLHS(auto lhs) { m_lhs = lhs; }
    constexpr void SetRHS(auto rhs) { m_rhs = rhs; }
    constexpr void SetOp(auto op) { m_op = op; }
  };

  template <class A>
  class GenericUnary final : public GenericExpr<A> {
    friend A;

    FlowPtr<GenericExpr<A>> m_expr;
    lex::Operator m_op;
    bool m_postfix;

  public:
    constexpr GenericUnary(auto expr, auto op, auto is_postfix)
        : GenericExpr<A>(IR_eUNARY),
          m_expr(expr),
          m_op(op),
          m_postfix(is_postfix) {}

    [[nodiscard]] constexpr auto GetExpr() const { return m_expr; }
    [[nodiscard]] constexpr auto GetOp() const { return m_op; }
    [[nodiscard]] constexpr auto IsPostfix() const { return m_postfix; }

    constexpr void SetExpr(auto expr) { m_expr = expr; }
    constexpr void SetOp(auto op) { m_op = op; }
    constexpr void SetPostfix(auto is_postfix) { m_postfix = is_postfix; }
  };

  template <class A>
  class GenericInt final : public GenericExpr<A> {
    friend A;

    uint128_t m_value;
    uint8_t m_size;

    static constexpr auto Str2u128(std::string_view s) -> uint128_t {
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
    constexpr GenericInt(auto val, auto size)
        : GenericExpr<A>(IR_eINT), m_value(val), m_size(size) {}

    constexpr GenericInt(std::string_view str, uint8_t size)
        : GenericExpr<A>(IR_eINT), m_value(Str2u128(str)) {
      m_size = size;
    }

    [[nodiscard]] constexpr auto GetSize() const { return m_size; }
    [[nodiscard]] constexpr auto GetValue() const -> uint128_t {
      return m_value;
    }
    [[nodiscard]] auto GetValueString() const -> std::string {
      return m_value.str();
    }
  };

  template <class A>
  class GenericFloat final : public GenericExpr<A> {
    friend A;

    double m_data;
    uint8_t m_size;

    static_assert(sizeof(double) == 8);

  public:
    constexpr GenericFloat(auto dec, auto size)
        : GenericExpr<A>(IR_eFLOAT), m_data{dec}, m_size(size) {}

    constexpr GenericFloat(string str) : GenericExpr<A>(IR_eFLOAT) {
      m_data = std::stod(std::string(str));
      if (str.ends_with("f128")) {
        m_size = 128;
      } else if (str.ends_with("f32")) {
        m_size = 32;
      } else if (str.ends_with("f16")) {
        m_size = 16;
      } else {
        m_size = 64;
      }
    }

    [[nodiscard]] constexpr auto GetSize() const { return m_size; }
    [[nodiscard]] constexpr auto GetValue() const { return m_data; }
  };

  template <class A>
  class GenericList final : public GenericExpr<A> {
    friend A;

    std::span<FlowPtr<GenericExpr<A>>> m_items;
    bool m_is_homogenous;

  public:
    constexpr GenericList(auto items, auto is_homogenous)
        : GenericExpr<A>(IR_eLIST),
          m_items(items),
          m_is_homogenous(is_homogenous) {}

    [[nodiscard]] constexpr auto Begin() const { return m_items.begin(); }
    [[nodiscard]] constexpr auto End() const { return m_items.end(); }
    [[nodiscard]] constexpr auto Size() const { return m_items.size(); }
    [[nodiscard]] constexpr auto Empty() const { return m_items.empty(); }

    constexpr auto operator[](size_t idx) const { return m_items[idx]; }
    [[nodiscard]] constexpr auto At(size_t idx) const { return m_items[idx]; }
    [[nodiscard]] constexpr auto IsHomogenous() const -> bool {
      return m_is_homogenous;
    }
  };

  template <class A>
  class GenericCall final : public GenericExpr<A> {
    friend A;

    std::span<FlowPtr<GenericExpr<A>>> m_args;
    /* Possibly cyclic reference to the callee. */
    NullableFlowPtr<GenericExpr<A>> m_iref;

  public:
    constexpr GenericCall(auto ref, auto args)
        : GenericExpr<A>(IR_eCALL), m_args(args), m_iref(ref) {}

    [[nodiscard]] constexpr auto GetTarget() const { return m_iref; }
    constexpr auto GetNumArgs() { return m_args.size(); }
    [[nodiscard]] constexpr auto GetArgs() const { return m_args; }

    constexpr void SetTarget(auto ref) { m_iref = ref; }
    void SetArgs(auto args) { m_args = args; }
  };

  template <class A>
  class GenericSeq final : public GenericExpr<A> {
    friend A;

    std::span<FlowPtr<GenericExpr<A>>> m_items;

  public:
    constexpr GenericSeq(auto items)
        : GenericExpr<A>(IR_eSEQ), m_items(items) {}

    [[nodiscard]] constexpr auto GetItems() const { return m_items; }
    [[nodiscard]] constexpr auto Begin() const { return m_items.begin(); }
    [[nodiscard]] constexpr auto End() const { return m_items.end(); }
    [[nodiscard]] constexpr auto Size() const { return m_items.size(); }
    [[nodiscard]] constexpr auto Empty() const { return m_items.empty(); }

    constexpr void SetItems(auto items) { m_items = items; }
  };

  template <class A>
  class GenericIndex final : public GenericExpr<A> {
    friend A;

    FlowPtr<GenericExpr<A>> m_expr, m_index;

  public:
    constexpr GenericIndex(auto expr, auto index)
        : GenericExpr<A>(IR_eINDEX), m_expr(expr), m_index(index) {}

    [[nodiscard]] constexpr auto GetExpr() const { return m_expr; }
    [[nodiscard]] constexpr auto GetIndex() const { return m_index; }

    constexpr void SetIndex(auto index) { m_index = index; }
    constexpr void SetExpr(auto expr) { m_expr = expr; }
  };

  template <class A>
  class GenericIdentifier final : public GenericExpr<A> {
    friend A;

    NullableFlowPtr<GenericExpr<A>> m_what;
    string m_name;

  public:
    constexpr GenericIdentifier(auto name, auto what)
        : GenericExpr<A>(IR_eIDENT), m_what(what), m_name(name) {}

    [[nodiscard]] constexpr auto GetWhat() const { return m_what; }
    [[nodiscard]] constexpr auto GetName() const { return m_name.Get(); }

    constexpr void SetWhat(auto what) { m_what = what; }
    constexpr void SetName(auto name) { m_name = name; }
  };

  template <class A>
  class GenericExtern final : public GenericExpr<A> {
    friend A;

    FlowPtr<GenericExpr<A>> m_value;
    string m_abi_name;

  public:
    constexpr GenericExtern(auto value, auto abi_name)
        : GenericExpr<A>(IR_eEXTERN), m_value(value), m_abi_name(abi_name) {}

    [[nodiscard]] constexpr auto GetAbiName() const { return m_abi_name.Get(); }
    [[nodiscard]] constexpr auto GetValue() const { return m_value; }

    constexpr void SetValue(auto value) { m_value = value; }
    constexpr void SetAbiName(auto abi_name) { m_abi_name = abi_name; }
  };

  template <class A>
  class GenericLocal final : public GenericExpr<A> {
    friend A;

    FlowPtr<GenericExpr<A>> m_value;
    string m_name, m_abi_name;
    StorageClass m_storage;
    bool m_readonly;

  public:
    constexpr GenericLocal(auto name, auto value, auto abi_name,
                           auto readonly = false,
                           auto storage_class = StorageClass::LLVM_StackAlloa)
        : GenericExpr<A>(IR_eLOCAL),
          m_value(value),
          m_name(name),
          m_abi_name(abi_name),
          m_storage(storage_class),
          m_readonly(readonly) {}

    [[nodiscard]] constexpr auto GetName() const { return m_name.Get(); }
    [[nodiscard]] constexpr auto GetValue() const { return m_value; }
    [[nodiscard]] constexpr auto GetAbiName() const { return m_abi_name; }
    [[nodiscard]] constexpr auto GetStorageClass() const { return m_storage; }
    [[nodiscard]] constexpr auto IsReadonly() const { return m_readonly; }

    constexpr void SetAbiName(auto abi_name) { m_abi_name = abi_name; }
    constexpr void SetValue(auto value) { m_value = value; }
    constexpr void SetName(auto name) { m_name = name; }
    constexpr void SetStorageClass(auto storage) { m_storage = storage; }
    constexpr void SetReadonly(auto readonly) { m_readonly = readonly; }
  };

  template <class A>
  class GenericRet final : public GenericExpr<A> {
    friend A;

    FlowPtr<GenericExpr<A>> m_expr;

  public:
    constexpr GenericRet(auto expr) : GenericExpr<A>(IR_eRET), m_expr(expr) {}

    [[nodiscard]] constexpr auto GetExpr() const { return m_expr; }
    constexpr void SetExpr(auto expr) { m_expr = expr; }
  };

  template <class A>
  class GenericBrk final : public GenericExpr<A> {
    friend A;

  public:
    constexpr GenericBrk() : GenericExpr<A>(IR_eBRK) {}
  };

  template <class A>
  class GenericCont final : public GenericExpr<A> {
    friend A;

  public:
    constexpr GenericCont() : GenericExpr<A>(IR_eSKIP) {}
  };

  template <class A>
  class GenericIf final : public GenericExpr<A> {
    friend A;

    FlowPtr<GenericExpr<A>> m_cond, m_then, m_else;

  public:
    constexpr GenericIf(auto cond, auto then, auto ele)
        : GenericExpr<A>(IR_eIF), m_cond(cond), m_then(then), m_else(ele) {}

    [[nodiscard]] constexpr auto GetCond() const { return m_cond; }
    [[nodiscard]] constexpr auto GetThen() const { return m_then; }
    [[nodiscard]] constexpr auto GetElse() const { return m_else; }

    constexpr void SetCond(auto cond) { m_cond = cond; }
    constexpr void SetThen(auto then) { m_then = then; }
    constexpr void SetElse(auto ele) { m_else = ele; }
  };

  template <class A>
  class GenericWhile final : public GenericExpr<A> {
    friend A;

    FlowPtr<GenericExpr<A>> m_cond;
    FlowPtr<GenericSeq<A>> m_body;

  public:
    constexpr GenericWhile(auto cond, auto body)
        : GenericExpr<A>(IR_eWHILE), m_cond(cond), m_body(body) {}

    [[nodiscard]] constexpr auto GetCond() const { return m_cond; }
    [[nodiscard]] constexpr auto GetBody() const { return m_body; }

    constexpr void SetCond(auto cond) { m_cond = cond; }
    constexpr void SetBody(auto body) { m_body = body; }
  };

  template <class A>
  class GenericFor final : public GenericExpr<A> {
    friend A;

    FlowPtr<GenericExpr<A>> m_init, m_cond, m_step, m_body;

  public:
    constexpr GenericFor(auto init, auto cond, auto step, auto body)
        : GenericExpr<A>(IR_eFOR),
          m_init(init),
          m_cond(cond),
          m_step(step),
          m_body(body) {}

    [[nodiscard]] constexpr auto GetInit() const { return m_init; }
    [[nodiscard]] constexpr auto GetCond() const { return m_cond; }
    [[nodiscard]] constexpr auto GetStep() const { return m_step; }
    [[nodiscard]] constexpr auto GetBody() const { return m_body; }

    constexpr void SetInit(auto init) { m_init = init; }
    constexpr void SetCond(auto cond) { m_cond = cond; }
    constexpr void SetStep(auto step) { m_step = step; }
    constexpr void SetBody(auto body) { m_body = body; }
  };

  template <class A>
  class GenericCase final : public GenericExpr<A> {
    friend A;

    FlowPtr<GenericExpr<A>> m_cond, m_body;

  public:
    constexpr GenericCase(auto cond, auto body)
        : GenericExpr<A>(IR_eCASE), m_cond(cond), m_body(body) {}

    constexpr auto GetCond() { return m_cond; }
    constexpr auto GetBody() { return m_body; }

    constexpr void SetCond(auto cond) { m_cond = cond; }
    constexpr void SetBody(auto body) { m_body = body; }
  };

  template <class A>
  class GenericSwitch final : public GenericExpr<A> {
    friend A;

    FlowPtr<GenericExpr<A>> m_cond;
    NullableFlowPtr<GenericExpr<A>> m_default;
    std::span<FlowPtr<GenericCase<A>>> m_cases;

  public:
    constexpr GenericSwitch(auto cond, auto cases, auto default_case)
        : GenericExpr<A>(IR_eSWITCH),
          m_cond(cond),
          m_default(default_case),
          m_cases(cases) {}

    [[nodiscard]] constexpr auto GetCond() const { return m_cond; }
    [[nodiscard]] constexpr auto GetCases() const { return m_cases; }
    [[nodiscard]] constexpr auto GetDefault() const { return m_default; }

    constexpr void SetCond(auto cond) { m_cond = cond; }
    constexpr void SetDefault(auto default_case) { m_default = default_case; }
    constexpr void SetCases(auto cases) { m_cases = cases; }
  };

  template <class A>
  class GenericFunction final : public GenericExpr<A> {
    friend A;

    std::span<std::pair<FlowPtr<GenericType<A>>, string>> m_params;
    NullableFlowPtr<GenericSeq<A>> m_body;
    FlowPtr<GenericType<A>> m_return;
    string m_name, m_abi_name;
    bool m_variadic;

  public:
    constexpr GenericFunction(auto name, auto params, auto ret_ty, auto body,
                              auto variadic, auto abi_name)
        : GenericExpr<A>(IR_eFUNCTION),
          m_params(params),
          m_body(body),
          m_return(ret_ty),
          m_name(name),
          m_abi_name(abi_name),
          m_variadic(variadic) {}

    [[nodiscard]] constexpr auto GetParams() const { return m_params; }
    [[nodiscard]] constexpr auto GetReturn() const { return m_return; }
    [[nodiscard]] constexpr auto GetBody() const { return m_body; }
    [[nodiscard]] constexpr auto IsVariadic() const { return m_variadic; }
    [[nodiscard]] constexpr auto GetAbiName() const { return m_abi_name; }
    [[nodiscard]] constexpr auto GetName() const { return m_name; }

    constexpr void SetName(auto name) { m_name = name; }
    constexpr void SetParams(auto params) { m_params = params; }
    constexpr void SetReturn(auto ret_ty) { m_return = ret_ty; }
    constexpr void SetBody(auto body) { m_body = body; }
    constexpr void SetVariadic(auto variadic) { m_variadic = variadic; }
    constexpr void SetAbiName(auto abi_name) { m_abi_name = abi_name; }
  };

  template <class A>
  class GenericAsm final : public GenericExpr<A> {
    friend A;

  public:
    constexpr GenericAsm() : GenericExpr<A>(IR_eASM) {}
  };

}  // namespace ncc::ir

#endif
