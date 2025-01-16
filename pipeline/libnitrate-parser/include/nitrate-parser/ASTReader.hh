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

#ifndef __NITRATE_AST_READER_H__
#define __NITRATE_AST_READER_H__

#include <cstdint>
#include <istream>
#include <nitrate-core/Macro.hh>
#include <nitrate-core/NullableFlowPtr.hh>
#include <nitrate-lexer/Scanner.hh>
#include <nitrate-parser/ASTBase.hh>
#include <optional>
#include <variant>

namespace ncc::parse {
  using ReaderSourceManager =
      std::optional<std::reference_wrapper<lex::IScanner>>;

  class NCC_EXPORT AstReader {
  public:
    using none = std::nullptr_t;

    class Value {
      using Data = std::variant<std::string, uint64_t, double, bool, none>;
      Data m_data;

    public:
      template <typename T>
      Value(T&& data) : m_data(std::forward<T>(data)) {}

      auto operator()() -> Data& { return m_data; }
      auto operator()() const -> const Data& { return m_data; }
      auto operator==(const Value& o) const -> bool {
        return m_data == o.m_data;
      }
    };

    using NextFunc = std::function<std::optional<Value>()>;

  private:
    NextFunc m_next_func;
    std::optional<Value> m_peek;
    ReaderSourceManager m_source;
    std::optional<FlowPtr<Base>> m_root;

    auto NextValue() -> std::optional<Value> {
      if (m_peek.has_value()) {
        auto val = m_peek;
        m_peek.reset();
        return val;
      }

      return m_next_func();
    }

    auto PeekValue() -> std::optional<Value> {
      if (!m_peek.has_value()) {
        m_peek = m_next_func();
      }

      return m_peek;
    }

    struct LocationRange {
      lex::Location m_start, m_end;
    };

    auto ReadLocationRange() -> std::optional<LocationRange>;

    auto DeserializeObject() -> NullableFlowPtr<Base>;
    auto DeserializeStatement() -> NullableFlowPtr<Stmt>;
    auto DeserializeExpression() -> NullableFlowPtr<Expr>;
    auto DeserializeType() -> NullableFlowPtr<Type>;

    struct TypeMetadata {
      NullableFlowPtr<Expr> m_width, m_min, m_max;
    };

    auto ReadTypeMetadata() -> std::optional<TypeMetadata>;

    auto ReadKindNode() -> NullableFlowPtr<Base>;
    auto ReadKindBinexpr() -> NullableFlowPtr<BinExpr>;
    auto ReadKindUnexpr() -> NullableFlowPtr<UnaryExpr>;
    auto ReadKindTerexpr() -> NullableFlowPtr<TernaryExpr>;
    auto ReadKindInt() -> NullableFlowPtr<ConstInt>;
    auto ReadKindFloat() -> NullableFlowPtr<ConstFloat>;
    auto ReadKindString() -> NullableFlowPtr<ConstString>;
    auto ReadKindChar() -> NullableFlowPtr<ConstChar>;
    auto ReadKindBool() -> NullableFlowPtr<ConstBool>;
    auto ReadKindNull() -> NullableFlowPtr<ConstNull>;
    auto ReadKindUndef() -> NullableFlowPtr<ConstUndef>;
    auto ReadKindCall() -> NullableFlowPtr<Call>;
    auto ReadKindList() -> NullableFlowPtr<List>;
    auto ReadKindAssoc() -> NullableFlowPtr<Assoc>;
    auto ReadKindIndex() -> NullableFlowPtr<Index>;
    auto ReadKindSlice() -> NullableFlowPtr<Slice>;
    auto ReadKindFstring() -> NullableFlowPtr<FString>;
    auto ReadKindIdent() -> NullableFlowPtr<Ident>;
    auto ReadKindSeqPoint() -> NullableFlowPtr<SeqPoint>;
    auto ReadKindPostUnexpr() -> NullableFlowPtr<PostUnaryExpr>;
    auto ReadKindStmtExpr() -> NullableFlowPtr<StmtExpr>;
    auto ReadKindTypeExpr() -> NullableFlowPtr<TypeExpr>;
    auto ReadKindTemplCall() -> NullableFlowPtr<TemplCall>;
    auto ReadKindRef() -> NullableFlowPtr<RefTy>;
    auto ReadKindU1() -> NullableFlowPtr<U1>;
    auto ReadKindU8() -> NullableFlowPtr<U8>;
    auto ReadKindU16() -> NullableFlowPtr<U16>;
    auto ReadKindU32() -> NullableFlowPtr<U32>;
    auto ReadKindU64() -> NullableFlowPtr<U64>;
    auto ReadKindU128() -> NullableFlowPtr<U128>;
    auto ReadKindI8() -> NullableFlowPtr<I8>;
    auto ReadKindI16() -> NullableFlowPtr<I16>;
    auto ReadKindI32() -> NullableFlowPtr<I32>;
    auto ReadKindI64() -> NullableFlowPtr<I64>;
    auto ReadKindI128() -> NullableFlowPtr<I128>;
    auto ReadKindF16() -> NullableFlowPtr<F16>;
    auto ReadKindF32() -> NullableFlowPtr<F32>;
    auto ReadKindF64() -> NullableFlowPtr<F64>;
    auto ReadKindF128() -> NullableFlowPtr<F128>;
    auto ReadKindVoid() -> NullableFlowPtr<VoidTy>;
    auto ReadKindPtr() -> NullableFlowPtr<PtrTy>;
    auto ReadKindOpaque() -> NullableFlowPtr<OpaqueTy>;
    auto ReadKindArray() -> NullableFlowPtr<ArrayTy>;
    auto ReadKindTuple() -> NullableFlowPtr<TupleTy>;
    auto ReadKindFuncTy() -> NullableFlowPtr<FuncTy>;
    auto ReadKindUnres() -> NullableFlowPtr<NamedTy>;
    auto ReadKindInfer() -> NullableFlowPtr<InferTy>;
    auto ReadKindTempl() -> NullableFlowPtr<TemplType>;
    auto ReadKindTypedef() -> NullableFlowPtr<TypedefStmt>;
    auto ReadKindStruct() -> NullableFlowPtr<StructDef>;
    auto ReadKindEnum() -> NullableFlowPtr<EnumDef>;
    auto ReadKindFunction() -> NullableFlowPtr<Function>;
    auto ReadKindScope() -> NullableFlowPtr<ScopeStmt>;
    auto ReadKindExport() -> NullableFlowPtr<ExportStmt>;
    auto ReadKindBlock() -> NullableFlowPtr<Block>;
    auto ReadKindLet() -> NullableFlowPtr<VarDecl>;
    auto ReadKindInlineAsm() -> NullableFlowPtr<InlineAsm>;
    auto ReadKindReturn() -> NullableFlowPtr<ReturnStmt>;
    auto ReadKindRetif() -> NullableFlowPtr<ReturnIfStmt>;
    auto ReadKindBreak() -> NullableFlowPtr<BreakStmt>;
    auto ReadKindContinue() -> NullableFlowPtr<ContinueStmt>;
    auto ReadKindIf() -> NullableFlowPtr<IfStmt>;
    auto ReadKindWhile() -> NullableFlowPtr<WhileStmt>;
    auto ReadKindFor() -> NullableFlowPtr<ForStmt>;
    auto ReadKindForeach() -> NullableFlowPtr<ForeachStmt>;
    auto ReadKindCase() -> NullableFlowPtr<CaseStmt>;
    auto ReadKindSwitch() -> NullableFlowPtr<SwitchStmt>;
    auto ReadKindExprStmt() -> NullableFlowPtr<ExprStmt>;

    class StrongBool {
    public:
      bool m_val;

      constexpr auto operator!() const -> StrongBool { return {!m_val}; }
      constexpr auto operator||(StrongBool o) const -> StrongBool {
        return {m_val || o.m_val};
      }
      constexpr operator bool() const { return m_val; }
    };

    template <typename ValueType>
    constexpr auto NextIf(const ValueType& v = ValueType()) -> StrongBool {
      if (auto n = PeekValue()) {
        if (std::holds_alternative<ValueType>(n->operator()()) &&
            std::get<ValueType>(n->operator()()) == v) {
          NextValue();
          return StrongBool(true);
        }
      }

      return StrongBool(false);
    }

    template <typename ValueType>
    constexpr auto NextIs() -> StrongBool {
      if (auto n = PeekValue()) {
        if (std::holds_alternative<ValueType>(n->operator()())) {
          return StrongBool(true);
        }
      }

      return StrongBool(false);
    }

    template <typename ValueType>
    constexpr auto Next() -> ValueType {
      if (auto n = NextValue()) {
        if (std::holds_alternative<ValueType>(n->operator()())) {
          return std::get<ValueType>(n->operator()());
        }
      }

      qcore_panic("Attempted to read value of incorrect type");
    }

  public:
    AstReader(NextFunc data_source, ReaderSourceManager source_manager)
        : m_next_func(std::move(data_source)), m_source(source_manager) {}
    virtual ~AstReader() = default;

    auto Get() -> std::optional<FlowPtr<Base>>;
  };

  class NCC_EXPORT AstJsonReader final : public AstReader {
    auto ReadValue() -> std::optional<Value>;
    std::istream& m_is;

    struct PImpl;
    std::unique_ptr<PImpl> m_pimpl;

  public:
    AstJsonReader(std::istream& is,
                  ReaderSourceManager source_manager = std::nullopt);

    static auto FromString(const std::string& json,
                           ReaderSourceManager source_manager = std::nullopt)
        -> std::optional<FlowPtr<Base>> {
      std::istringstream is(json);
      AstJsonReader reader(is, source_manager);
      return reader.Get();
    }

    ~AstJsonReader() override;
  };

  class NCC_EXPORT AstMsgPackReader final : public AstReader {
    auto ReadValue() -> std::optional<Value>;
    std::istream& m_is;

    struct PImpl;
    std::unique_ptr<PImpl> m_pimpl;

  public:
    AstMsgPackReader(std::istream& is,
                     ReaderSourceManager source_manager = std::nullopt);

    static auto FromString(const std::string& msgpack,
                           ReaderSourceManager source_manager = std::nullopt)
        -> std::optional<FlowPtr<Base>> {
      std::istringstream is(msgpack);
      AstMsgPackReader reader(is, source_manager);
      return reader.Get();
    }

    ~AstMsgPackReader() override;
  };
}  // namespace ncc::parse

#endif
