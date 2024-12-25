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

#include <cstdint>
#include <nitrate-ir/IRBase.hh>
#include <nitrate-ir/IRData.hh>
#include <nitrate-ir/IRFwd.hh>
#include <nitrate-lexer/Token.hh>
#include <optional>
#include <ostream>
#include <string>
#include <vector>

namespace ncc::ir {
  ///=============================================================================
  /// BEGIN: EXPRESSIONS CATEGORIES
  ///=============================================================================

  enum class Op {
    Plus,      /* '+': Addition operator */
    Minus,     /* '-': Subtraction operator */
    Times,     /* '*': Multiplication operator */
    Slash,     /* '/': Division operator */
    Percent,   /* '%': Modulus operator */
    BitAnd,    /* '&': Bitwise AND operator */
    BitOr,     /* '|': Bitwise OR operator */
    BitXor,    /* '^': Bitwise XOR operator */
    BitNot,    /* '~': Bitwise NOT operator */
    LogicAnd,  /* '&&': Logical AND operator */
    LogicOr,   /* '||': Logical OR operator */
    LogicNot,  /* '!': Logical NOT operator */
    LShift,    /* '<<': Left shift operator */
    RShift,    /* '>>': Right shift operator */
    Inc,       /* '++': Increment operator */
    Dec,       /* '--': Decrement operator */
    Set,       /* '=': Assignment operator */
    LT,        /* '<': Less than operator */
    GT,        /* '>': Greater than operator */
    LE,        /* '<=': Less than or equal to operator */
    GE,        /* '>=': Greater than or equal to operator */
    Eq,        /* '==': Equal to operator */
    NE,        /* '!=': Not equal to operator */
    Alignof,   /* 'alignof': Alignment of operator */
    BitcastAs, /* 'bitcast_as': Bitcast operator */
    CastAs,    /* 'cast_as': Common operator */
    Bitsizeof, /* 'bitsizeof': Bit size of operator */
  };

  enum class AbiTag {
    C,
    Nitrate,
    Internal,
    Default = Nitrate,
  };

  std::ostream &operator<<(std::ostream &os, Op op);

  template <class A>
  class IR_Vertex_BinExpr final : public IR_Vertex_Expr<A> {
    IR_Vertex_Expr<A> *m_lhs, *m_rhs;
    Op m_op;

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
    IR_Vertex_Expr<A> *m_expr;
    Op m_op;
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

    constexpr void setExpr(IR_Vertex_Expr<A> *expr) { m_expr = expr; }
    constexpr void setOp(Op op) { m_op = op; }
    constexpr void setPostfix(bool is_postfix) { m_postfix = is_postfix; }
  };

  ///=============================================================================
  /// END: EXPRESSIONS CATEGORIES
  ///=============================================================================

  ///=============================================================================
  /// BEGIN: LITERALS
  ///=============================================================================

  template <class A>
  class IR_Vertex_Int final : public IR_Vertex_Expr<A> {
    struct map_hash {
      std::size_t operator()(std::pair<uint128_t, uint8_t> const &v) const {
        return std::hash<uint128_t>()(v.first) ^ std::hash<uint8_t>()(v.second);
      }
    };

    static inline std::unordered_map<std::pair<uint128_t, uint8_t>,
                                     IR_Vertex_Int<A> *, map_hash>
        m_cache;

    unsigned __int128 m_value __attribute__((aligned(16)));
    uint8_t m_size;

    static uint128_t str2u128(std::string_view x);

  public:
    IR_Vertex_Int(uint128_t val, uint8_t size)
        : IR_Vertex_Expr<A>(IR_eINT), m_value(val), m_size(size) {}

    IR_Vertex_Int(std::string_view str, uint8_t size)
        : IR_Vertex_Expr<A>(IR_eINT), m_value(str2u128(str)) {
      m_size = size;
    }

    static IR_Vertex_Int *get(uint128_t val, uint8_t size);
    static IR_Vertex_Int *get(std::string_view str, uint8_t size) {
      return get(str2u128(str), size);
    }

    uint8_t getSize() const { return m_size; }
    uint128_t getValue() const { return m_value; }
    std::string getValueString() const;
  } __attribute__((packed));

  static_assert(sizeof(IR_Vertex_Int<void>) == 48);

  enum class FloatSize : uint8_t {
    F16,
    F32,
    F64,
    F128,
  };

  template <class A>
  class IR_Vertex_Float final : public IR_Vertex_Expr<A> {
    double m_data;
    FloatSize m_size;

    static_assert(sizeof(double) == 8);

  public:
    IR_Vertex_Float(double dec, FloatSize size)
        : IR_Vertex_Expr<A>(IR_eFLOAT), m_data{dec}, m_size(size) {}

    IR_Vertex_Float(std::string_view str) : IR_Vertex_Expr<A>(IR_eFLOAT) {
      m_data = std::stod(std::string(str));
      if (str.ends_with("f128")) {
        m_size = FloatSize::F128;
      } else if (str.ends_with("f32")) {
        m_size = FloatSize::F32;
      } else if (str.ends_with("f16")) {
        m_size = FloatSize::F16;
      } else {
        m_size = FloatSize::F64;
      }
    }

    FloatSize getSize() const { return m_size; }
    double getValue() const { return m_data; }
  } __attribute__((packed));

  template <class A>
  using IR_Vertex_ListItems =
      std::vector<IR_Vertex_Expr<A> *, Arena<IR_Vertex_Expr<A> *>>;

  template <class A>
  class IR_Vertex_List final : public IR_Vertex_Expr<A> {
    /// FIXME: Implement run-length compression

    IR_Vertex_ListItems<A> m_items;
    bool m_is_homogenous;

  public:
    IR_Vertex_List(const IR_Vertex_ListItems<A> &items, bool is_homogenous)
        : IR_Vertex_Expr<A>(IR_eLIST),
          m_items(items),
          m_is_homogenous(is_homogenous) {}

    auto begin() const { return m_items.begin(); }
    auto end() const { return m_items.end(); }
    size_t size() const { return m_items.size(); }

    IR_Vertex_Expr<A> *operator[](size_t idx) const { return m_items[idx]; }
    IR_Vertex_Expr<A> *at(size_t idx) const { return m_items.at(idx); }

    bool isHomogenous() const { return m_is_homogenous; }
  };

  ///=============================================================================
  /// END: LITERALS
  ///=============================================================================

  ///=============================================================================
  /// BEGIN: EXPRESSIONS
  ///=============================================================================

  template <class A>
  using IR_Vertex_CallArgs =
      std::vector<IR_Vertex_Expr<A> *, Arena<IR_Vertex_Expr<A> *>>;

  template <class A>
  class IR_Vertex_Call final : public IR_Vertex_Expr<A> {
    IR_Vertex_Expr<A> *m_iref; /* Possibly cyclic reference to the target. */
    IR_Vertex_CallArgs<A> m_args;

  public:
    IR_Vertex_Call(IR_Vertex_Expr<A> *ref, const IR_Vertex_CallArgs<A> &args)
        : IR_Vertex_Expr<A>(IR_eCALL), m_iref(ref), m_args(args) {}

    auto getTarget() const { return m_iref; }
    void setTarget(IR_Vertex_Expr<A> *ref) { m_iref = ref; }

    const IR_Vertex_CallArgs<A> &getArgs() const { return m_args; }
    IR_Vertex_CallArgs<A> &getArgs() { return m_args; }
    void setArgs(const IR_Vertex_CallArgs<A> &args) { m_args = args; }

    size_t getNumArgs() { return m_args.size(); }
  };

  template <class A>
  using IR_Vertex_SeqItems =
      std::vector<IR_Vertex_Expr<A> *, Arena<IR_Vertex_Expr<A> *>>;

  template <class A>
  class IR_Vertex_Seq final : public IR_Vertex_Expr<A> {
    IR_Vertex_SeqItems<A> m_items;

  public:
    IR_Vertex_Seq(const IR_Vertex_SeqItems<A> &items)
        : IR_Vertex_Expr<A>(IR_eSEQ), m_items(items) {}

    const IR_Vertex_SeqItems<A> &getItems() const { return m_items; }
    IR_Vertex_SeqItems<A> &getItems() { return m_items; }
    void setItems(const IR_Vertex_SeqItems<A> &items) { m_items = items; }
  };

  template <class A>
  class IR_Vertex_Index final : public IR_Vertex_Expr<A> {
    IR_Vertex_Expr<A> *m_expr;
    IR_Vertex_Expr<A> *m_index;

  public:
    IR_Vertex_Index(IR_Vertex_Expr<A> *expr, IR_Vertex_Expr<A> *index)
        : IR_Vertex_Expr<A>(IR_eINDEX), m_expr(expr), m_index(index) {}

    auto getExpr() const { return m_expr; }
    void setExpr(IR_Vertex_Expr<A> *expr) { m_expr = expr; }

    auto getIndex() const { return m_index; }
    void setIndex(IR_Vertex_Expr<A> *index) { m_index = index; }
  };

  template <class A>
  class IR_Vertex_Ident final : public IR_Vertex_Expr<A> {
    std::string_view m_name;
    IR_Vertex_Expr<A> *m_what;

  public:
    IR_Vertex_Ident(std::string_view name, IR_Vertex_Expr<A> *what)
        : IR_Vertex_Expr<A>(IR_eIDENT), m_name(name), m_what(what) {}

    auto getWhat() const { return m_what; }
    auto getName() const { return m_name; }

    void setWhat(IR_Vertex_Expr<A> *what) { m_what = what; }
    void setName(std::string_view name) { m_name = name; }
  };

  template <class A>
  class IR_Vertex_Extern final : public IR_Vertex_Expr<A> {
    std::string_view m_abi_name;
    IR_Vertex_Expr<A> *m_value;

  public:
    IR_Vertex_Extern(IR_Vertex_Expr<A> *value, std::string_view abi_name)
        : IR_Vertex_Expr<A>(IR_eEXTERN), m_abi_name(abi_name), m_value(value) {}

    std::string_view getAbiName() const { return m_abi_name; }
    std::string_view setAbiName(std::string_view abi_name) {
      m_abi_name = abi_name;
    }

    auto getValue() const { return m_value; }
    void setValue(IR_Vertex_Expr<A> *value) { m_value = value; }
  };

  template <class A>
  class IR_Vertex_Local final : public IR_Vertex_Expr<A> {
    std::string_view m_name;
    IR_Vertex_Expr<A> *m_value;
    AbiTag m_abi_tag;
    StorageClass m_storage;
    bool m_readonly;

  public:
    IR_Vertex_Local(std::string_view name, IR_Vertex_Expr<A> *value,
                    AbiTag abi_tag, bool readonly = false,
                    StorageClass storage_class = StorageClass::LLVM_StackAlloa)
        : IR_Vertex_Expr<A>(IR_eLOCAL),
          m_name(name),
          m_value(value),
          m_abi_tag(abi_tag),
          m_storage(storage_class),
          m_readonly(readonly) {}

    std::string_view getName() const { return m_name; }
    auto getValue() const { return m_value; }
    AbiTag getAbiTag() const { return m_abi_tag; }
    StorageClass getStorageClass() const { return m_storage; }
    bool isReadonly() const { return m_readonly; }

    void setAbiTag(AbiTag abi_tag) { m_abi_tag = abi_tag; }
    void setValue(IR_Vertex_Expr<A> *value) { m_value = value; }
    void setName(std::string_view name) { m_name = name; }
    void setStorageClass(StorageClass storage) { m_storage = storage; }
    void setReadonly(bool readonly) { m_readonly = readonly; }
  };

  template <class A>
  class IR_Vertex_Ret final : public IR_Vertex_Expr<A> {
    IR_Vertex_Expr<A> *m_expr;

  public:
    IR_Vertex_Ret(IR_Vertex_Expr<A> *expr)
        : IR_Vertex_Expr<A>(IR_eRET), m_expr(expr) {}

    auto getExpr() const { return m_expr; }
    void setExpr(IR_Vertex_Expr<A> *expr) { m_expr = expr; }
  };

  template <class A>
  class IR_Vertex_Brk final : public IR_Vertex_Expr<A> {
  public:
    IR_Vertex_Brk() : IR_Vertex_Expr<A>(IR_eBRK) {}
  };

  template <class A>
  class IR_Vertex_Cont final : public IR_Vertex_Expr<A> {
  public:
    IR_Vertex_Cont() : IR_Vertex_Expr<A>(IR_eSKIP) {}
  };

  template <class A>
  class IR_Vertex_If final : public IR_Vertex_Expr<A> {
    IR_Vertex_Expr<A> *m_cond;
    IR_Vertex_Expr<A> *m_then;
    IR_Vertex_Expr<A> *m_else;

  public:
    IR_Vertex_If(IR_Vertex_Expr<A> *cond, IR_Vertex_Expr<A> *then,
                 IR_Vertex_Expr<A> *else_)
        : IR_Vertex_Expr<A>(IR_eIF),
          m_cond(cond),
          m_then(then),
          m_else(else_) {}

    auto getCond() const { return m_cond; }
    void setCond(IR_Vertex_Expr<A> *cond) { m_cond = cond; }

    auto getThen() const { return m_then; }
    void setThen(IR_Vertex_Expr<A> *then) { m_then = then; }

    auto getElse() const { return m_else; }
    void setElse(IR_Vertex_Expr<A> *else_) { m_else = else_; }
  };

  template <class A>
  class IR_Vertex_While final : public IR_Vertex_Expr<A> {
    IR_Vertex_Expr<A> *m_cond;
    IR_Vertex_Seq<A> *m_body;

  public:
    IR_Vertex_While(IR_Vertex_Expr<A> *cond, IR_Vertex_Seq<A> *body)
        : IR_Vertex_Expr<A>(IR_eWHILE), m_cond(cond), m_body(body) {}

    auto getCond() const { return m_cond; }
    void setCond(IR_Vertex_Expr<A> *cond) { m_cond = cond; }

    auto getBody() const { return m_body; }
    IR_Vertex_Seq<A> *setBody(IR_Vertex_Seq<A> *body) { m_body = body; }
  };

  template <class A>
  class IR_Vertex_For final : public IR_Vertex_Expr<A> {
    IR_Vertex_Expr<A> *m_init;
    IR_Vertex_Expr<A> *m_cond;
    IR_Vertex_Expr<A> *m_step;
    IR_Vertex_Expr<A> *m_body;

  public:
    IR_Vertex_For(IR_Vertex_Expr<A> *init, IR_Vertex_Expr<A> *cond,
                  IR_Vertex_Expr<A> *step, IR_Vertex_Expr<A> *body)
        : IR_Vertex_Expr<A>(IR_eFOR),
          m_init(init),
          m_cond(cond),
          m_step(step),
          m_body(body) {}

    auto getInit() const { return m_init; }
    void setInit(IR_Vertex_Expr<A> *init) { m_init = init; }

    auto getCond() const { return m_cond; }
    void setCond(IR_Vertex_Expr<A> *cond) { m_cond = cond; }

    auto getStep() const { return m_step; }
    void setStep(IR_Vertex_Expr<A> *step) { m_step = step; }

    auto getBody() const { return m_body; }
    void setBody(IR_Vertex_Expr<A> *body) { m_body = body; }
  };

  template <class A>
  class IR_Vertex_Case final : public IR_Vertex_Expr<A> {
    IR_Vertex_Expr<A> *m_cond;
    IR_Vertex_Expr<A> *m_body;

  public:
    IR_Vertex_Case(IR_Vertex_Expr<A> *cond, IR_Vertex_Expr<A> *body)
        : IR_Vertex_Expr<A>(IR_eCASE), m_cond(cond), m_body(body) {}

    auto getCond() { return m_cond; }
    void setCond(IR_Vertex_Expr<A> *cond) { m_cond = cond; }

    auto getBody() { return m_body; }
    void setBody(IR_Vertex_Expr<A> *body) { m_body = body; }
  };

  template <class A>
  using SwitchCases =
      std::vector<IR_Vertex_Case<A> *, Arena<IR_Vertex_Case<A> *>>;

  template <class A>
  class IR_Vertex_Switch final : public IR_Vertex_Expr<A> {
    IR_Vertex_Expr<A> *m_cond;
    IR_Vertex_Expr<A> *m_default;
    SwitchCases<A> m_cases;

  public:
    IR_Vertex_Switch(IR_Vertex_Expr<A> *cond, const SwitchCases<A> &cases,
                     IR_Vertex_Expr<A> *default_)
        : IR_Vertex_Expr<A>(IR_eSWITCH),
          m_cond(cond),
          m_default(default_),
          m_cases(cases) {}

    auto getCond() const { return m_cond; }
    void setCond(IR_Vertex_Expr<A> *cond) { m_cond = cond; }

    auto getDefault() const { return m_default; }
    void setDefault(IR_Vertex_Expr<A> *default_) { m_default = default_; }

    const SwitchCases<A> &getCases() const { return m_cases; }
    SwitchCases<A> &getCases() { return m_cases; }
    void setCases(const SwitchCases<A> &cases) { m_cases = cases; }
    void addCase(IR_Vertex_Case<A> *c) { m_cases.push_back(c); }
  };

  template <class A>
  using Params =
      std::vector<std::pair<IR_Vertex_Type<A> *, std::string_view>,
                  Arena<std::pair<IR_Vertex_Type<A> *, std::string_view>>>;

  template <class A>
  class IR_Vertex_Fn final : public IR_Vertex_Expr<A> {
    std::string_view m_name;
    Params<A> m_params;
    IR_Vertex_Type<A> *m_return;
    std::optional<IR_Vertex_Seq<A> *> m_body;
    bool m_variadic;
    AbiTag m_abi_tag;

  public:
    IR_Vertex_Fn(std::string_view name, const Params<A> &params,
                 IR_Vertex_Type<A> *ret_ty,
                 std::optional<IR_Vertex_Seq<A> *> body, bool variadic,
                 AbiTag abi_tag)
        : IR_Vertex_Expr<A>(IR_eFUNCTION),
          m_name(name),
          m_params(params),
          m_return(ret_ty),
          m_body(body),
          m_variadic(variadic),
          m_abi_tag(abi_tag) {}

    std::string_view setName(std::string_view name) { m_name = name; }

    const Params<A> &getParams() const { return m_params; }
    Params<A> &getParams() { return m_params; }
    void setParams(const Params<A> &params) { m_params = params; }

    auto getReturn() const { return m_return; }
    IR_Vertex_Type<A> *setReturn(IR_Vertex_Type<A> *ret_ty) {
      m_return = ret_ty;
    }

    std::optional<IR_Vertex_Seq<A> *> getBody() const { return m_body; }
    std::optional<IR_Vertex_Seq<A> *> setBody(
        std::optional<IR_Vertex_Seq<A> *> body) {
      m_body = body;
    }

    bool isVariadic() const { return m_variadic; }
    void setVariadic(bool variadic) { m_variadic = variadic; }

    AbiTag getAbiTag() const { return m_abi_tag; }
    AbiTag setAbiTag(AbiTag abi_tag) { m_abi_tag = abi_tag; }
  };

  template <class A>
  class IR_Vertex_Asm final : public IR_Vertex_Expr<A> {
  public:
    IR_Vertex_Asm() : IR_Vertex_Expr<A>(IR_eASM) { qcore_implement(); }
  };

  ///=============================================================================
  /// END: EXPRESSIONS
  ///=============================================================================

}  // namespace ncc::ir

#endif
