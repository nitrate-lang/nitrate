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
#include <nitrate-lexer/Lexer.hh>
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

      Data& operator()() { return m_data; }
      const Data& operator()() const { return m_data; }
      bool operator==(const Value& o) const { return m_data == o.m_data; }
    };

    using NextFunc = std::function<std::optional<Value>()>;

  private:
    NextFunc m_next_func;
    std::optional<Value> m_peek;
    ReaderSourceManager m_source;
    std::optional<FlowPtr<Base>> m_root;

    std::optional<Value> NextValue() {
      if (m_peek.has_value()) {
        auto val = m_peek;
        m_peek.reset();
        return val;
      }

      return m_next_func();
    }

    std::optional<Value> PeekValue() {
      if (!m_peek.has_value()) {
        m_peek = m_next_func();
      }

      return m_peek;
    }

    struct LocationRange {
      lex::Location m_start, m_end;
    };

    std::optional<LocationRange> ReadLocationRange();

    NullableFlowPtr<Base> DeserializeObject();
    NullableFlowPtr<Stmt> DeserializeStatement();
    NullableFlowPtr<Expr> DeserializeExpression();
    NullableFlowPtr<Type> DeserializeType();

    struct TypeMetadata {
      NullableFlowPtr<Expr> m_width, m_min, m_max;
    };

    std::optional<TypeMetadata> ReadTypeMetadata();

    NullableFlowPtr<Base> ReadKindNode();
    NullableFlowPtr<BinExpr> ReadKindBinexpr();
    NullableFlowPtr<UnaryExpr> ReadKindUnexpr();
    NullableFlowPtr<TernaryExpr> ReadKindTerexpr();
    NullableFlowPtr<ConstInt> ReadKindInt();
    NullableFlowPtr<ConstFloat> ReadKindFloat();
    NullableFlowPtr<ConstString> ReadKindString();
    NullableFlowPtr<ConstChar> ReadKindChar();
    NullableFlowPtr<ConstBool> ReadKindBool();
    NullableFlowPtr<ConstNull> ReadKindNull();
    NullableFlowPtr<ConstUndef> ReadKindUndef();
    NullableFlowPtr<Call> ReadKindCall();
    NullableFlowPtr<List> ReadKindList();
    NullableFlowPtr<Assoc> ReadKindAssoc();
    NullableFlowPtr<Index> ReadKindIndex();
    NullableFlowPtr<Slice> ReadKindSlice();
    NullableFlowPtr<FString> ReadKindFstring();
    NullableFlowPtr<Ident> ReadKindIdent();
    NullableFlowPtr<SeqPoint> ReadKindSeqPoint();
    NullableFlowPtr<PostUnaryExpr> ReadKindPostUnexpr();
    NullableFlowPtr<StmtExpr> ReadKindStmtExpr();
    NullableFlowPtr<TypeExpr> ReadKindTypeExpr();
    NullableFlowPtr<TemplCall> ReadKindTemplCall();
    NullableFlowPtr<RefTy> ReadKindRef();
    NullableFlowPtr<U1> ReadKindU1();
    NullableFlowPtr<U8> ReadKindU8();
    NullableFlowPtr<U16> ReadKindU16();
    NullableFlowPtr<U32> ReadKindU32();
    NullableFlowPtr<U64> ReadKindU64();
    NullableFlowPtr<U128> ReadKindU128();
    NullableFlowPtr<I8> ReadKindI8();
    NullableFlowPtr<I16> ReadKindI16();
    NullableFlowPtr<I32> ReadKindI32();
    NullableFlowPtr<I64> ReadKindI64();
    NullableFlowPtr<I128> ReadKindI128();
    NullableFlowPtr<F16> ReadKindF16();
    NullableFlowPtr<F32> ReadKindF32();
    NullableFlowPtr<F64> ReadKindF64();
    NullableFlowPtr<F128> ReadKindF128();
    NullableFlowPtr<VoidTy> ReadKindVoid();
    NullableFlowPtr<PtrTy> ReadKindPtr();
    NullableFlowPtr<OpaqueTy> ReadKindOpaque();
    NullableFlowPtr<ArrayTy> ReadKindArray();
    NullableFlowPtr<TupleTy> ReadKindTuple();
    NullableFlowPtr<FuncTy> ReadKindFuncTy();
    NullableFlowPtr<NamedTy> ReadKindUnres();
    NullableFlowPtr<InferTy> ReadKindInfer();
    NullableFlowPtr<TemplType> ReadKindTempl();
    NullableFlowPtr<TypedefStmt> ReadKindTypedef();
    NullableFlowPtr<StructDef> ReadKindStruct();
    NullableFlowPtr<EnumDef> ReadKindEnum();
    NullableFlowPtr<Function> ReadKindFunction();
    NullableFlowPtr<ScopeStmt> ReadKindScope();
    NullableFlowPtr<ExportStmt> ReadKindExport();
    NullableFlowPtr<Block> ReadKindBlock();
    NullableFlowPtr<VarDecl> ReadKindLet();
    NullableFlowPtr<InlineAsm> ReadKindInlineAsm();
    NullableFlowPtr<ReturnStmt> ReadKindReturn();
    NullableFlowPtr<ReturnIfStmt> ReadKindRetif();
    NullableFlowPtr<BreakStmt> ReadKindBreak();
    NullableFlowPtr<ContinueStmt> ReadKindContinue();
    NullableFlowPtr<IfStmt> ReadKindIf();
    NullableFlowPtr<WhileStmt> ReadKindWhile();
    NullableFlowPtr<ForStmt> ReadKindFor();
    NullableFlowPtr<ForeachStmt> ReadKindForeach();
    NullableFlowPtr<CaseStmt> ReadKindCase();
    NullableFlowPtr<SwitchStmt> ReadKindSwitch();
    NullableFlowPtr<ExprStmt> ReadKindExprStmt();

    class StrongBool {
    public:
      bool m_val;

      constexpr StrongBool operator!() const { return {!m_val}; }
      constexpr StrongBool operator||(StrongBool o) const {
        return {m_val || o.m_val};
      }
      constexpr operator bool() { return m_val; }
    };

    template <typename ValueType>
    constexpr StrongBool NextIf(const ValueType& v = ValueType()) {
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
    constexpr StrongBool NextIs() {
      if (auto n = PeekValue()) {
        if (std::holds_alternative<ValueType>(n->operator()())) {
          return StrongBool(true);
        }
      }

      return StrongBool(false);
    }

    template <typename ValueType>
    constexpr ValueType Next() {
      if (auto n = NextValue()) {
        if (std::holds_alternative<ValueType>(n->operator()())) {
          return std::get<ValueType>(n->operator()());
        }
      }

      qcore_panic("Attempted to read value of incorrect type");
    }

  public:
    AstReader(NextFunc data_source, ReaderSourceManager source_manager)
        : m_next_func(data_source), m_source(source_manager) {}
    virtual ~AstReader() = default;

    std::optional<FlowPtr<Base>> Get();
  };

  class NCC_EXPORT AstJsonReader final : public AstReader {
    std::optional<Value> ReadValue();
    std::istream& m_is;

    struct PImpl;
    std::unique_ptr<PImpl> m_pimpl;

  public:
    AstJsonReader(std::istream& is,
                  ReaderSourceManager source_manager = std::nullopt);

    static std::optional<FlowPtr<Base>> FromString(
        const std::string& json,
        ReaderSourceManager source_manager = std::nullopt) {
      std::istringstream is(json);
      AstJsonReader reader(is, source_manager);
      return reader.Get();
    }

    ~AstJsonReader() override;
  };

  class NCC_EXPORT AstMsgPackReader final : public AstReader {
    std::optional<Value> ReadValue();
    std::istream& m_is;

    struct PImpl;
    std::unique_ptr<PImpl> m_pimpl;

  public:
    AstMsgPackReader(std::istream& is,
                     ReaderSourceManager source_manager = std::nullopt);

    static std::optional<FlowPtr<Base>> FromString(
        const std::string& msgpack,
        ReaderSourceManager source_manager = std::nullopt) {
      std::istringstream is(msgpack);
      AstMsgPackReader reader(is, source_manager);
      return reader.Get();
    }

    ~AstMsgPackReader() override;
  };
}  // namespace ncc::parse

#endif
