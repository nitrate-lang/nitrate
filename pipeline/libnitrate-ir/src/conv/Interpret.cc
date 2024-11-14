////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///  ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
///  ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
///    ░▒▓█▓▒░                                                               ///
///     ░▒▓██▓▒░                                                             ///
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

#include <core/LibMacro.h>
#include <nitrate-core/Error.h>
#include <nitrate-ir/IR.h>
#include <nitrate-ir/TypeDecl.h>

#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <map>
#include <nitrate-ir/IRGraph.hh>
#include <stack>
#include <variant>

using namespace nr;

namespace comptime {
  struct ValueTy {
    nr_ty_t ty;

    constexpr ValueTy(nr_ty_t v) : ty(v) {}

    bool is_floating() const {
      /// TODO:
      qcore_implement();
    }

    bool is_integer() const {
      /// TODO:
      qcore_implement();
    }
  };

  struct IntegerValue {
    boost::multiprecision::cpp_int v;
    IntSize w;
    bool is_signed;

    IntegerValue(uint128_t val = 0, IntSize width = IntSize::U32) noexcept {
      v = val;
      w = width;
      is_signed = false;
    }
  };

  using FloatingValue = std::pair<boost::multiprecision::cpp_dec_float_100, FloatSize>;

  class Value {
    std::variant<IntegerValue, FloatingValue, std::vector<Value>, std::map<std::string_view, Value>,
                 Fn *, ValueTy>
        m_data;

  public:
    Value() noexcept = default;
    Value(const Value &other) noexcept : m_data(other.m_data) {}
    Value(Value &&other) noexcept { m_data = std::move(other.m_data); }
    Value &operator=(Value &&other) noexcept {
      m_data = std::move(other.m_data);
      return *this;
    }
    Value(std::variant<IntegerValue, FloatingValue, std::vector<Value>,
                       std::map<std::string_view, Value>, Fn *, ValueTy>
              value)
        : m_data(value) {}

    const auto &data() const { return m_data; }
    auto &data() { return m_data; }

    template <typename T>
    T *get() {
      return std::get_if<T>(&m_data);
    }

    ValueTy getType() const noexcept {
      if (std::holds_alternative<IntegerValue>(m_data)) {
        using SubObj = std::array<nr_ty_t, 2>;
        static const std::array<std::array<nr_ty_t, 2>, 6> tab = {
            SubObj({QIR_NODE_U1_TY, QIR_NODE_U1_TY}),
            SubObj({QIR_NODE_U8_TY, QIR_NODE_I8_TY}),
            SubObj({QIR_NODE_U16_TY, QIR_NODE_I16_TY}),
            SubObj({QIR_NODE_U32_TY, QIR_NODE_I32_TY}),
            SubObj({QIR_NODE_U64_TY, QIR_NODE_I64_TY}),
            SubObj({QIR_NODE_U128_TY, QIR_NODE_I128_TY}),
        };

        const IntegerValue &v = std::get<IntegerValue>(m_data);

        return tab[(int)v.w][(int)v.is_signed];
      } else if (std::holds_alternative<FloatingValue>(m_data)) {
        static const std::array<nr_ty_t, 4> tab = {
            QIR_NODE_F16_TY,
            QIR_NODE_F32_TY,
            QIR_NODE_F64_TY,
            QIR_NODE_F128_TY,
        };

        return tab[(int)std::get<FloatingValue>(m_data).second];
      } else if (std::holds_alternative<std::vector<Value>>(m_data)) {
        /// TODO:
        qcore_implement();

      } else if (std::holds_alternative<std::map<std::string_view, Value>>(m_data)) {
        /// TODO:
        qcore_implement();

      } else if (std::holds_alternative<Fn *>(m_data)) {
        /// TODO:
        qcore_implement();

      } else if (std::holds_alternative<ValueTy>(m_data)) {
        /// TODO:
        qcore_implement();

      } else {
        __builtin_unreachable();
      }
    }

    ///********************************************************************///

    std::optional<Value> addOpI(const Value &) const noexcept {
      /// TODO:
      return std::nullopt;
    }

    std::optional<Value> addOpF(const Value &) const noexcept {
      /// TODO:
      return std::nullopt;
    }

    std::optional<Value> subOpI(const Value &) const noexcept {
      /// TODO:
      return std::nullopt;
    }

    std::optional<Value> subOpF(const Value &) const noexcept {
      /// TODO:
      return std::nullopt;
    }

    std::optional<Value> mulOpI(const Value &) const noexcept {
      /// TODO:
      return std::nullopt;
    }

    std::optional<Value> mulOpF(const Value &) const noexcept {
      /// TODO:
      return std::nullopt;
    }

    std::optional<Value> divOpI(const Value &) const noexcept {
      /// TODO:
      return std::nullopt;
    }

    std::optional<Value> divOpF(const Value &) const noexcept {
      /// TODO:
      return std::nullopt;
    }

    std::optional<Value> modOpI(const Value &) const noexcept {
      /// TODO:
      return std::nullopt;
    }

    std::optional<Value> modOpF(const Value &) const noexcept {
      /// TODO:
      return std::nullopt;
    }

    std::optional<Value> bitAndOpI(const Value &) const noexcept {
      /// TODO:
      return std::nullopt;
    }

    std::optional<Value> bitOrOpI(const Value &) const noexcept {
      /// TODO:
      return std::nullopt;
    }

    std::optional<Value> bitXorOpI(const Value &) const noexcept {
      /// TODO:
      return std::nullopt;
    }

    std::optional<Value> lShiftOpI(const Value &) const noexcept {
      /// TODO:
      return std::nullopt;
    }

    std::optional<Value> rShiftOpI(const Value &) const noexcept {
      /// TODO:
      return std::nullopt;
    }

    std::optional<Value> lTOpI(const Value &) const noexcept {
      /// TODO:
      return std::nullopt;
    }

    std::optional<Value> lTOpF(const Value &) const noexcept {
      /// TODO:
      return std::nullopt;
    }

    std::optional<Value> gTOpI(const Value &) const noexcept {
      /// TODO:
      return std::nullopt;
    }

    std::optional<Value> gTOpF(const Value &) const noexcept {
      /// TODO:
      return std::nullopt;
    }

    std::optional<Value> lEOpI(const Value &) const noexcept {
      /// TODO:
      return std::nullopt;
    }

    std::optional<Value> lEOpF(const Value &) const noexcept {
      /// TODO:
      return std::nullopt;
    }

    std::optional<Value> gEOpI(const Value &) const noexcept {
      /// TODO:
      return std::nullopt;
    }

    std::optional<Value> gEOpF(const Value &) const noexcept {
      /// TODO:
      return std::nullopt;
    }

    std::optional<Value> eqOpI(const Value &) const noexcept {
      /// TODO:
      return std::nullopt;
    }

    std::optional<Value> eqOpF(const Value &) const noexcept {
      /// TODO:
      return std::nullopt;
    }

    std::optional<Value> nEOpI(const Value &) const noexcept {
      /// TODO:
      return std::nullopt;
    }

    std::optional<Value> nEOpF(const Value &) const noexcept {
      /// TODO:
      return std::nullopt;
    }

    std::optional<Value> bitcastAsOp(const Value &) const noexcept {
      /// TODO:
      return std::nullopt;
    }

    std::optional<Value> castAsOp(const Value &) const noexcept {
      /// TODO:
      return std::nullopt;
    }
  };

  struct ScopeBlock {
    std::unordered_map<std::string_view, Value> locals;
  };

  struct CallFrame {
    std::string_view name;
  };

  class Program {
    std::vector<ScopeBlock> scope_stack;
    std::stack<CallFrame> call_stack;
    std::function<void(std::string_view)> eprintn_cb;

  public:
    std::optional<Value> find_value(std::string_view name) const {
      auto local_it = scope_stack.back().locals.find(name);
      if (local_it != scope_stack.back().locals.end()) {
        return local_it->second;
      } else [[unlikely]] {
        return std::nullopt;
      }
    }

    void setEPrintN(std::function<void(std::string_view)> cb) { eprintn_cb = cb; }

    void eprintn(std::string_view message) { eprintn_cb(message); }

    Program() {
      scope_stack.push_back({});
      call_stack.push({});
    }
  };

  static std::optional<Value> compute_binexpr(Program &, const Value &L, Op O,
                                              const Value &R) noexcept {
    std::optional<Value> ANS;

    ValueTy LT = L.getType();

    if (LT.is_integer()) [[likely]] {
      switch (O) {
        case Op::Plus: {
          ANS = L.addOpI(R);
          break;
        }

        case Op::Minus: {
          ANS = L.subOpI(R);
          break;
        }

        case Op::Times: {
          ANS = L.mulOpI(R);
          break;
        }

        case Op::Slash: {
          ANS = L.divOpI(R);
          /// FIXME: Print error on divide by zero
          break;
        }

        case Op::Percent: {
          ANS = L.modOpI(R);
          /// FIXME: Print error on mod by zero
          break;
        }

        case Op::BitAnd: {
          ANS = L.bitAndOpI(R);
          break;
        }

        case Op::BitOr: {
          ANS = L.bitOrOpI(R);
          break;
        }

        case Op::BitXor: {
          ANS = L.bitXorOpI(R);
          break;
        }

        case Op::LogicAnd: {
          /// TODO: Implement operator
          break;
        }

        case Op::LogicOr: {
          /// TODO: Implement operator
          break;
        }

        case Op::LShift: {
          ANS = L.lShiftOpI(R);
          break;
        }

        case Op::RShift: {
          ANS = L.rShiftOpI(R);
          break;
        }

        case Op::ROTR: {
          /// TODO: Implement operator
          break;
        }

        case Op::ROTL: {
          /// TODO: Implement operator
          break;
        }

        case Op::Set: {
          /// TODO: Implement operator
          break;
        }

        case Op::LT: {
          ANS = R.lTOpI(R);
          break;
        }

        case Op::GT: {
          ANS = R.gTOpI(R);
          break;
        }

        case Op::LE: {
          ANS = R.lEOpI(R);
          break;
        }

        case Op::GE: {
          ANS = R.gEOpI(R);
          break;
        }

        case Op::Eq: {
          ANS = R.eqOpI(R);
          break;
        }

        case Op::NE: {
          ANS = R.nEOpI(R);
          break;
        }

        case Op::BitcastAs: {
          ANS = R.bitcastAsOp(R);
          break;
        }

        case Op::CastAs: {
          ANS = R.castAsOp(R);
          break;
        }

        default: {
          break;
        }
      }
    } else {
      switch (O) {
        case Op::Plus: {
          ANS = L.addOpF(R);
          break;
        }

        case Op::Minus: {
          ANS = L.subOpF(R);
          break;
        }

        case Op::Times: {
          ANS = L.mulOpF(R);
          break;
        }

        case Op::Slash: {
          ANS = L.divOpF(R);
          /// FIXME: Print error on divide by zero
          break;
        }

        case Op::Percent: {
          ANS = L.modOpF(R);
          /// FIXME: Print error on mod by zero
          break;
        }

        case Op::LogicAnd: {
          /// TODO: Implement operator
          break;
        }

        case Op::LogicOr: {
          /// TODO: Implement operator
          break;
        }

        case Op::Set: {
          /// TODO: Implement operator
          break;
        }

        case Op::LT: {
          ANS = R.lTOpF(R);
          break;
        }

        case Op::GT: {
          ANS = R.gTOpF(R);
          break;
        }

        case Op::LE: {
          ANS = R.lEOpF(R);
          break;
        }

        case Op::GE: {
          ANS = R.gEOpF(R);
          break;
        }

        case Op::Eq: {
          ANS = R.eqOpF(R);
          break;
        }

        case Op::NE: {
          ANS = R.nEOpF(R);
          break;
        }

        case Op::BitcastAs: {
          ANS = R.bitcastAsOp(R);
          break;
        }

        case Op::CastAs: {
          ANS = R.castAsOp(R);
          break;
        }

        default: {
          break;
        }
      }
    }

    return ANS;
  }

  static std::optional<Value> compute_unexpr(Program &P, const Value &E, Op O) {
    std::optional<Value> ANS;

    ValueTy ET = E.getType();

    (void)P;

    switch (O) {
      case Op::Plus: {
        ANS = E;
        break;
      }

      case Op::BitAnd: {
        /// TODO: Implement operator
        break;
      }

      case Op::Alignof: {
        /// TODO: Implement operator
        break;
      }

      case Op::Bitsizeof: {
        /// TODO: Implement operator
        break;
      }

      default: {
        break;
      }
    }

    if (!ANS.has_value() && ET.is_integer()) {
      switch (O) {
        case Op::Minus: {
          /// TODO: Implement operator
          break;
        }

        case Op::Times: {
          /// TODO: Implement operator
          break;
        }

        case Op::BitAnd: {
          /// TODO: Implement operator
          break;
        }

        case Op::BitNot: {
          /// TODO: Implement operator
          break;
        }

        case Op::LogicNot: {
          /// TODO: Implement operator
          break;
        }

        case Op::Inc: {
          /// TODO: Implement operator
          break;
        }

        case Op::Dec: {
          /// TODO: Implement operator
          break;
        }

        default: {
          break;
        }
      }
    } else if (!ANS.has_value()) {
      switch (O) {
        case Op::Minus: {
          /// TODO: Implement operator
          break;
        }

        case Op::Times: {
          /// TODO: Implement operator
          break;
        }

        case Op::BitAnd: {
          /// TODO: Implement operator
          break;
        }

        case Op::BitNot: {
          /// TODO: Implement operator
          break;
        }

        case Op::LogicNot: {
          /// TODO: Implement operator
          break;
        }

        case Op::Inc: {
          /// TODO: Implement operator
          break;
        }

        case Op::Dec: {
          /// TODO: Implement operator
          break;
        }

        default: {
          break;
        }
      }
    }

    return ANS;
  }

  std::optional<Value> evaluate(Program &P, nr::Expr *x) noexcept {
    auto x_kind = x->getKind();

    switch (x_kind) {
        ///**********************************************************************///
        ///                          SIMPLE EXPRESSION NODES                     ///
        ///**********************************************************************///

      case QIR_NODE_BINEXPR: {
        BinExpr *B = x->as<BinExpr>();

        auto L = evaluate(P, B->getLHS());
        if (!L.has_value()) {
          P.eprintn("Failed to evaluate left-hand side of binary expression");
          return std::nullopt;
        }

        auto R = evaluate(P, B->getRHS());
        if (!R.has_value()) {
          P.eprintn("Failed to evaluate right-hand side of binary expression");
          return std::nullopt;
        }

        /// TODO: Type promotion

        return compute_binexpr(P, L.value(), B->getOp(), R.value());
      }

      case QIR_NODE_UNEXPR: {
        UnExpr *U = x->as<UnExpr>();

        auto E = evaluate(P, U->getExpr());
        if (!E.has_value()) {
          P.eprintn("Failed to evaluate unary expression");
          return std::nullopt;
        }

        E = compute_unexpr(P, E.value(), U->getOp());
        if (!E.has_value()) {
          P.eprintn("Failed to compute unary expression");
          return std::nullopt;
        }

        return E;
      }

      case QIR_NODE_POST_UNEXPR: {
        PostUnExpr *U = x->as<PostUnExpr>();

        auto E = evaluate(P, U->getExpr());
        if (!E.has_value()) {
          P.eprintn("Failed to evaluate post-unary expression");
          return std::nullopt;
        }

        if (!compute_unexpr(P, E.value(), U->getOp()).has_value()) {
          P.eprintn("Failed to compute post-unary expression");
          return std::nullopt;
        }

        return E;
      }

        ///**********************************************************************///
        ///                     COMPLEX EXPRESSION NODES                         ///
        ///**********************************************************************///

      case QIR_NODE_CALL: {
        /// TODO: Implement expression
        P.eprintn("Call expressions are not yet implemented");
        return std::nullopt;
      }

      case QIR_NODE_SEQ: {
        /// TODO: Implement expression
        P.eprintn("Sequence expressions are not yet implemented");
        return std::nullopt;
      }

      case QIR_NODE_INDEX: {
        /// TODO: Implement expression
        P.eprintn("Index expressions are not yet implemented");
        return std::nullopt;
      }

      case QIR_NODE_IDENT: {
        /// TODO: Implement expression
        P.eprintn("Identifier expressions are not yet implemented");
        return std::nullopt;
      }

      case QIR_NODE_EXTERN: {
        /// TODO: Implement expression
        P.eprintn("Extern expressions are not yet implemented");
        return std::nullopt;
      }

      case QIR_NODE_LOCAL: {
        /// TODO: Implement expression
        P.eprintn("Local expressions are not yet implemented");
        return std::nullopt;
      }

      case QIR_NODE_RET: {
        /// TODO: Implement expression
        P.eprintn("Return expressions are not yet implemented");
        return std::nullopt;
      }

      case QIR_NODE_BRK: {
        /// TODO: Implement expression
        P.eprintn("Break expressions are not yet implemented");
        return std::nullopt;
      }

      case QIR_NODE_CONT: {
        /// TODO: Implement expression
        P.eprintn("Continue expressions are not yet implemented");
        return std::nullopt;
      }

      case QIR_NODE_IF: {
        /// TODO: Implement expression
        P.eprintn("If expressions are not yet implemented");
        return std::nullopt;
      }

      case QIR_NODE_WHILE: {
        /// TODO: Implement expression
        P.eprintn("While expressions are not yet implemented");
        return std::nullopt;
      }

      case QIR_NODE_FOR: {
        /// TODO: Implement expression
        P.eprintn("For expressions are not yet implemented");
        return std::nullopt;
      }

      case QIR_NODE_CASE: {
        /// TODO: Implement expression
        P.eprintn("Case expressions are not yet implemented");
        return std::nullopt;
      }

      case QIR_NODE_SWITCH: {
        /// TODO: Implement expression
        P.eprintn("Switch expressions are not yet implemented");
        return std::nullopt;
      }

      case QIR_NODE_FN: {
        /// TODO: Implement expression
        P.eprintn("Function expressions are not yet implemented");
        return std::nullopt;
      }

      case QIR_NODE_ASM: {
        /// TODO: Implement expression
        P.eprintn("Asm expressions are not yet implemented");
        return std::nullopt;
      }

      case QIR_NODE_IGN: {
        P.eprintn("Unexpected 'QIR_NODE_IGN' node in program DAG");
        return std::nullopt;
      }

      case QIR_NODE_TMP: {
        P.eprintn("Unexpected 'QIR_NODE_TMP' node in program DAG");
        return std::nullopt;
      }

        ///**********************************************************************///
        ///                          PASS THROUGH NODES                          ///
        ///**********************************************************************///

      case QIR_NODE_INT: {
        Int *n = x->as<Int>();

        return Value(IntegerValue(n->getValue(), n->getSize()));
      }

      case QIR_NODE_FLOAT: {
        Float *n = x->as<Float>();

        return Value(FloatingValue(n->getValue(), n->getSize()));
      }

      case QIR_NODE_LIST: {
        List *n = x->as<List>();
        std::vector<Value> items;
        items.resize(n->getItems().size());

        for (size_t i = 0; i < n->getItems().size(); i++) {
          auto tmp = evaluate(P, n->getItems()[i]);
          if (!tmp.has_value()) {
            P.eprintn("Failed to convert list element");
            return std::nullopt;
          }

          items[i] = std::move(tmp.value());
        }

        return Value(std::move(items));
      }

      case QIR_NODE_U1_TY:
      case QIR_NODE_U8_TY:
      case QIR_NODE_U16_TY:
      case QIR_NODE_U32_TY:
      case QIR_NODE_U64_TY:
      case QIR_NODE_U128_TY:
      case QIR_NODE_I8_TY:
      case QIR_NODE_I16_TY:
      case QIR_NODE_I32_TY:
      case QIR_NODE_I64_TY:
      case QIR_NODE_I128_TY:
      case QIR_NODE_F16_TY:
      case QIR_NODE_F32_TY:
      case QIR_NODE_F64_TY:
      case QIR_NODE_F128_TY:
      case QIR_NODE_VOID_TY:
      case QIR_NODE_PTR_TY:
      case QIR_NODE_OPAQUE_TY:
      case QIR_NODE_STRUCT_TY:
      case QIR_NODE_UNION_TY:
      case QIR_NODE_ARRAY_TY:
      case QIR_NODE_FN_TY: {
        // return x;
        /// TODO: Do conversion
        qcore_implement();
      }
    }
  }

  static Type *valuety_to_type(const ValueTy &V) {
    switch (V.ty) {
      case QIR_NODE_U1_TY: {
        return create<U1Ty>();
      }

      case QIR_NODE_U8_TY: {
        return create<U8Ty>();
      }

      case QIR_NODE_U16_TY: {
        return create<U64Ty>();
      }

      case QIR_NODE_U32_TY: {
        return create<U32Ty>();
      }

      case QIR_NODE_U64_TY: {
        return create<U64Ty>();
      }

      case QIR_NODE_U128_TY: {
        return create<U128Ty>();
      }

      case QIR_NODE_I8_TY: {
        return create<I8Ty>();
      }

      case QIR_NODE_I16_TY: {
        return create<I16Ty>();
      }

      case QIR_NODE_I32_TY: {
        return create<I32Ty>();
      }

      case QIR_NODE_I64_TY: {
        return create<I64Ty>();
      }

      case QIR_NODE_I128_TY: {
        return create<I128Ty>();
      }

      case QIR_NODE_F16_TY: {
        return create<F16Ty>();
      }

      case QIR_NODE_F32_TY: {
        return create<F32Ty>();
      }

      case QIR_NODE_F64_TY: {
        return create<F64Ty>();
      }

      case QIR_NODE_F128_TY: {
        return create<F128Ty>();
      }

      case QIR_NODE_VOID_TY: {
        return create<VoidTy>();
      }

      case QIR_NODE_PTR_TY: {
        /// TODO:
        qcore_implement();
        // return create<Ty>();
      }

      case QIR_NODE_OPAQUE_TY: {
        /// TODO:
        qcore_implement();
        // return create<Ty>();
      }

      case QIR_NODE_STRUCT_TY: {
        /// TODO:
        qcore_implement();
        // return create<Ty>();
      }

      case QIR_NODE_UNION_TY: {
        /// TODO:
        qcore_implement();
        // return create<Ty>();
      }

      case QIR_NODE_ARRAY_TY: {
        /// TODO:
        qcore_implement();
        // return create<Ty>();
      }

      case QIR_NODE_FN_TY: {
        /// TODO:
        qcore_implement();
        // return create<Ty>();
      }

      default: {
        qcore_panic("unreachable");
      }
    }
  }

  static std::optional<Expr *> value_to_expression(Program &P, const Value &V) {
    const auto &data = V.data();

    if (std::holds_alternative<IntegerValue>(data)) {
      const IntegerValue &num = std::get<IntegerValue>(data);

      return create<Int>(num.v.convert_to<uint128_t>(), num.w);
    } else if (std::holds_alternative<FloatingValue>(data)) {
      const FloatingValue &num = std::get<FloatingValue>(data);

      return create<Float>(num.first.convert_to<long double>(), num.second);
    } else if (std::holds_alternative<std::vector<Value>>(data)) {
      const auto &items = std::get<std::vector<Value>>(data);

      ListItems list_items;
      list_items.resize(items.size());

      for (size_t i = 0; i < items.size(); i++) {
        auto tmp = value_to_expression(P, items[i]);
        if (!tmp.has_value()) [[unlikely]] {
          P.eprintn("Internal Error: Failed to convert list item into IRGraph node");
          return std::nullopt;
        }

        list_items[i] = tmp.value();
      }

      return create<List>(std::move(list_items), false);
    } else if (std::holds_alternative<std::map<std::string_view, Value>>(data)) {
      const auto &items = std::get<std::map<std::string_view, Value>>(data);

      ListItems pairs;
      pairs.resize(items.size());
      size_t i = 0;

      for (const auto &[key, value] : items) {
        auto v = value_to_expression(P, value);
        if (!v.has_value()) {
          P.eprintn("Internal Error: Failed to convert value in map to IRGraph");
          return std::nullopt;
        }

        ListItems entry;
        entry.resize(2);

        entry[0] = createStringLiteral(key);
        entry[1] = v.value();

        pairs[i++] = create<List>(std::move(entry), false);
      }

      return create<List>(std::move(pairs), false);
    } else if (std::holds_alternative<Fn *>(data)) {
      return std::get<Fn *>(data);
    } else if (std::holds_alternative<ValueTy>(data)) {
      return valuety_to_type(std::get<ValueTy>(data));
    } else {
      return std::nullopt;
    }
  }
}  // namespace comptime

std::optional<nr::Expr *> nr::comptime_impl(
    nr::Expr *x, std::optional<std::function<void(std::string_view)>> eprintn) noexcept {
  comptime::Program P;

  P.setEPrintN(eprintn.value_or([](std::string_view) {}));

  auto result = evaluate(P, x);
  if (!result.has_value()) {
    P.eprintn("Failed to interpret program");
    return std::nullopt;
  }

  auto converted = value_to_expression(P, result.value());
  if (!result.has_value()) {
    P.eprintn("Internal Error: Failed to convert abstract value representation into IRGraph node");
  }

  return converted.value();
}
