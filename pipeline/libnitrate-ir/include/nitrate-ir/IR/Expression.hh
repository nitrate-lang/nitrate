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

    constexpr void setLHS(auto lhs) { m_lhs = lhs; }
    constexpr void setRHS(auto rhs) { m_rhs = rhs; }
    constexpr void setOp(auto op) { m_op = op; }
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

    constexpr void setExpr(auto expr) { m_expr = expr; }
    constexpr void setOp(auto op) { m_op = op; }
    constexpr void setPostfix(auto is_postfix) { m_postfix = is_postfix; }
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
    std::string getValueString() const;
  };

  template <class A>
  class IR_Vertex_Float final : public IR_Vertex_Expr<A> {
    friend A;

    double m_data;
    FloatSize m_size;

    static_assert(sizeof(double) == 8);

  public:
    constexpr IR_Vertex_Float(auto dec, auto size)
        : IR_Vertex_Expr<A>(IR_eFLOAT), m_data{dec}, m_size(size) {}

    constexpr IR_Vertex_Float(string str) : IR_Vertex_Expr<A>(IR_eFLOAT) {
      m_data = std::stod(std::string(str));
      if (str->ends_with("f128")) {
        m_size = FloatSize::F128;
      } else if (str->ends_with("f32")) {
        m_size = FloatSize::F32;
      } else if (str->ends_with("f16")) {
        m_size = FloatSize::F16;
      } else {
        m_size = FloatSize::F64;
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
    FlowPtr<IR_Vertex_Expr<A>> m_iref;

  public:
    constexpr IR_Vertex_Call(auto ref, IR_Vertex_CallArgs<A> args)
        : IR_Vertex_Expr<A>(IR_eCALL), m_iref(ref), m_args(args) {}

    constexpr auto getTarget() const { return m_iref; }
    constexpr auto getNumArgs() { return m_args.size(); }
    constexpr auto getArgs() const { return m_args; }

    constexpr void setTarget(FlowPtr<IR_Vertex_Expr<A>> ref) { m_iref = ref; }
    void setArgs(IR_Vertex_CallArgs<A> args) { m_args = args; }
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

    constexpr void setItems(auto items) { m_items = items; }
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

    constexpr void setIndex(auto index) { m_index = index; }
    constexpr void setExpr(auto expr) { m_expr = expr; }
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
    constexpr auto getName() const { return m_name.get(); }

    constexpr void setWhat(auto what) { m_what = what; }
    constexpr void setName(auto name) { m_name = name; }
  };

  template <class A>
  class IR_Vertex_Extern final : public IR_Vertex_Expr<A> {
    friend A;

    FlowPtr<IR_Vertex_Expr<A>> m_value;
    string m_abi_name;

  public:
    constexpr IR_Vertex_Extern(auto value, auto abi_name)
        : IR_Vertex_Expr<A>(IR_eEXTERN), m_value(value), m_abi_name(abi_name) {}

    constexpr auto getAbiName() const { return m_abi_name.get(); }
    constexpr auto getValue() const { return m_value; }

    constexpr void setValue(auto value) { m_value = value; }
    constexpr void setAbiName(auto abi_name) { m_abi_name = abi_name; }
  };

  template <class A>
  class IR_Vertex_Local final : public IR_Vertex_Expr<A> {
    friend A;

    FlowPtr<IR_Vertex_Expr<A>> m_value;
    string m_name;
    AbiTag m_abi_tag;
    StorageClass m_storage;
    bool m_readonly;

  public:
    constexpr IR_Vertex_Local(
        auto name, auto value, auto abi_tag, auto readonly = false,
        auto storage_class = StorageClass::LLVM_StackAlloa)
        : IR_Vertex_Expr<A>(IR_eLOCAL),
          m_value(value),
          m_name(name),
          m_abi_tag(abi_tag),
          m_storage(storage_class),
          m_readonly(readonly) {}

    constexpr auto getName() const { return m_name.get(); }
    constexpr auto getValue() const { return m_value; }
    constexpr auto getAbiTag() const { return m_abi_tag; }
    constexpr auto getStorageClass() const { return m_storage; }
    constexpr auto isReadonly() const { return m_readonly; }

    constexpr void setAbiTag(auto abi_tag) { m_abi_tag = abi_tag; }
    constexpr void setValue(auto value) { m_value = value; }
    constexpr void setName(auto name) { m_name = name; }
    constexpr void setStorageClass(auto storage) { m_storage = storage; }
    constexpr void setReadonly(auto readonly) { m_readonly = readonly; }
  };

  template <class A>
  class IR_Vertex_Ret final : public IR_Vertex_Expr<A> {
    friend A;

    FlowPtr<IR_Vertex_Expr<A>> m_expr;

  public:
    constexpr IR_Vertex_Ret(auto expr)
        : IR_Vertex_Expr<A>(IR_eRET), m_expr(expr) {}

    constexpr auto getExpr() const { return m_expr; }
    constexpr void setExpr(auto expr) { m_expr = expr; }
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

    constexpr void setCond(auto cond) { m_cond = cond; }
    constexpr void setThen(auto then) { m_then = then; }
    constexpr void setElse(auto ele) { m_else = ele; }
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

    constexpr void setCond(auto cond) { m_cond = cond; }
    constexpr void setBody(auto body) { m_body = body; }
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

    constexpr void setInit(auto init) { m_init = init; }
    constexpr void setCond(auto cond) { m_cond = cond; }
    constexpr void setStep(auto step) { m_step = step; }
    constexpr void setBody(auto body) { m_body = body; }
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

    constexpr void setCond(auto cond) { m_cond = cond; }
    constexpr void setBody(auto body) { m_body = body; }
  };

  template <class A>
  class IR_Vertex_Switch final : public IR_Vertex_Expr<A> {
    friend A;

    FlowPtr<IR_Vertex_Expr<A>> m_cond, m_default;
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

    constexpr void setCond(auto cond) { m_cond = cond; }
    constexpr void setDefault(auto default_) { m_default = default_; }
    constexpr void setCases(auto cases) { m_cases = cases; }
  };

  template <class A>
  class IR_Vertex_Function final : public IR_Vertex_Expr<A> {
    friend A;

    std::span<std::pair<FlowPtr<IR_Vertex_Type<A>>, std::string_view>> m_params;
    NullableFlowPtr<IR_Vertex_Seq<A>> m_body;
    FlowPtr<IR_Vertex_Type<A>> m_return;
    string m_name;
    AbiTag m_abi_tag;
    bool m_variadic;

  public:
    constexpr IR_Vertex_Function(auto name, auto params, auto ret_ty, auto body,
                                 auto variadic, auto abi_tag)
        : IR_Vertex_Expr<A>(IR_eFUNCTION),
          m_params(params),
          m_body(body),
          m_return(ret_ty),
          m_name(name),
          m_abi_tag(abi_tag),
          m_variadic(variadic) {}

    constexpr auto getParams() const { return m_params; }
    constexpr auto getReturn() const { return m_return; }
    constexpr auto getBody() const { return m_body; }
    constexpr auto isVariadic() const { return m_variadic; }
    constexpr auto getAbiTag() const { return m_abi_tag; }
    constexpr auto getName() const { return m_name.get(); }

    constexpr void setName(auto name) { m_name = name; }
    constexpr void setParams(auto params) { m_params = params; }
    constexpr void setReturn(auto ret_ty) { m_return = ret_ty; }
    constexpr void setBody(auto body) { m_body = body; }
    constexpr void setVariadic(auto variadic) { m_variadic = variadic; }
    constexpr void setAbiTag(auto abi_tag) { m_abi_tag = abi_tag; }
  };

  template <class A>
  class IR_Vertex_Asm final : public IR_Vertex_Expr<A> {
    friend A;

  public:
    constexpr IR_Vertex_Asm() : IR_Vertex_Expr<A>(IR_eASM) {}
  };

}  // namespace ncc::ir

#endif
